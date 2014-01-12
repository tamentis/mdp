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

#ifndef _UTILS_H_
#define _UTILS_H_

char		*join(const char, const char *, const char *);
char		*join_path(const char *, const char *);
void		 wcs_strip_trailing_whitespaces(wchar_t *);
void		 strip_trailing_whitespaces(char *);
wchar_t		*wcscasestr(const wchar_t *, const wchar_t *);
char		*wcs_duplicate_as_mbs(const wchar_t *);
wchar_t		*mbs_duplicate_as_wcs(const char *);
void		 cancel_pid_timeout(void);
void		 set_pid_timeout(pid_t, int);
int		 file_exists(const char *);

#endif /* _UTILS_H_ */
