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

#include <wchar.h>
#include <err.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"
#include "xmalloc.h"


#define WCS_WHITESPACE	L" \t\r\n"
#define WHITESPACE	 " \t\r\n"


/*
 * Join two strings with the given separator.
 */
char *
join(const char sep, const char *base, const char *suffix)
{
	char *s;

	xasprintf(&s, "%s%c%s", base, sep, suffix);

	return s;
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
wchar_t *
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

	/*
	 * Ignore the warning for this, it's better to accept const explicitly
	 * and have a warning than removing const everywhere.
	 */
	return ((wchar_t *)s);
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
		err(EXIT_FAILURE, "wcs_duplicate_as_mbs:wcstombs(NULL)");
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
		err(EXIT_FAILURE, "wcs_duplicate_as_mbs:wcstombs(output)");
	}

	return output;
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
		err(EXIT_FAILURE, "mbs_duplicate_as_wcs:mbstowcs(NULL)");
	}

	output = xcalloc(bytelen + 1, sizeof(wchar_t));

	bytelen = mbstowcs(output, str, bytelen + 1);
	if (bytelen == (size_t)-1) {
		err(EXIT_FAILURE, "mbs_duplicate_as_wcs:mbstowcs(output)");
	}

	return output;
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
		return true;
	}

	return false;
}
