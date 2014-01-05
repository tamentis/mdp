/*
 * Copyright (c) 2013 Bertrand Janin <b@janin.com>
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
 *
 *
 * All the profile related variables and functions.
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <err.h>

#include "array.h"
#include "cmd.h"
#include "config.h"
#include "profile.h"
#include "randpass.h"
#include "xmalloc.h"


struct profile_list profiles = ARRAY_INITIALIZER;


/*
 * Instantiate a new profile with default values.
 */
struct profile *
profile_new(char *name)
{
	struct profile *new;

	new = xcalloc(1, sizeof(struct profile));
	new->name = strdup(name);
	new->password_count = cfg_password_count;
	new->character_count = cmd_password_length;
	new->character_set = strdup(CHARSET_ALPHANUMERIC);

	return new;
}


/*
 * Return a profile from the global profile list.
 *
 * If the name is not found in the registry, this funtion returns NULL.
 */
struct profile *
profile_get_from_name(char *name)
{
	struct profile *profile;

	for (unsigned int i = 0; i < ARRAY_LENGTH(&profiles); i++) {
		profile = ARRAY_ITEM(&profiles, i);

		if (strcmp(profile->name, name) == 0) {
			return profile;
		}
	}

	return NULL;
}


/*
 * Print a set of passwords from the profile definition.
 */
void
profile_fprint_passwords(FILE *stream, struct profile *profile)
{
	for (unsigned int i = 0; i < profile->password_count; i++) {
		char *password;
		password = profile_generate_password(profile);
		fprintf(stream, "%s\n", password);
		xfree(password);
	}
}


/*
 * Generate and return a single password based on the profile definition.
 *
 * Callers are responsible for free'ing the memory.
 */
char *
profile_generate_password(struct profile *profile)
{
	char *s;
	int retcode;

	s = xcalloc(profile->character_count + 1, sizeof(char));

	retcode = generate_password_from_set(s, profile->character_count,
			profile->character_set);
	if (retcode != 0) {
		errx(EXIT_FAILURE, "failed to generate password");
	}

	return s;
}
