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

#include <sys/termios.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <wchar.h>
#include <limits.h>
#include <errno.h>
#include <err.h>
#include <locale.h>
#include <inttypes.h>
#include <curses.h>

#include "utils.h"


#define QUOTE		L"\""


extern wchar_t   cfg_config_path[MAXPATHLEN];
extern wchar_t	 cfg_gpg_path[MAXPATHLEN];
extern wchar_t   cfg_gpg_key_id[MAXPATHLEN];
extern wchar_t   cfg_editor[MAXPATHLEN];
extern int	 cfg_timeout;

extern wchar_t	 passwords_path[MAXPATHLEN];
extern wchar_t	 lock_path[MAXPATHLEN];
extern wchar_t	 home[MAXPATHLEN];


/* Utility functions from OpenBSD/SSH in separate files (ISC license) */
size_t		 wcslcpy(wchar_t *, const wchar_t *, size_t);
wchar_t		*strdelim(wchar_t **);


/*
 * Sets the value of the given variable, also do some type check
 * just in case.
 */
void
set_variable(wchar_t *name, wchar_t *value, int linenum)
{
	/* set gpg_path <string> */
	if (wcscmp(name, L"gpg_path") == 0) {
		if (value == NULL || *value == '\0') {
			*cfg_gpg_path = '\0';
			return;
		}
		wcslcpy(cfg_gpg_path, value, MAXPATHLEN);

	/* set gpg_key_id <string> */
	} else if (wcscmp(name, L"gpg_key_id") == 0) {
		if (value == NULL || *value == '\0') {
			*cfg_gpg_key_id = '\0';
			return;
		}
		wcslcpy(cfg_gpg_key_id, value, MAXPATHLEN);

	/* set editor <string> */
	} else if (wcscmp(name, L"editor") == 0) {
		if (value == NULL || *value == '\0') {
			*cfg_editor = '\0';
			return;
		}
		wcslcpy(cfg_editor, value, MAXPATHLEN);

	/* set timeout <integer> */
	} else if (wcscmp(name, L"timeout") == 0) {
		if (value == NULL || *value == '\0')
			errx(1, "config:%d: invalid value for timeout\n",
					linenum);

		cfg_timeout = wcstoumax(value, NULL, 10);

	/* ??? */
	} else {
		errx(1, "config: unknown variable for set on line %d.\n",
				linenum);
	}
}


/*
 * Parse a single line of the configuration file.
 *
 * Returns 0 on success or anything else if an error occurred, it will be rare
 * since most fatal errors will quit the program with an error message anyways.
 */
int
process_config_line(wchar_t *line, int linenum)
{
	wchar_t *keyword, *name, *value;

	strip_trailing_whitespaces(line);

	/* Get the keyword (each line is supposed to begin with a keyword). */
	if ((keyword = strdelim(&line)) == NULL)
		return 0;

	/* Ignore leading whitespace. */
	if (*keyword == '\0')
		keyword = strdelim(&line);

	if (keyword == NULL || !*keyword || *keyword == '\n' || *keyword == '#')
		return 0;

	/* set varname value */
	if (wcscmp(keyword, L"set") == 0) {
		if ((name = strdelim(&line)) == NULL) {
			errx(1, "%ls: set without variable name on line %d.\n",
					cfg_config_path, linenum);
			return -1;
		}
		value = strdelim(&line);
		set_variable(name, value, linenum);

	/* Unknown operation... Code help us. */
	} else {
		errx(1, "%ls: unknown command on line %d.\n", cfg_config_path,
				linenum);
		return -1;
	}

	return 0;
}

/*
 * Creates and/or check the configuration directory.
 *
 * Exits program with error message if anything is wrong.
 */
