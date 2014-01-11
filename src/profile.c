/*
 * Copyright (c) 2013-2014 Bertrand Janin <b@janin.com>
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

#include <sys/types.h>
#include <sys/param.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
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
 * Instantiate a new profile with default values from the global scope.
 */
struct profile *
profile_new(const char *name)
{
	struct profile *new;

	new = xcalloc(1, sizeof(struct profile));
	new->name = strdup(name);
	new->password_count = cfg_password_count;
	new->character_count = cfg_character_count;

	if (cfg_character_set != NULL) {
		new->character_set = strdup(cfg_character_set);
	} else {
		new->character_set = strdup(CHARSET_ALPHANUMERIC);
	}

	return new;
}


/*
 * Return a profile from the global profile list.
 *
 * If the name is not found in the registry, this funtion returns NULL. Calling
 * this function with the default profile name as parameter will never return
 * NULL as a default profile is generated.
 */
struct profile *
profile_get_from_name(const char *name)
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
	unsigned int password_count;

	/* A password count specified on the command line takes precedence. */
	if (cmd_password_count > 0) {
		password_count = cmd_password_count;
	} else {
		password_count = profile->password_count;
	}

	for (unsigned int i = 0; i < password_count; i++) {
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
	unsigned int character_count;

	/* A character count specified on the command line takes precedence. */
	if (cmd_character_count > 0) {
		character_count = cmd_character_count;
	} else {
		character_count = profile->character_count;
	}

	s = xcalloc(character_count + 1, sizeof(char));

	retcode = generate_password_from_set(s, character_count,
			profile->character_set);
	if (retcode != 0) {
		errx(EXIT_FAILURE, "failed to generate password");
	}

	return s;
}


/*
 * Adds the provided profile to the global profile registry.
 */
void
profile_register(struct profile *p)
{
	if (p == NULL) {
		errx(EXIT_FAILURE, "profile_register(NULL)");
	}

	ARRAY_ADD(&profiles, p);
}
