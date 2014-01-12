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
 *
 * Initial implementation is inspired from randpass.c of apg, BSD Licensed
 * (3-clauses), with the following copyright notice:
 *
 *     Copyright (c) 1999, 2000, 2001, 2002, 2003
 *     Adel I. Mirzazhanov. All rights reserved
 */

#ifndef _MDP_RANDPASS_H
#define _MDP_RANDPASS_H

#define MAX_PASSWORD_LENGTH	255

int		 generate_password_from_set(wchar_t *, int, const wchar_t *);

#endif /* _MDP_RANDPASS_H_ */