void
config_check_directory(wchar_t *path)
{
	struct stat sb;
	char mbs_path[MAXPATHLEN];

	wcstombs(mbs_path, path, MAXPATHLEN);

	if (stat(mbs_path, &sb) != 0) {
		if (errno == ENOENT) {
			if (mkdir(mbs_path, 0700) != 0) {
				errx(1, "can't create %ls: %s", path,
						strerror(errno));
			}
			if (stat(mbs_path, &sb) != 0) {
				errx(1, "can't stat newly created %ls: %s",
						path, strerror(errno));
			}
		} else {
			errx(1, "can't access %ls: %s", path, strerror(errno));
		}
	}

	if (!S_ISDIR(sb.st_mode))
		errx(1, "%ls is not a directory", path);

	if (sb.st_uid != 0 && sb.st_uid != getuid())
		errx(1, "bad owner on %ls", path);

	if ((sb.st_mode & 022) != 0)
		errx(1, "bad permissions on %ls", path);
}


/*
 * Check the configuration file.
 *
 * Exits program with error message if anything is wrong.
 */
void
config_check_file(wchar_t *path)
{
	struct stat sb;
	char mbs_path[MAXPATHLEN];

	wcstombs(mbs_path, path, MAXPATHLEN);

	if (stat(mbs_path, &sb) != 0) {
		/* User hasn't created a config file, that's perfectly fine. */
		if (errno == ENOENT) {
			return;
		} else {
			errx(1, "can't access %ls: %s", path, strerror(errno));
		}
	}

	if (!S_ISREG(sb.st_mode))
		errx(1, "%ls is not a regular file", path);

	if (sb.st_uid != 0 && sb.st_uid != getuid())
		errx(1, "bad owner on %ls", path);

	if ((sb.st_mode & 022) != 0)
		errx(1, "bad permissions on %ls", path);
}


/*
 * Prepare and check the configuration paths.
 *
 * Create the ~/.mdp/ directory if it doesn't exist yet, then make sure it has
 * the right permissions, including all the relevant files within.
 */
void
config_check_paths()
{
	FILE *fp;
	char line[128];
	wchar_t wline[128];
	int linenum = 1;
	wchar_t path[MAXPATHLEN];
	char mbs_path[MAXPATHLEN];

	swprintf(path, MAXPATHLEN, L"%ls/.mdp", home);
	config_check_directory(path);

	swprintf(passwords_path, MAXPATHLEN, L"%ls/.mdp/passwords", home);
	config_check_file(passwords_path);

	swprintf(path, MAXPATHLEN, L"%ls/.mdp/config", home);
	config_check_file(path);

	wcstombs(mbs_path, path, MAXPATHLEN);
	fp = fopen(mbs_path, "r");
	if (fp == NULL)
		return;

	while (fgets(line, sizeof(line), fp)) {
		mbstowcs(wline, line, 128);
		process_config_line(wline, linenum++);
	}

	fclose(fp);
}


/*
 * Validates all the variables.
 */
void
config_check_variables()
{
	struct stat sb;
	char mbs_path[MAXPATHLEN];

	wcstombs(mbs_path, cfg_gpg_path, MAXPATHLEN);

	if (stat(mbs_path, &sb) != 0)
		err(1, "config: wrong gpg path (or not installed) %s", mbs_path);
}


void
config_set_defaults()
{
	if (wcslen(cfg_config_path) == 0)
		swprintf(cfg_config_path, MAXPATHLEN, L"%ls/.mdp/config", home);

	swprintf(lock_path, MAXPATHLEN, L"%ls/.mdp/lock", home);
}


/*
 * Open the file and feed each line one by one to process_config_line.
 */
void
config_read()
{
	FILE *fp;
	char line[128];
	wchar_t wline[128];
	int linenum = 1;
	char path[MAXPATHLEN];

	wcstombs(path, cfg_config_path, MAXPATHLEN);

	fp = fopen(path, "r");
	if (fp == NULL)
		return;

	while (fgets(line, sizeof(line), fp)) {
		mbstowcs(wline, line, 128);
		process_config_line(wline, linenum++);
	}

	fclose(fp);
}
