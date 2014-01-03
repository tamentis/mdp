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
#include <stdbool.h>

#include "xmalloc.h"
#include "array.h"
#include "profile.h"
#include "cmd.h"
#include "config.h"


struct profile_list profiles = ARRAY_INITIALIZER;


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


void
profile_generate_from_name(char *name)
{
	struct profile *profile;

	profile = profile_get_from_name(name);
}
