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

#include <sys/param.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <wchar.h>
#include <errno.h>
#include <err.h>
#include <signal.h>
#include <stdarg.h>
#include <locale.h>
#include <inttypes.h>
#include <stdbool.h>
#include <curses.h>

#include "curses.h"
#include "xmalloc.h"
#include "array.h"
#include "mdp.h"
#include "utils.h"
#include "config.h"
#include "results.h"
#include "pager.h"
#include "gpg.h"
#include "lock.h"
#include "randpass.h"
#include "keywords.h"


/* Configuration variables with defaults (global, defined in config.c). */
char		*cfg_config_path = NULL;
char		*cfg_gpg_path = NULL;
char		*cfg_gpg_key_id = NULL;
int		 cfg_gpg_timeout = 20;
char		*cfg_editor;
int		 cfg_timeout = 10;
int		 cfg_password_count = 4;
bool		 cfg_backup = true;
bool		 cfg_debug = false;
bool		 cfg_regex = false;

/* Other globals */
char		*home = NULL;
char		*passwords_path = NULL;
char		*lock_path = NULL;
char		*tmp_path = NULL;
int		 password_length = 16;

/* Result set defined in results.c */
extern struct wlist results;
extern uint32_t result_sum;
extern uint32_t result_size;

/* getopt globals */
extern int optind;
extern char *optarg;


static void
cleanup(void)
{
	if (tmp_path != NULL && unlink(tmp_path) != 0) {
		err(EXIT_FAILURE, "WARNING: unable to remove '%s'", tmp_path);
	}

	lock_unset();

	/* Just in case we error'd out somewhere during the pager. */
	shutdown_curses();
}


static void
atexit_cleanup(void)
{
	debug("atexit_cleanup (PID: %d)", getpid());
	cleanup();
}


static void
sig_cleanup(int dummy)
{
	/* Avoid unused parameter warning. */
	(void)(dummy);

	debug("sig_cleanup (PID: %d)", getpid());
	cleanup();
}


/*
 * Spawn the editor on a file.
 */
static void
spawn_editor(char *path)
{
	char s[MAXPATHLEN];

	snprintf(s, MAXPATHLEN, "%s \"%s\"", cfg_editor, path);

	debug("spawn_editor: %s", s);

	if (system(s) != 0)
		err(EXIT_FAILURE, "unable to spawn editor: %s", s);
}


/*
 * Check if the given path has the right sum and size.
 *
 * This is far from perfect, but for the purpose of detecting change, this is
 * just fine. Returns 1 if it matches.
 */
static int
has_changed(char *tmp_path)
{
	FILE *fp;
	uint32_t previous_sum, previous_size;

	/*
	 * Keep track of the previous sum/size so we can check if anything
	 * changed.
	 */
	previous_sum = result_sum;
	previous_size = result_size;

	fp = fopen(tmp_path, "r");
	load_results_fp(fp);
	fclose(fp);

	if (previous_sum != result_sum || previous_size != result_size)
		return 1;

	return 0;
}


/*
 * Edit the passwords.
 *
 * This function dumps all the plain-text passwords ("results") in a temporary
 * file in your ~/.mdp/ folder, fires your editor and save the output back to
 * your password file.
 */
static void
edit_results(void)
{
	unsigned int i;
	int tmp_fd = -1;
	struct result *result;

	/* Create the temporary file for edit mode. */
	tmp_path = join_path(home, ".mdp/tmp_edit.XXXXXXXX");
	tmp_fd = mkstemp(tmp_path);
	if (tmp_fd == -1) {
		err(EXIT_FAILURE, "edit_results mkstemp()");
	}

	/* Iterate over the results and dump them in this file. */
	for (i = 0; i < ARRAY_LENGTH(&results); i++) {
		result = ARRAY_ITEM(&results, i);
		if (write(tmp_fd, result->mbs_value, result->mbs_len) == -1)
			err(EXIT_FAILURE, "edit_results write");
		if (write(tmp_fd, "\n", 1) == -1)
			err(EXIT_FAILURE, "edit_results write (new-line)");
	}

	if (close(tmp_fd) != 0) {
		err(EXIT_FAILURE, "edit_results close(tmp_fd)");
	}

	spawn_editor(tmp_path);

	if (has_changed(tmp_path)) {
		gpg_encrypt(tmp_path);
	} else {
		fprintf(stderr, "No changes, exiting...\n");
	}
}


