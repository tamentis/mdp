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
#include <limits.h>
#include <errno.h>
#include <err.h>
#include <locale.h>
#include <inttypes.h>
#include <curses.h>

#include "mdp.h"
#include "utils.h"
#include "config.h"
#include "strdelim.h"
#include "xmalloc.h"


extern char	*cfg_config_path;
extern char	*cfg_gpg_path;
extern char	*cfg_gpg_key_id;
extern int	 cfg_gpg_timeout;
extern char	*cfg_editor;
extern int	 cfg_password_count;
extern int	 cfg_backup;
extern int	 cfg_timeout;

extern char	*passwords_path;
extern char	*lock_path;
extern char	*home;


#define get_boolean(v) (v != NULL && *v == 'o') ? 1 : 0
#define conf_err(m) errx(100, "config:%d: " m, linenum)


/*
 * Sets the value of the given variable, also do some type check
 * just in case.
 */
static void
set_variable(char *name, char *value, int linenum)
{
	/* set gpg_path <string> */
	if (strcmp(name, "gpg_path") == 0) {
		if (cfg_gpg_path != NULL) {
			conf_err("gpg_path defined multiple times");
		}

		if (value == NULL || *value == '\0') {
			conf_err("invalid value for gpg_path");
		}

		cfg_gpg_path = strdup(value);

	/* set gpg_key_id <string> */
	} else if (strcmp(name, "gpg_key_id") == 0) {
		if (cfg_gpg_key_id != NULL) {
			conf_err("gpg_key_id defined multiple times");
		}

		if (value == NULL || *value == '\0') {
			conf_err("invalid value for gpg_key_id");
		}

		cfg_gpg_key_id = strdup(value);

	/* set gpg_timeout <integer> */
	} else if (strcmp(name, "gpg_timeout") == 0) {
		if (value == NULL || *value == '\0') {
			conf_err("invalid value for gpg_timeout");
		}

		cfg_gpg_timeout = strtoull(value, NULL, 10);

	/* set editor <string> */
	} else if (strcmp(name, "editor") == 0) {
		if (cfg_editor != NULL) {
			conf_err("editor defined multiple times");
		}

		if (value == NULL || *value == '\0') {
			conf_err("invalid value for gpg_timeout");
		}
		cfg_editor = strdup(value);

	/* set password_count <integer> */
	} else if (strcmp(name, "password_count") == 0) {
		if (value == NULL || *value == '\0') {
			conf_err("invalid value for password_count");
		}

		cfg_password_count = strtoull(value, NULL, 10);

	/* set timeout <integer> */
	} else if (strcmp(name, "timeout") == 0) {
		if (value == NULL || *value == '\0')
			errx(1, "config:%d: invalid value for timeout\n",
					linenum);

		cfg_timeout = strtoull(value, NULL, 10);

	/* set backup <bool> */
	} else if (strcmp(name, "backup") == 0) {
		cfg_backup = get_boolean(value);

	/* ??? */
	} else {
		conf_err("unknown variable");
	}
}


/*
 * Parse a single line of the configuration file.
 *
 * Returns 0 on success or anything else if an error occurred, it will be rare
 * since most fatal errors will quit the program with an error message anyways.
 */
static int
process_config_line(char *line, int linenum)
{
	char *keyword, *name, *value;

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
	if (strcmp(keyword, "set") == 0) {
		if ((name = strdelim(&line)) == NULL) {
			errx(1, "%s: set without variable name on line %d.\n",
					cfg_config_path, linenum);
			return -1;
		}
		value = strdelim(&line);
		set_variable(name, value, linenum);

	/* Unknown operation... Code help us. */
	} else {
		errx(1, "%s: unknown command on line %d.\n", cfg_config_path,
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
config_check_directory(char *path)
{
	struct stat sb;

	if (stat(path, &sb) != 0) {
		if (errno == ENOENT) {
			if (mkdir(path, 0700) != 0) {
				errx(1, "can't create %s: %s", path,
						strerror(errno));
			}
			if (stat(path, &sb) != 0) {
				errx(1, "can't stat newly created %s: %s",
						path, strerror(errno));
			}
		} else {
			errx(1, "can't access %s: %s", path, strerror(errno));
		}
	}

	if (!S_ISDIR(sb.st_mode))
		errx(1, "%s is not a directory", path);

	if (sb.st_uid != 0 && sb.st_uid != getuid())
		errx(1, "bad owner on %s", path);

	if ((sb.st_mode & 022) != 0)
		errx(1, "bad permissions on %s", path);
}


/*
 * Check the configuration file.
 *
 * Exits program with error message if anything is wrong.
 */
void
config_check_file(char *path)
{
	struct stat sb;

	if (stat(path, &sb) != 0) {
		/* User hasn't created a config file, that's perfectly fine. */
		if (errno == ENOENT) {
			return;
		} else {
			errx(1, "can't access %s: %s", path, strerror(errno));
		}
	}

	if (!S_ISREG(sb.st_mode))
		errx(1, "%s is not a regular file", path);

	if (sb.st_uid != 0 && sb.st_uid != getuid())
		errx(1, "bad owner on %s", path);

	if ((sb.st_mode & 022) != 0)
		errx(1, "bad permissions on %s", path);
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
	char *path;

	path = join_path(home, ".mdp");
	config_check_directory(path);
	xfree(path);

	passwords_path = join_path(home, ".mdp/passwords");
	config_check_file(passwords_path);

	path = join_path(home, ".mdp/config");
	config_check_file(path);
}


void
config_set_defaults()
{
	if (cfg_gpg_path == NULL) {
		cfg_gpg_path = strdup("/usr/bin/gpg");
	}

	lock_path = join_path(home, ".mdp/lock");
}


/*
 * Open the file and feed each line one by one to process_config_line.
 */
void
config_read()
{
	FILE *fp;
	// FIXME remove the limit here.
	char line[128];
	int linenum = 1;

	fp = fopen(cfg_config_path, "r");
	if (fp == NULL)
		return;

	while (fgets(line, sizeof(line), fp)) {
		process_config_line(line, linenum++);
	}

	fclose(fp);
}
