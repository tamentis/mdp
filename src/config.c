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
#include <wchar.h>

#include "array.h"
#include "cmd.h"
#include "config.h"
#include "gpg.h"
#include "lock.h"
#include "mdp.h"
#include "profile.h"
#include "str.h"
#include "strdelim.h"
#include "utils.h"
#include "xmalloc.h"


char		*cfg_gpg_path = NULL;
char		*cfg_gpg_key_id = NULL;
unsigned int	 cfg_gpg_timeout = 20;
char		*cfg_editor;
unsigned int	 cfg_timeout = 10;
unsigned int	 cfg_password_count = DEFAULT_PASSWORD_COUNT;
unsigned int	 cfg_character_count = DEFAULT_CHARACTER_COUNT;
wchar_t		*cfg_character_set = NULL;
bool		 cfg_backup = true;


#define parse_boolean(v) (v != NULL && *v == 'o') ? true : false
#define conf_err(m) errx(EXIT_FAILURE, "config:%d: " m, linenum)


/*
 * Given a value fetched from the configuration file, return either the content
 * of a pre-defined character set (start with '$') or the value itself.
 */
wchar_t *
config_resolve_character_set(const char *value)
{
	wchar_t *character_set = NULL;

	if (value[0] == '$') {
		if (streq(value, "$DIGITS")) {
			character_set = wcsdup(CHARSET_DIGITS);
		} else if (streq(value, "$LOWERCASE")) {
			character_set = wcsdup(CHARSET_LOWERCASE);
		} else if (streq(value, "$UPPERCASE")) {
			character_set = wcsdup(CHARSET_UPPERCASE);
		} else if (streq(value, "$ALPHA")) {
			character_set = wcsdup(CHARSET_ALPHA);
		} else if (streq(value, "$ALPHANUMERIC")) {
			character_set = wcsdup(CHARSET_ALPHANUMERIC);
		} else if (streq(value, "$SYMBOLS")) {
			character_set = wcsdup(CHARSET_SYMBOLS);
		} else if (streq(value, "$PRINTABLE")) {
			character_set = wcsdup(CHARSET_PRINTABLE);
		}
	}

	if (character_set == NULL) {
		character_set = mbs_duplicate_as_wcs(value);
	}

	return character_set;
}


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
			xfree(cfg_editor);
		}

		if (value == NULL || *value == '\0') {
			conf_err("invalid value for editor");
		}
		cfg_editor = strdup(value);

	/* set password_count <integer> */
	} else if (strcmp(name, "password_count") == 0) {
		if (value == NULL || *value == '\0') {
			conf_err("invalid value for password_count");
		}

		cfg_password_count = strtoull(value, NULL, 10);

	/* set character_count <integer> */
	} else if (strcmp(name, "character_count") == 0) {
		if (value == NULL || *value == '\0') {
			conf_err("invalid value for character_count");
		}

		cfg_character_count = strtoull(value, NULL, 10);

	/* set character_set <string> */
	} else if (strcmp(name, "character_set") == 0) {
		if (cfg_character_set != NULL) {
			xfree(cfg_character_set);
		}

		if (value == NULL || *value == '\0') {
			conf_err("invalid value for character_set");
		}
		cfg_character_set = config_resolve_character_set(value);

	/* set timeout <integer> */
	} else if (strcmp(name, "timeout") == 0) {
		if (value == NULL || *value == '\0') {
			conf_err("invalid value for timeout");
		}

		cfg_timeout = strtoull(value, NULL, 10);

	/* set backup <bool> */
	} else if (strcmp(name, "backup") == 0) {
		cfg_backup = parse_boolean(value);

	/* ??? */
	} else {
		conf_err("unknown variable");
	}
}


/*
 * Sets the value of the given profile variable, also do some type check just
 * in case.
 */
