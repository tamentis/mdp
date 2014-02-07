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

#include <err.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "array.h"
#include "keywords.h"
#include "xmalloc.h"


struct kwlist keywords = ARRAY_INITIALIZER;


/*
 * Returns the count of keywords in the global array.
 */
unsigned int
keywords_count()
{
	return ARRAY_LENGTH(&keywords);
}


/*
 * Remove all the keywords from the global array.
 */
void
keywords_clear()
{
	for (unsigned int i = 0; i < ARRAY_LENGTH(&keywords); i++) {
		xfree(ARRAY_ITEM(&keywords, i));
	}

	ARRAY_CLEAR(&keywords);
}


/*
 * Extract keywords from a NULL-terminated list of strings.
 */
void
keywords_load_from_argv(char **av)
{
	char **kw = av;

	keywords_clear();

	while (*kw != NULL) {
		ARRAY_ADD(&keywords, strdup(*kw));
		kw++;
	}
}


/*
 * Extract keywords from a space-separated strings.
 */
void
keywords_load_from_char(char *kw)
{
	char *p, *last = NULL;

	keywords_clear();

	for ((p = strtok_r(kw, " ", &last)); p;
	    (p = strtok_r(NULL, " ", &last))) {
		ARRAY_ADD(&keywords, strdup(p));
	}
}
