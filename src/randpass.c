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
 * Initial implementation is inspired from randpass.c of apg, BSD Licensed
 * (3-clauses), with the following copyright notice:
 *
 *     Copyright (c) 1999, 2000, 2001, 2002, 2003
 *     Adel I. Mirzazhanov. All rights reserved
 */

#include <sys/types.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <unistd.h>

#include "arc4random.h"
#include "randpass.h"
#include "xmalloc.h"


/*
 * Generate a password of the given length using the provided set of
 * characters.
 */
int
generate_password_from_set(char *password_string, int length, char *set)
{
	char *str_pointer;
	int *random_weight;
	int max_weight = 0;
	int max_weight_element_number = 0;
	size_t setlen;

	setlen = strlen(set);

	if (length > MAX_PASSWORD_LENGTH || length < 1) {
		return -1;
	}

	random_weight = xcalloc(setlen, sizeof(int));

	for (size_t i = 0; i < setlen; i++) {
		random_weight[i] = 0;
	}

	str_pointer = password_string;

	for (int i = 0; i < length; i++) {
		/* Assign random weight in weight array if mode is present */
		for (size_t j = 0; j < setlen ; j++) {
			random_weight[j] = 1 + arc4random_uniform(20000);
		}

		/* Find an element with maximum weight */
		for (size_t j = 0; j < setlen; j++) {
			if (random_weight[j] > max_weight) {
				max_weight = random_weight[j];
				max_weight_element_number = j;
			}
		}

		/* Get password symbol */
		*str_pointer = set[max_weight_element_number];
		str_pointer++;
		max_weight = 0;
		max_weight_element_number = 0;
		for (size_t j = 0; j < setlen; j++) {
			random_weight[j] = 0;
		}
	}

	*str_pointer = '\0';

	return 0;
}
