/*
 * Copyright (c) 2012-2015 Bertrand Janin <b@janin.com>
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

#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <regex.h>

#include "cmd.h"
#include "crc.h"
#include "gpg.h"
#include "keywords.h"
#include "mdp.h"
#include "results.h"
#include "str.h"
#include "xmalloc.h"

#if defined(HAS_NO_WCSDUP)
# include "wcsdup.h"
#endif


struct wlist results = ARRAY_INITIALIZER;

/* This crc32 is used to check if a file has changed after edit. */
uint32_t result_crc32 = 0;


/*
 * Instantiate a new result.
 *
 * These items won't be free'd, they will stay around until the program ends.
 * We convert and measure everything from here, unless you have a million
 * password this shouldn't be much of a bottleneck.
 */
struct result *
result_new(const wchar_t *value)
{
	struct result *new;

	new = xmalloc(sizeof(struct result));
	new->visible = true;
	new->wcs_value = wcsdup(value);
	new->mbs_value = wcs_duplicate_as_mbs(value);
	if (new->mbs_value == NULL) {
		return (NULL);
	}
	new->wcs_len = wcslen(value);
	new->mbs_len = strlen(new->mbs_value);


	return (new);
}


/*
 * Count of visible results.
 */
unsigned int
results_visible_length()
{
	unsigned int len = 0;
	struct result *result;

	for (unsigned int i = 0; i < ARRAY_LENGTH(&results); i++) {
		result = ARRAY_ITEM(&results, i);
		if (result->visible)
			len++;
	}

	return (len);
}


/*
 * Check if the line matches all the keywords.
 */
static bool
line_matches_plain(const wchar_t *line)
{
	bool matches = true;
	wchar_t kw[KEYWORD_LENGTH];

	for (unsigned int i = 0; i < ARRAY_LENGTH(&keywords); i++) {
		mbstowcs(kw, ARRAY_ITEM(&keywords, i), KEYWORD_LENGTH);

		if (wcscasestr(line, (const wchar_t *)kw) == NULL) {
			matches = false;
			break;
		}
	}

	return (matches);
}


/*
 * Check if the line matches all the regexes. This is not optimize as the
 * regexes are compiled from scratch every time. It's okay, it makes the
 * implementation simpler.
 */
static int
line_matches_regex(const wchar_t *line)
{
	bool matches = true;
	int flags = 0;
	static char mbs_line[MAX_LINE_SIZE];
	regex_t preg;

	for (unsigned int i = 0; i < ARRAY_LENGTH(&keywords); i++) {
		wcstombs(mbs_line, line, sizeof(mbs_line));

		if (regcomp(&preg, ARRAY_ITEM(&keywords, i), flags) != 0)
			err(EXIT_FAILURE, "line_matches_regex");

		if (regexec(&preg, mbs_line, 0, NULL, 0) != 0) {
			matches = false;
			regfree(&preg);
			break;
		}

		regfree(&preg);
	}

	return (matches);
}


/*
 * Check if the line matches all the keywords.
 *
 * Commented lines are excluded by default.
 */
static bool
line_matches(const wchar_t *line)
{
	if (line[0] == L'#') {
		return (false);
	}

	if (cmd_regex) {
		return line_matches_regex(line);
	} else {
		return line_matches_plain(line);
	}
}


unsigned int
get_max_length()
{
	unsigned int maxlen = 0;
	struct result *result;

	for (unsigned int i = 0; i < ARRAY_LENGTH(&results); i++) {
		result = ARRAY_ITEM(&results, i);

		if (!result->visible)
			continue;

		if (result->wcs_len > maxlen)
			maxlen = result->wcs_len;
	}

	return (maxlen);
}


/*
 * Filter results from ARRAY.
 *
 * Given the keywords, set the status on individual results in the current set.
 */
void
filter_results()
{
	struct result *result;

	for (unsigned int i = 0; i < ARRAY_LENGTH(&results); i++) {
		result = ARRAY_ITEM(&results, i);

		if (line_matches(result->wcs_value)) {
			result->visible = true;
		} else {
			result->visible = false;
		}
	}
}


/*
 * Populate the results array and return the number of lines.
 */
int
load_results_fp(FILE *fp)
{
	unsigned int line_count = 0;
	static wchar_t wline[MAX_LINE_SIZE];
	static char line[MAX_LINE_SIZE];
	struct result *result;
	CKSUM_CTX crcctx;

	CKSUM_Init(&crcctx);

	while (fp != NULL && fgets(line, sizeof(line), fp)) {
		line_count++;

                /* One of the line may not have been read completely. */
		if (strchr(line, '\n') == NULL) {
			fprintf(stderr, "WARNING: Line %d is too long "
					"(max:%ld) or does not end with a new "
					"line.\n", line_count, sizeof(line));
		}

		CKSUM_Update(&crcctx, (unsigned char *)line, strlen(line));

		mbstowcs(wline, line, sizeof(line));
		wcs_strip_trailing_whitespaces(wline);

		result = result_new(wline);
		if (result == NULL) {
			errx(EXIT_FAILURE, "unable to read line %d with the "
					"current locale.", line_count);
		}
		ARRAY_ADD(&results, result);
	}

	CKSUM_Final(&crcctx);
	result_crc32 = crcctx.crc;

	return ARRAY_LENGTH(&results);
}


/*
 * Load the results from the main GnuPG encrypted password file and return the
 * number of results.
 *
 * Returns -1 if GnuPG did not return successfully.
 */
int
load_results_gpg()
{
	int length;
	FILE *fp;

	fp = gpg_open();

	/* Password file does not exist yet. */
	if (fp == NULL)
		return (0);

	length = load_results_fp(fp);

	/* GnuPG exited with an non-zero return code. */
	if (gpg_close(fp) != 0) {
		errx(EXIT_FAILURE, "GnuPG returned with an error");
	}

	return (length);
}


/*
 * Print the results to screen with the get command's "raw" mode (-r).
 */
void
print_results(void)
{
	struct result *result;

	for (unsigned int i = 0; i < ARRAY_LENGTH(&results); i++) {
		result = ARRAY_ITEM(&results, i);
		if (result->visible) {
			printf("%ls\n", result->wcs_value);
		}
	}
}
