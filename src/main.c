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
#include "wcslcpy.h"


/*
 * Global variables (extern'd in various files)
 */
wchar_t	 cfg_config_path[MAXPATHLEN] = L"";
wchar_t	 cfg_gpg_path[MAXPATHLEN] = L"/usr/bin/gpg";
wchar_t	 cfg_gpg_key_id[MAXPATHLEN] = L"";
int	 cfg_gpg_timeout = 5;
wchar_t	 cfg_editor[MAXPATHLEN] = L"";
int	 cfg_timeout = 10;
int	 cfg_password_count = 4;
int	 cfg_debug = 0;

wchar_t	 home[MAXPATHLEN];
wchar_t	 passwords_path[MAXPATHLEN];
wchar_t	 lock_path[MAXPATHLEN];
wchar_t	 editor[MAXPATHLEN];
int	 password_length = 16;
char	 tmp_path[MAXPATHLEN] = "";

extern struct wlist results;


void
cleanup(void)
{
	debug("atexit_cleanup");

	if (tmp_path[0] != '\0' && unlink(tmp_path) != 0)
		err(1, "WARNING: unable to remove '%s'", tmp_path);

	lock_unset();

	/* Just in case we error'd out somewhere during the pager. */
	shutdown_curses();
}


void
atexit_cleanup(void)
{
	cleanup();
}


void
sig_cleanup(int dummy)
{
	cleanup();
}


/*
 * Spawn the editor on a file.
 */
void
spawn_editor(char *path)
{
	char s[MAXPATHLEN];

	if (wcslen(cfg_editor) == 0) {
		if (wcslen(editor) == 0) {
			wcslcpy(cfg_editor, L"/usr/bin/vi", MAXPATHLEN);
		} else {
			wcslcpy(cfg_editor, editor, MAXPATHLEN);
		}
	}

	snprintf(s, MAXPATHLEN, "%ls \"%s\"", cfg_editor, path);

	debug("spawn_editor: %s", s);

	if (system(s) != 0)
		err(1, "unable to spawn editor: %s", s);
}


/*
 * Check if the given path has the right sum and size.
 *
 * This is far from perfect, but for the purpose of detecting change, this is
 * just fine. Returns 1 if it matches.
 */
int
has_changed(char *tmp_path, uint32_t sum, uint32_t size)
{
	uint32_t new_size = 0, new_sum = 0;
	FILE *fp = fopen(tmp_path, "r");
	char line[MAX_LINE_SIZE];
	int i;

	while (fgets(line, sizeof(line), fp)) {
		new_size += strlen(line);

		for (i = 0; i < strlen(line); i++) {
			new_sum += line[i];
		}
	}

	if (sum != new_sum || size != new_size)
		return 1;

	return 0;
}


/*
 * Edit the passwords.
 *
 * This function dumps all the plain-text passwords ("results") in a temporary
 * file in your * ~/.mdp/ folder, fires your editor and save the output back
 * to your password file.
 */
void
edit_results()
{
	int i, tmp_fd = -1;
	struct result *result;
	uint32_t sum = 0, size = 0;
	char line[MAX_LINE_SIZE];

	if (atexit(atexit_cleanup) != 0)
		err(1, "get_results atexit");

	signal(SIGINT, sig_cleanup);
	signal(SIGKILL, sig_cleanup);

	/* Create the temporary file for edit mode. */
	snprintf(tmp_path, MAXPATHLEN, "%ls/.mdp/tmp_edit.XXXXXXXX", home);
	tmp_fd = mkstemp(tmp_path);
	if (tmp_fd == -1) {
		err(1, "edit_results mkstemp()");
	}

	/* Iterate over the results and dump them in this file. */
	for (i = 0; i < ARRAY_LENGTH(&results); i++) {
		result = ARRAY_ITEM(&results, i);
		wcstombs(line, result->value, sizeof(line));
		if (write(tmp_fd, line, strlen(line)) == -1)
			err(1, "edit_results write");
	}

	if (close(tmp_fd) != 0) {
		err(1, "edit_results close(tmp_fd)");
	}

	spawn_editor(tmp_path);

	if (has_changed(tmp_path, sum, size)) {
		gpg_encrypt(tmp_path);
	} else {
		fprintf(stderr, "No changes, exiting...\n");
	}
}


void
print_results()
{
	int i;
	struct result *result;

	for (i = 0; i < ARRAY_LENGTH(&results); i++) {
		result = ARRAY_ITEM(&results, i);
		if (result->status == RESULT_SHOW)
			printf("%ls\n", result->value);
	}
}


void
print_passwords(int length, int count)
{
	char password[MAX_PASSWORD_LENGTH];
	int i;

	for (i = 0; i < count; i++) {
		generate_password(password, length, "NCL");
		printf("%s\n", password);
	}
}


void
usage()
{
	printf("usage: mdp [-ecrVh] [-g len] [keyword ...]\n");
	exit(-1);
}


int
main(int ac, char **av)
{
	char *t;
	int opt, mode = MODE_PAGER;
	extern int optind;
	extern char *optarg;

	if (ac < 2)
		usage();

	setlocale(LC_ALL, "");

	/* Populate $HOME */
	t = getenv("HOME");
	if (t == NULL || *t == '\0')
		errx(0, "Unknown variable '$HOME'.");
	mbstowcs(home, t, MAXPATHLEN);

	/* Populate $EDITOR */
	t = getenv("EDITOR");
	if (t != NULL)
		mbstowcs(editor, t, MAXPATHLEN);

	while ((opt = getopt(ac, av, "hVdeg:qrc:")) != -1) {
		switch (opt) {
		case 'd':
			cfg_debug = 1;
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
			swprintf(cfg_config_path, MAXPATHLEN, L"%s", optarg);
			break;
		default:
			usage();
		}
	}

	debug("read config");

	config_set_defaults();
	config_check_paths();
	config_read();

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
			load_results();
			filter_results();
			print_results();
			break;

		case MODE_PAGER:
			debug("mode: MODE_PAGER");
			if (ac == 0)
				usage();

			gpg_check();
			load_results();
			filter_results();
			pager(START_WITHOUT_PROMPT);
			break;

		case MODE_QUERY:
			debug("mode: MODE_QUERY");
			gpg_check();
			load_results(mode);
			pager(START_WITH_PROMPT);
			break;

		case MODE_EDIT:
			debug("mode: MODE_EDIT");
			if (ac != 0)
				usage();

			gpg_check();
			lock_set();
			load_results();
			edit_results();
			break;

		case MODE_GENERATE:
			debug("mode: MODE_GENERATE");
			if (ac != 0)
				usage();

			print_passwords(password_length, cfg_password_count);
			break;

		default:
			errx(1, "unknown mode");
			break;
	}

	debug("normal shutdown");

	return 0;
}
