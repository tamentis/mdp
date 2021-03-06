/*
 * Copyright (c) 2012-2015 Bertrand Janin <b@janin.com>
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

#include <stdlib.h>
#include <err.h>
#include <locale.h>
#include <signal.h>

#include "cleanup.h"
#include "cmd.h"
#include "config.h"
#include "debug.h"
#include "editor.h"
#include "gpg.h"
#include "lock.h"
#include "pager.h"
#include "profile.h"
#include "results.h"
#include "utils.h"
#include "xmalloc.h"


/*
 * Since we set the lock, configure atexit and signals right away in case
 * something fail before a normal exit.
 */
static void
setup_signals_and_atexit(void)
{
	if (atexit(atexit_cleanup) != 0) {
		err(EXIT_FAILURE, "get_results atexit");
	}

	signal(SIGINT, sig_cleanup);
	signal(SIGKILL, sig_cleanup);
}


static void
mdp_add(void)
{
	struct profile *profile = NULL;

	debug("mdp_add()");

	if (cmd_profile_name == NULL) {
		profile = profile_new("default");
	} else {
		profile = profile_get_from_name(cmd_profile_name);
	}

	if (profile == NULL) {
		errx(EXIT_FAILURE, "unknown profile");
	}

	gpg_check();
	lock_set();

	setup_signals_and_atexit();

	load_results_gpg();
	profile_passwords_to_results(profile, cmd_add_prefix);
	edit_results();
}


static void
mdp_edit(void)
{
	debug("mdp_edit()");

	gpg_check();
	lock_set();

	setup_signals_and_atexit();

	load_results_gpg();
	edit_results();
}


static void
mdp_generate(void)
{
	struct profile *profile = NULL;

	debug("mdp_generate()");

	if (cmd_profile_name == NULL) {
		profile = profile_new("default");
	} else {
		profile = profile_get_from_name(cmd_profile_name);
	}

	if (profile == NULL) {
		errx(EXIT_FAILURE, "unknown profile");
	}

	profile_fprint_passwords(stdout, profile);
}


static void
mdp_get(void)
{
	debug("mdp_get()");

	gpg_check();

	if (load_results_gpg() == 0)
		errx(EXIT_FAILURE, "no passwords");

	filter_results();

	if (cmd_raw) {
		print_results();
	} else {
		pager();
	}
}


static void
mdp_prompt(void)
{
	debug("mdp_prompt()");

	gpg_check();

	if (load_results_gpg() == 0)
		errx(EXIT_FAILURE, "no passwords");

	pager_with_prompt();
}


static void
read_config(void)
{
	char *home;
	char *config_dir;
	char *password_dir;

	home = get_home();

	config_dir = join_path(home, ".mdp");
	config_ensure_directory(config_dir);

	editor_init(config_dir);

	if (cmd_config_path == NULL) {
		cmd_config_path = join_path(config_dir, "config");
	}
	config_check_file(cmd_config_path);

	config_read();
	config_set_defaults(config_dir);

	password_dir = xdirname(cfg_password_file);
	config_ensure_directory(password_dir);
	config_check_password_file(cfg_password_file);

	xfree(config_dir);
	xfree(home);
}


int
main(int argc, char **argv)
{
	int command_index;

	if (setlocale(LC_ALL, "") == NULL) {
		fprintf(stderr, "WARNING: setting the locale failed, make "
				"sure %s is available on your system.\n",
				getenv("LC_ALL"));
	}

	/*
	 * Anything before the command is considered core argument (not
	 * related to any command). If we didn't find a command, parse all the
	 * arguments as core.
	 */
	command_index = cmd_parse_core(argc, argv);

	if (command_index < 1) {
		cmd_usage_core();
		exit(EXIT_FAILURE);
	}

	argc -= command_index;
	argv += command_index;

	read_config();

	if (command_match(argv[0], "edit", 1)) {
		cmd_parse_edit(argc, argv);
		mdp_edit();
	} else if (command_match(argv[0], "generate", 3)) {
		cmd_parse_generate(argc, argv);
		mdp_generate();
	} else if (command_match(argv[0], "add", 1)) {
		cmd_parse_add(argc, argv);
		mdp_add();
	} else if (command_match(argv[0], "get", 3)) {
		cmd_parse_get(argc, argv);
		mdp_get();
	} else if (command_match(argv[0], "prompt", 1)) {
		cmd_parse_prompt(argc, argv);
		mdp_prompt();
	} else {
		errx(EXIT_FAILURE, "unknown command '%s' (try mdp -h)", argv[0]);
	}

	debug("normal shutdown");
	return (EXIT_SUCCESS);
}
