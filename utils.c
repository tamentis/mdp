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

#include <stdio.h>
#include <wchar.h>
#include <stdarg.h>
#include <stdlib.h>


#define WHITESPACE	L" \t\r\n"


/*
 * Strip trailing whitespace.
 */
void
strip_trailing_whitespaces(wchar_t *s)
{
	int len;

	for (len = wcslen(s) - 1; len > 0; len--) {
		if (wcschr(WHITESPACE, s[len]) == NULL)
			break;
		s[len] = '\0';
	}
}


/*
 * Emergency exit. Panic, scream, run for your life.
 */
void
fatal(const char *fmt,...)
{
        va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(-1);
}

