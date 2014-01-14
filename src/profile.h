/*
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
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _PROFILE_H_
#define _PROFILE_H_

#define CHARSET_LOWERCASE	L"abcdefghijklmnopqrstuvwxyz"
#define CHARSET_UPPERCASE	L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define CHARSET_ALPHA		CHARSET_LOWERCASE CHARSET_UPPERCASE
#define CHARSET_DIGITS		L"1234567890"
#define CHARSET_ALPHANUMERIC	CHARSET_ALPHA CHARSET_DIGITS
#define CHARSET_SYMBOLS		L"~`!@#$%^&*()_+-=[]\\{}|;':\",./<>?"
#define CHARSET_PRINTABLE	CHARSET_ALPHANUMERIC CHARSET_SYMBOLS

#define DEFAULT_CHARACTER_COUNT 16
#define DEFAULT_PASSWORD_COUNT 4

struct profile {
	char *name;
	unsigned int password_count;
	unsigned int character_count;
	wchar_t *character_set;
};

ARRAY_DECL(profile_list, struct profile *);

extern struct profile_list profiles;

struct profile	*profile_new(const char *);
struct profile	*profile_get_from_name(const char *);
void		 profile_fprint_passwords(FILE *, struct profile *);
wchar_t		*profile_generate_password(struct profile *);
void		 profile_register(struct profile *);

#endif /* _PROFILE_H_ */
