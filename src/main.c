/*
 * Copyright (c) 2012-2014 Bertrand Janin <b@janin.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <err.h>
#include <signal.h>
#include <inttypes.h>
#include <stdbool.h>
#include <locale.h>

#include "array.h"
#include "cleanup.h"
#include "cmd.h"
#include "config.h"
#include "debug.h"
#include "editor.h"
#include "gpg.h"
#include "keywords.h"
#include "lock.h"
#include "mdp.h"
#include "pager.h"
#include "profile.h"
#include "results.h"
#include "utils.h"


char *home = NULL;


static void
usage(void)
{
	printf("usage: mdp -e [-Vh] [-c config] [-k key_id]\n");
	printf("       mdp -g [-Vh] [-c config] [-p profile] [-n count] [-l length]\n");
	printf("       mdp [-ErqVh] [-c config] keyword ...\n");

	exit(EXIT_FAILURE);
}


/*
 * Return a copy of the $HOME environment variable. Be generous in flaming the
 * user if their environment is in poor condition.
 */
static char *
get_home(void)
{
	char *s;

	s = getenv("HOME");

	if (s == NULL) {
		errx(EXIT_FAILURE, "unknown variable '$HOME'");
	}

	if (!file_exists(s)) {
		errx(EXIT_FAILURE, "your $HOME does not exist");
	}

	return strdup(s);
}


static void
mdp(enum command command)
{
	struct profile *profile;

	switch (command) {
	case COMMAND_VERSION:
		debug("command: VERSION");
		printf("mdp-%s\n", MDP_VERSION);
		break;

	case COMMAND_USAGE:
		debug("command: USAGE");
		usage();
		/* NOTREACHED */
		break;

	case COMMAND_RAW:
		debug("command: RAW");
		if (keywords_count() == 0) {
			usage();
		}

		gpg_check();
		if (load_results_gpg() == 0)
			errx(EXIT_FAILURE, "no passwords");
		filter_results();
		print_results();
		break;

	case COMMAND_PAGER:
		debug("command: PAGER");
		if (keywords_count() == 0) {
			usage();
		}

		gpg_check();
		if (load_results_gpg() == 0)
			errx(EXIT_FAILURE, "no passwords");
		filter_results();
		pager(START_WITHOUT_PROMPT);
		break;

	case COMMAND_QUERY:
		debug("command: QUERY");
		gpg_check();
		if (load_results_gpg() == 0)
			errx(EXIT_FAILURE, "no passwords");
		pager(START_WITH_PROMPT);
		break;

	case COMMAND_EDIT:
		debug("command: EDIT");
		if (keywords_count() > 0) {
			usage();
		}

		gpg_check();
		lock_set();

		/*
		 * Since we set the lock, configure atexit and signals right
		 * away in case something fail before a normal exit.
		 */
		if (atexit(atexit_cleanup) != 0) {
			err(EXIT_FAILURE, "get_results atexit");
		}

		signal(SIGINT, sig_cleanup);
		signal(SIGKILL, sig_cleanup);

		load_results_gpg();
		edit_results();
		break;

	case COMMAND_GENERATE:
		debug("command: GENERATE");

		if (keywords_count() > 0) {
			usage();
		}

		if (cmd_profile_name == NULL) {
			profile = profile_new("default");
		} else {
			profile = profile_get_from_name(cmd_profile_name);
		}

		if (profile == NULL) {
			errx(EXIT_FAILURE, "unknown profile");
		}

		profile_fprint_passwords(stdout, profile);
		break;

	default:
		errx(EXIT_FAILURE, "unknown command");
		break;
	}
}

int
main(int ac, char **av)
{
	enum command command;

	setlocale(LC_ALL, "");

	home = get_home();
	editor_init(home);

	command = cmd_parse(ac, av);

	if (cmd_config_path == NULL) {
		cmd_config_path = join_path(home, ".mdp/config");
	}
	debug("read config (%s)", cmd_config_path);
	config_check_paths(home);
	config_read();
	config_set_defaults(home);

	mdp(command);

	debug("normal shutdown");

	return EXIT_SUCCESS;
}
