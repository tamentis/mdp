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

#include <wchar.h>
#include <stdlib.h>
#include <stdarg.h>

#include "xmalloc.h"
#include "array.h"
#include "results.h"
#include "keywords.h"
#include "utils.h"


struct wlist results = ARRAY_INITIALIZER;
extern struct kwlist keywords;


/*
 * Instantiate a new result.
 *
 * These items won't be free'd, they will stay around until the program ends.
 */
struct result *
result_new(wchar_t *value)
{
	struct result *new;

	new = xmalloc(sizeof(struct result));
	new->status = RESULT_SHOW;
	new->value = wcsdup(value);

	return new;
}


/*
 * Count of results with visible status (RESULT_SHOW).
 */
int
results_visible_length()
{
	int i, len = 0;
	struct result *result;

	for (i = 0; i < ARRAY_LENGTH(&results); i++) {
		result = ARRAY_ITEM(&results, i);
		if (result->status == RESULT_SHOW)
			len++;
	}

	return len;
}


/*
 * Check if the line matches all the keywords.
 */
int
line_matches(const wchar_t *line)
{
	int i, matches = 1;
	wchar_t kw[50];


	for (i = 0; i < ARRAY_LENGTH(&keywords); i++) {
		mbstowcs(kw, ARRAY_ITEM(&keywords, i), 50);

		if (wcscasestr(line, (const wchar_t *)kw) == NULL) {
			matches = 0;
			break;
		}
	}

	return matches;
}


int
get_widest_result()
{
	int i, len = 0, res_len;
	struct result *result;

	for (i = 0; i < ARRAY_LENGTH(&results); i++) {
		result = ARRAY_ITEM(&results, i);
		res_len = wcslen(result->value);

		if (result->status != RESULT_SHOW)
			continue;

		if (res_len > len)
			len = res_len;
	}

	return len;
}


/*
 * Filter results from ARRAY.
 *
 * Given the keywords, set the status on individual results in the current set.
 */
void
filter_results()
{
	int i;
	struct result *result;

	for (i = 0; i < ARRAY_LENGTH(&results); i++) {
		result = ARRAY_ITEM(&results, i);

		if (line_matches((const wchar_t *)result->value)) {
			result->status = RESULT_SHOW;
		} else {
			result->status = RESULT_HIDDEN;
		}
	}
}

