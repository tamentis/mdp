/*
 * Copyright (c) 2012 Bertrand Janin <b@janin.com>
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

#ifndef MDP_RANDPASS_H
#define MDP_RANDPASS_H  1

#define MAX_PASSWORD_LENGTH	255
#define MAX_MODE_LENGTH		4

#define S_NB	0x01 /* Numeric */
#define S_SS	0x02 /* Special */
#define S_CL	0x04 /* Capital */
#define S_SL	0x08 /* Small   */
#define S_RS    0x10 /* Restricted Symbol*/


int		 generate_password(char *, int, char *);

#endif
