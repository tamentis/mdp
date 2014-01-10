/*
 * Copyright (c) 2004 Nicholas Marriott <nicm@users.sourceforge.net>
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
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/param.h>

#include <stdio.h>
#include <err.h>
#include <errno.h>
#include <libgen.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "xmalloc.h"
#include "strlcpy.h"


char *
xstrdup(const char *s)
{
	char	*ptr;
	size_t	 len;

	len = strlen(s) + 1;
	ptr = xmalloc(len);

	strlcpy(ptr, s, len);
	return (ptr);
}

void *
xcalloc(size_t nmemb, size_t size)
{
	void	*ptr;

	if (size == 0 || nmemb == 0) {
		errx(EXIT_FAILURE, "zero size");
	}
	if (SIZE_MAX / nmemb < size) {
		errx(EXIT_FAILURE, "nmemb * size > SIZE_MAX");
	}
	if ((ptr = calloc(nmemb, size)) == NULL) {
		err(EXIT_FAILURE, "xcalloc failed");
	}

	return (ptr);
}

void *
xmalloc(size_t size)
{
	void	*ptr;

	if (size == 0) {
		errx(EXIT_FAILURE, "zero size");
	}
	if ((ptr = malloc(size)) == NULL) {
		err(EXIT_FAILURE, "xmalloc failed");
	}

	return (ptr);
}

void *
xrealloc(void *oldptr, size_t nmemb, size_t size)
{
	size_t	 newsize = nmemb * size;
	void	*newptr;

	if (newsize == 0) {
		errx(EXIT_FAILURE, "zero size");
	}
	if (SIZE_MAX / nmemb < size) {
		errx(EXIT_FAILURE, "nmemb * size > SIZE_MAX");
	}
	if ((newptr = realloc(oldptr, newsize)) == NULL) {
		err(EXIT_FAILURE, "xrealloc failed");
	}

	return (newptr);
}

void
xfree(void *ptr)
{
	if (ptr == NULL) {
		errx(EXIT_FAILURE, "null pointer");
	}
	free(ptr);
}

int
xasprintf(char **ret, const char *fmt, ...)
{
	va_list ap;
	int	i;

	va_start(ap, fmt);
	i = xvasprintf(ret, fmt, ap);
	va_end(ap);

	return (i);
}

int
xvasprintf(char **ret, const char *fmt, va_list ap)
{
	int	i;

	i = vasprintf(ret, fmt, ap);
	if (i < 0 || *ret == NULL) {
		err(EXIT_FAILURE, "xvasprintf failed");
	}

	return (i);
}

int
xsnprintf(char *buf, size_t len, const char *fmt, ...)
{
	va_list ap;
	int	i;

	va_start(ap, fmt);
	i = xvsnprintf(buf, len, fmt, ap);
	va_end(ap);

	return (i);
}

int
xvsnprintf(char *buf, size_t len, const char *fmt, va_list ap)
{
	int	i;

	if (len > INT_MAX) {
		errx(EXIT_FAILURE, "len > INT_MAX");
	}

	i = vsnprintf(buf, len, fmt, ap);
	if (i < 0) {
		err(EXIT_FAILURE, "vsnprintf failed");
	}

	return (i);
}
