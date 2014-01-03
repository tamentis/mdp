/*
 * Copyright (c) 2012-2013 Bertrand Janin <b@janin.com>
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

#include "array.h"
#include "mdp.h"
#include "utils.h"
#include "config.h"
#include "results.h"
#include "pager.h"
#include "gpg.h"
#include "lock.h"
#include "generate.h"
#include "keywords.h"
#include "cleanup.h"
#include "editor.h"
#include "debug.h"
#include "cmd.h"


char		*home = NULL;
char		*passwords_path = NULL;


static void
usage(void)
{
	printf("usage: mdp [-eVh] [-c config] [-k key_id]\n");
	printf("       mdp [-Vh] file ... -g length\n");
	printf("       mdp [-ErqVh] [-c config] [-k key_id] keyword ...\n");

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
mdp(enum action_mode mode)
{
	switch (mode) {
	case MODE_VERSION:
		debug("mode: VERSION");
		printf("mdp-%s\n", MDP_VERSION);
		break;

	case MODE_USAGE:
		debug("mode: USAGE");
		usage();
		/* NOT REACHED */
		break;

	case MODE_RAW:
		debug("mode: RAW");
		if (keywords_count() == 0) {
			usage();
		}

		gpg_check();
		if (load_results_gpg() == 0)
			errx(EXIT_FAILURE, "no passwords");
		filter_results();
		print_results();
		break;

	case MODE_PAGER:
		debug("mode: PAGER");
		if (keywords_count() == 0) {
			usage();
		}

		gpg_check();
		if (load_results_gpg() == 0)
			errx(EXIT_FAILURE, "no passwords");
		filter_results();
		pager(START_WITHOUT_PROMPT);
		break;

	case MODE_QUERY:
		debug("mode: QUERY");
		gpg_check();
		if (load_results_gpg() == 0)
			errx(EXIT_FAILURE, "no passwords");
		pager(START_WITH_PROMPT);
		break;

	case MODE_EDIT:
		debug("mode: EDIT");
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

	case MODE_GENERATE:
		debug("mode: GENERATE");
		if (keywords_count() > 0) {
			usage();
		}

		generate_passwords(cmd_password_length,
				cfg_password_count);
		break;

	default:
		errx(EXIT_FAILURE, "unknown mode");
		break;
	}
}

int
main(int ac, char **av)
{
	enum action_mode mode;

	home = get_home();
	editor_init();

	mode = cmd_parse(ac, av);

	if (cmd_config_path == NULL) {
		cmd_config_path = join_path(home, ".mdp/config");
	}
	debug("read config (%s)", cmd_config_path);
	config_check_paths();
	config_read();
	config_set_defaults();

	mdp(mode);

	debug("normal shutdown");

	return EXIT_SUCCESS;
}
