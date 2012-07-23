/*
 * Copyright (c) 2012 Bertrand Janin <b@grun.gy>
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
#include <locale.h>
#include <inttypes.h>

#include "utils.h"
#include "config.h"
#include "pager.h"
#include "gpg.h"


wchar_t	 cfg_gpg_path[MAXPATHLEN] = L"/usr/bin/gpg";
wchar_t	 cfg_gpg_key_id[MAXPATHLEN] = L"";
wchar_t	 cfg_editor[MAXPATHLEN] = L"";
int	 cfg_timeout = 10;

wchar_t	 home[MAXPATHLEN];
wchar_t	 passwords_path[MAXPATHLEN];
wchar_t	 editor[MAXPATHLEN];


/* Utility functions from OpenBSD/SSH in separate files (ISC license) */
size_t		 wcslcpy(wchar_t *, const wchar_t *, size_t);
wchar_t		*strdelim(wchar_t **);
size_t		 strlcpy(char *, const char *, size_t);

void		 shutdown_curses();


enum action_mode {
	MODE_PAGER,
	MODE_RAW,
	MODE_EDIT,
	MODE_CREATE
};


/*
 * Spawn the editor on a file.
 *
 * Not quite implemented...
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
	system(s);
}


/*
 * Check if the line matches all the keywords.
 */
int
line_matches(wchar_t *line, char **keywords)
{
	int matches = 1;
	char **raw_keyword = keywords;
	wchar_t keyword[128];

	while (*raw_keyword != NULL) {
		mbstowcs(keyword, *raw_keyword, 128);
		if (wcsstr((const wchar_t *)line,
					(const wchar_t *)keyword) == NULL) {
			matches = 0;
			break;
		}
		raw_keyword++;
	}

	return matches;
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
	char line[256];
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


void
get_results(char **keywords, int mode)
{
	int status, idx = 0, i, tmp_fd = -1;
	uint32_t sum = 0, size = 0;
	wchar_t wline[256], *results[RESULTS_MAX_LEN];
	char tmp_path[MAXPATHLEN], line[256];
	FILE *fp;

	fp = gpg_open();

	if (mode == MODE_EDIT) {
		snprintf(tmp_path, MAXPATHLEN, "%ls/.mdp/tmp_edit.XXXXXXXX",
				home);
		tmp_fd = mkstemp(tmp_path);
		if (tmp_fd == -1)
			err(1, "get_results mkstemp()");
	}

	while (fgets(line, sizeof(line), fp)) {
		size += strlen(line);

		for (i = 0; i < strlen(line); i++) {
			sum += line[i];
		}

		if (mode == MODE_EDIT) {
			write(tmp_fd, line, strlen(line));
			continue;
		}

		mbstowcs(wline, line, 128);
		strip_trailing_whitespaces(wline);
		if (line_matches(wline, keywords)) {
			switch (mode) {
				case MODE_PAGER:
					if (idx < RESULTS_MAX_LEN - 1)
						results[idx] = wcsdup(wline);
					idx++;
					break;

				case MODE_RAW:
					printf("%ls\n", wline);
					break;

				default:
					break;
			}
		}
	}

	gpg_close(fp, &status);

	switch (mode) {
		case MODE_PAGER:
			show_results_in_pager(idx, results);
			break;

		case MODE_EDIT:
			if (close(tmp_fd) != 0)
				err(1, "get_results close(tmp_fd)");

			spawn_editor(tmp_path);

			if (has_changed(tmp_path, sum, size)) {
				gpg_encrypt(tmp_path);
			} else {
				fprintf(stderr, "No changes, exiting...\n");
			}
			break;

		default:
			break;
	}
}


void
usage()
{
	printf("usage: mdp [-ecr] [keyword ...]\n");
	exit(-1);
}


int
main(int ac, char **av)
{
	char *t;
	int opt, mode = MODE_PAGER;
	extern int optind, optreset;

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
	mbstowcs(editor, t, MAXPATHLEN);

	config_check_paths();
	config_read();
	config_check_variables();

	while ((opt = getopt(ac, av, "ecr")) != -1) {
		switch (opt) {
		case 'e':
			mode = MODE_EDIT;
			break;
		case 'c':
			mode = MODE_CREATE;
			break;
		case 'r':
			mode = MODE_RAW;
			break;
		default:
			usage();
		}
	}

	ac -= optind;
	av += optind;

	/* Decide if we use the internal pager or just dump to screen. */
	switch (mode) {
		case MODE_RAW:
			if (ac == 0)
				usage();

			get_results(av, mode);
			break;

		case MODE_PAGER:
			if (ac == 0)
				usage();

			get_results(av, mode);
			break;

		case MODE_EDIT:
			if (ac != 0)
				usage();

			get_results(av, mode);
			break;

		default:
			errx(1, "unknown mode");
			break;
	}

	return 0;
}
