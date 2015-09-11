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
 */

#include <wchar.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>

#include "str.h"
#include "strlcat.h"
#include "xmalloc.h"
#include "wcsncasecmp.h"


#define WCS_WHITESPACE	L" \t\r\n"
#define WHITESPACE	 " \t\r\n"


/*
 * Join an arbitrary number of strings together using a separator.
 */
char *
join_list(char sep, int count, char **tokens)
{
	char *c;
	char *o = NULL;
	size_t len = 0;
	int separator_count;
	bool first = true;
	char csep[] = { sep, '\0' };
	int i;

	/* Allocate at least for each separator and the NUL byte. */
	separator_count = count - 1;
	len += separator_count + 1;

	for (i = 0; i < count; i++) {
		c = tokens[i];
		len += strlen(c);
		if (first) {
			first = false;
			o = xmalloc(len * sizeof(char));
			o[0] = '\0';
		} else {
			len += sizeof(char);
			o = xrealloc(o, len, sizeof(char));
			strlcat(o, csep, len);
		}
		strlcat(o, c, len);
	}

	return (o);
}


/*
 * Join two strings with the given separator.
 */
char *
join(char sep, const char *base, const char *suffix)
{
	char *s;

	xasprintf(&s, "%s%c%s", base, sep, suffix);

	return (s);
}


/*
 * Join two wide-char strings with the given separator.
 */
wchar_t *
wcsjoin(wchar_t sep, const wchar_t *base, const wchar_t *suffix)
{
	wchar_t *s;
	int i;

	i = wcslen(base) + wcslen(suffix) + 1;

	s = calloc(i + 1, sizeof(wchar_t));

	i = swprintf(s, i + 1, L"%ls%lc%ls", base, sep, suffix);
	if (i < 0) {
		err(EXIT_FAILURE, "wsprintf failed");
	}

	return (s);
}


/*
 * Strip trailing whitespace.
 */
void
wcs_strip_trailing_whitespaces(wchar_t *s)
{
	int len;

	for (len = wcslen(s) - 1; len >= 0; len--) {
		if (wcschr(WCS_WHITESPACE, s[len]) == NULL)
			break;
		s[len] = '\0';
	}
}


/*
 * Strip trailing whitespace.
 */
void
strip_trailing_whitespaces(char *s)
{
	int len;

	for (len = strlen(s) - 1; len >= 0; len--) {
		if (strchr(WHITESPACE, s[len]) == NULL)
			break;
		s[len] = '\0';
	}
}


/*
 * Same as wcsstr but case-insensitive.
 */
const wchar_t *
wcscasestr(const wchar_t *s, const wchar_t *find)
{
	wchar_t c, sc;
	size_t len;

	if ((c = *find++) != 0) {
		c = (wchar_t)towlower((wchar_t)c);
		len = wcslen(find);
		do {
			do {
				if ((sc = *s++) == 0)
					return (NULL);
			} while ((wchar_t)towlower((wchar_t)sc) != c);
		} while (wcsncasecmp(s, find, len) != 0);
		s--;
	}

	return (s);
}


/*
 * Duplicate a wide-char string as a multi-byte strings.
 *
 * This is essentially a wrapper around wcstombs with proper memory allocation.
 * You are responsible for free'ing the returned pointer's data. Any
 * encoding/decoding error will cause an immediate exit (e.g. one of the
 * wide-char can't be converted according to the current locale).
 */
char *
wcs_duplicate_as_mbs(const wchar_t *str)
{
	size_t bytelen;
	char *output;

	/*
	 * Find out how much space we need to store the multi-byte string
	 * (excluding the NUL-byte).
	 */
	bytelen = wcstombs(NULL, str, 0);
	if (bytelen == (size_t)-1) {
		return (NULL);
	}

	/*
	 * valgrind will complain about the memory allocated on platforms where
	 * wcslen is optimized to scan by larger chunks (e.g. 8 bytes). You can
	 * safely ignore it since the memory valgrind talks about is never
	 * actually read by the optimized function.
	 */
	output = xmalloc(bytelen + 1);

	bytelen = wcstombs(output, str, bytelen + 1);
	if (bytelen == (size_t)-1) {
		xfree(output);
		return (NULL);
	}

	return (output);
}


/*
 * Decode a multi-byte string as wide-char string.
 *
 * Wrapper around mbstowcs with proper memory allocation.  You are responsible
 * for free'ing the returned pointer's data. Any encoding/decoding error will
 * cause an immediate exit (e.g. one of the wide-char can't be converted
 * according to the current locale).
 */
wchar_t *
mbs_duplicate_as_wcs(const char *str)
{
	size_t bytelen;
	wchar_t *output;

	/*
	 * Find out how much space we need to store the wide-char string
	 * (excluding the NUL-byte).
	 */
	bytelen = mbstowcs(NULL, str, 0);
	if (bytelen == (size_t)-1) {
		return (NULL);
	}

	output = xcalloc(bytelen + 1, sizeof(wchar_t));

	bytelen = mbstowcs(output, str, bytelen + 1);
	if (bytelen == (size_t)-1) {
		xfree(output);
		return (NULL);
	}

	return (output);
}


/*
 * Check if two strings are equal.
 *
 * Just a wrapper around strcmp with a smaller footprint and a boolean return
 * value.
 */
bool
streq(const char *a, const char *b)
{
	if (strcmp(a, b) == 0) {
		return (true);
	}

	return (false);
}