/*
 * Print the results to screen in "raw" mode (-r).
 */
static void
print_results(void)
{
	unsigned int i;
	struct result *result;

	for (i = 0; i < ARRAY_LENGTH(&results); i++) {
		result = ARRAY_ITEM(&results, i);
		if (result->visible) {
			printf("%ls\n", result->wcs_value);
		}
	}
}


static void
print_passwords(int length, int count)
{
	char password[MAX_PASSWORD_LENGTH];
	int i;

	for (i = 0; i < count; i++) {
		generate_password(password, length, "NCL");
		printf("%s\n", password);
	}
}


static void
usage(void)
{
	printf("usage: mdp [-ecrVh] [-g len] [keyword ...]\n");
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


/*
 * Return a copy of the $EDITOR environment variable or NULL if not found.
 */
static char *
get_env_editor(void)
{
	char *s;

	s = getenv("EDITOR");

	if (s == NULL) {
		return NULL;
	}

	return strdup(s);
}


int
main(int ac, char **av)
{
	int opt, mode = MODE_PAGER;

	if (ac < 2)
		usage();

	setlocale(LC_ALL, "");

	home = get_home();
	cfg_editor = get_env_editor();

	while ((opt = getopt(ac, av, "hdEVqeg:rc:")) != -1) {
		switch (opt) {
		case 'd':
			cfg_debug = true;
			break;
		case 'E':
			cfg_regex = true;
			break;
		case 'V':
			mode = MODE_VERSION;
			break;
		case 'q':
			mode = MODE_QUERY;
			break;
		case 'e':
			mode = MODE_EDIT;
			break;
		case 'g':
			mode = MODE_GENERATE;
			password_length = strtoumax(optarg, NULL, 10);
			break;
		case 'r':
			mode = MODE_RAW;
			break;
		case 'c':
			cfg_config_path = strdup(optarg);
			break;
		default:
			usage();
		}
	}

	if (cfg_config_path == NULL) {
		cfg_config_path = join_path(home, ".mdp/config");
	}
	debug("read config (%s)", cfg_config_path);

	config_check_paths();
	config_read();
	config_set_defaults();

	ac -= optind;
	av += optind;

	keywords_load_from_argv(av);

	/* Decide if we use the internal pager or just dump to screen. */
	switch (mode) {
		case MODE_VERSION:
			printf("mdp-%s\n", MDP_VERSION);
			break;

		case MODE_RAW:
			debug("mode: MODE_RAW");
			if (ac == 0)
				usage();

			gpg_check();
			if (load_results_gpg() == 0)
				errx(EXIT_FAILURE, "no passwords");
			filter_results();
			print_results();
			break;

		case MODE_PAGER:
			debug("mode: MODE_PAGER");
			if (ac == 0)
				usage();

			gpg_check();
			if (load_results_gpg() == 0)
				errx(EXIT_FAILURE, "no passwords");
			filter_results();
			pager(START_WITHOUT_PROMPT);
			break;

		case MODE_QUERY:
			debug("mode: MODE_QUERY");
			gpg_check();
			if (load_results_gpg() == 0)
				errx(EXIT_FAILURE, "no passwords");
			pager(START_WITH_PROMPT);
			break;

		case MODE_EDIT:
			debug("mode: MODE_EDIT");
			if (ac != 0)
				usage();

			gpg_check();
			lock_set();

			/*
			 * Since we set the lock, configure atexit and signals
			 * right away in case something fail before a normal
			 * exit.
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
			debug("mode: MODE_GENERATE");
			if (ac != 0)
				usage();

			print_passwords(password_length, cfg_password_count);
			break;

		default:
			errx(EXIT_FAILURE, "unknown mode");
			break;
	}

	debug("normal shutdown");

	return EXIT_SUCCESS;
}
