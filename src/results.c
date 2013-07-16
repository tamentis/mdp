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
 *
 *
 * This file contains all the tools to handle the result set.
 */

#include <sys/types.h>
#include <sys/param.h>

#include <wchar.h>
#include <stdlib.h>
#include <stdarg.h>
#include <err.h>
#include <strings.h>

#include "xmalloc.h"
#include "array.h"
#include "results.h"
#include "keywords.h"
#include "utils.h"
#include "gpg.h"


#define KEYWORD_LENGTH 50


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
	wchar_t kw[KEYWORD_LENGTH];


	for (i = 0; i < ARRAY_LENGTH(&keywords); i++) {
		mbstowcs(kw, ARRAY_ITEM(&keywords, i), KEYWORD_LENGTH);

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


/*
 * Populate the results array and return the number of lines.
 */
int
load_results()
{
	int i, line_count = 0;
	uint32_t sum = 0, size = 0;
	wchar_t wline[MAX_LINE_SIZE];
	char line[MAX_LINE_SIZE];
	FILE *fp;

	fp = gpg_open();

	while (fp != NULL && fgets(line, sizeof(line), fp)) {
		line_count++;

                /* One of the line may not have been read completely. */
		if (strchr(line, '\n') == NULL) {
			fprintf(stderr, "WARNING: Line %d is too long (max:%ld) "
					"or does not end with a new line.",
					line_count, sizeof(line));
		}

		size += strlen(line);

		for (i = 0; i < strlen(line); i++) {
			sum += line[i];
		}

		mbstowcs(wline, line, sizeof(line));
		strip_trailing_whitespaces(wline);

		ARRAY_ADD(&results, result_new(wline));
	}

	/* This happens when the password file does not exist yet. */
	if (fp != NULL)
		gpg_close(fp);

	if (ARRAY_LENGTH(&results) == 0)
		errx(1, "no passwords");

	return line_count;
}