static void
set_profile_variable(struct profile *profile, char *name, char *value,
		int linenum)
{
	/* set password_count <unsigned integer> */
	if (strcmp(name, "password_count") == 0) {
		if (value == NULL || *value == '\0') {
			conf_err("invalid value for password_count");
		}

		profile->password_count = strtoull(value, NULL, 10);

	/* set timeout <unsigned integer> */
	} else if (strcmp(name, "character_count") == 0) {
		if (value == NULL || *value == '\0') {
			conf_err("invalid value for character_count");
		}

		profile->character_count = strtoull(value, NULL, 10);

	/* set character_set <bool> */
	} else if (strcmp(name, "character_set") == 0) {
		if (profile->character_set != NULL) {
			xfree(profile->character_set);
		}

		if (value == NULL || *value == '\0') {
			conf_err("invalid value for character_set");
		}
		profile->character_set = config_resolve_character_set(value);

	/* ??? */
	} else {
		conf_err("unknown profile variable");
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
	static struct profile *current_profile = NULL;

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
			errx(EXIT_FAILURE, "%s: set without variable name on "
					"line %d.", cmd_config_path, linenum);
			return -1;
		}
		value = strdelim(&line);
		if (current_profile == NULL) {
			set_variable(name, value, linenum);
		} else {
			set_profile_variable(current_profile, name, value,
					linenum);
		}

	/* profile name */
	} else if (strcmp(keyword, "profile") == 0) {
		if ((name = strdelim(&line)) == NULL) {
			errx(EXIT_FAILURE, "%s: profile without variable name "
					"on line %d.", cmd_config_path,
					linenum);
			return -1;
		}

		/* Every set after that moment is to configure this profile. */
		current_profile = profile_new(name);
		profile_register(current_profile);

	/* Unknown operation... Code help us. */
	} else {
		errx(EXIT_FAILURE, "%s: unknown command on line %d.",
				cmd_config_path, linenum);
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
config_check_directory(const char *path)
{
	struct stat sb;

	if (stat(path, &sb) != 0) {
		if (errno == ENOENT) {
			if (mkdir(path, 0700) != 0) {
				err(EXIT_FAILURE, "can't create %s", path);
			}
			if (stat(path, &sb) != 0) {
				err(EXIT_FAILURE, "can't stat newly created "
						"file %s", path);
			}
		} else {
			err(EXIT_FAILURE, "can't access %s", path);
		}
	}

	if (!S_ISDIR(sb.st_mode))
		errx(EXIT_FAILURE, "%s is not a directory", path);

	if (sb.st_uid != 0 && sb.st_uid != getuid())
		errx(EXIT_FAILURE, "bad owner on %s", path);

	if ((sb.st_mode & 0077) != 0)
		errx(EXIT_FAILURE, "bad permissions on %s", path);
}


/*
 * Check the configuration file.
 *
 * Exits program with error message if anything is wrong.
 */
void
config_check_file(const char *path)
{
	struct stat sb;

	if (stat(path, &sb) != 0) {
		/* User hasn't created a config file, that's perfectly fine. */
		if (errno == ENOENT) {
			return;
		} else {
			err(EXIT_FAILURE, "can't access %s", path);
		}
	}

	if (!S_ISREG(sb.st_mode)) {
		errx(EXIT_FAILURE, "%s is not a regular file", path);
	}

	if (sb.st_uid != 0 && sb.st_uid != getuid()) {
		errx(EXIT_FAILURE, "bad owner on %s", path);
	}

	if ((sb.st_mode & 022) != 0) {
		errx(EXIT_FAILURE, "bad permissions on %s", path);
	}
}


/*
 * Prepare and check the configuration paths.
 *
 * Create the ~/.mdp/ directory if it doesn't exist yet, then make sure it has
 * the right permissions, including all the relevant files within.
 */
void
config_check_paths(const char *home)
{
	char *path;

	path = join_path(home, ".mdp");
	config_check_directory(path);
	xfree(path);

	gpg_passwords_path = join_path(home, ".mdp/passwords");
	config_check_file(gpg_passwords_path);

	path = join_path(home, ".mdp/config");
	config_check_file(path);
	xfree(path);
}


void
config_set_defaults(const char *home)
{
	if (cfg_gpg_path == NULL) {
		cfg_gpg_path = strdup("/usr/bin/gpg");
	}

	if (cmd_gpg_key_id != NULL) {
		if (cfg_gpg_key_id != NULL) {
			xfree(cfg_gpg_key_id);
		}
		cfg_gpg_key_id = cmd_gpg_key_id;
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
	static char line[MAX_LINE_SIZE];
	int linenum = 1;

	fp = fopen(cmd_config_path, "r");
	if (fp == NULL)
		return;

	while (fgets(line, sizeof(line), fp)) {
		process_config_line(line, linenum++);
	}

	fclose(fp);
}
