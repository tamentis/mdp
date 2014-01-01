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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <unistd.h>

#include "arc4random.h"
#include "randpass.h"


struct zzsym {
	char ch;
	short type;
};

struct zzsym smbl[95] = {
	{'a', S_SL}, {'b', S_SL}, {'c', S_SL}, {'d', S_SL}, {'e', S_SL},
	{'f', S_SL}, {'g', S_SL}, {'h', S_SL}, {'i', S_SL}, {'j', S_SL},
	{'k', S_SL}, {'l', S_SL}, {'m', S_SL}, {'n', S_SL}, {'o', S_SL},
	{'p', S_SL}, {'q', S_SL}, {'r', S_SL}, {'s', S_SL}, {'t', S_SL},
	{'u', S_SL}, {'v', S_SL}, {'w', S_SL}, {'x', S_SL}, {'y', S_SL},
	{'z', S_SL}, {'A', S_CL}, {'B', S_CL}, {'C', S_CL}, {'D', S_CL},
	{'E', S_CL}, {'F', S_CL}, {'G', S_CL}, {'H', S_CL}, {'I', S_CL},
	{'J', S_CL}, {'K', S_CL}, {'L', S_CL}, {'M', S_CL}, {'N', S_CL},
	{'O', S_CL}, {'P', S_CL}, {'Q', S_CL}, {'R', S_CL}, {'S', S_CL},
	{'T', S_CL}, {'U', S_CL}, {'V', S_CL}, {'W', S_CL}, {'X', S_CL},
	{'Y', S_CL}, {'Z', S_CL}, {'1', S_NB}, {'2', S_NB}, {'3', S_NB},
	{'4', S_NB}, {'5', S_NB}, {'6', S_NB}, {'7', S_NB}, {'8', S_NB},
	{'9', S_NB}, {'0', S_NB}, {33 , S_SS}, {34 , S_SS}, {35 , S_SS},
	{36 , S_SS}, {37 , S_SS}, {38 , S_SS}, {39 , S_SS}, {40 , S_SS},
	{41 , S_SS}, {42 , S_SS}, {43 , S_SS}, {44 , S_SS}, {45 , S_SS},
	{46 , S_SS}, {47 , S_SS}, {58 , S_SS}, {59 , S_SS}, {60 , S_SS},
	{61 , S_SS}, {62 , S_SS}, {63 , S_SS}, {64 , S_SS}, {91 , S_SS},
	{92 , S_SS}, {93 , S_SS}, {94 , S_SS}, {95 , S_SS}, {96 , S_SS},
	{123, S_SS}, {124, S_SS}, {125, S_SS}, {126, S_SS}
};


static int
construct_mode(char *s_mode)
{
	unsigned int mode = 0;
	int ch = 0;
	int i = 0;
	int str_length = 0;

	str_length = strlen(s_mode);

	if (str_length > MAX_MODE_LENGTH)
		return -1;

	for (i = 0; i < str_length; i++) {
		ch = (int)*s_mode;
		switch (ch) {
			case 'S':
			case 's':
				mode = mode | S_SS;
				break;
			case 'N':
			case 'n':
				mode = mode | S_NB;
				break;
			case 'C':
			case 'c':
				mode = mode | S_CL;
				break;
			case 'L':
			case 'l':
				mode = mode | S_SL;
				break;
			default:
				return -1;
				break;
		}
		s_mode++;
	}

	return mode;
}


/*
 * Generates random password of specified type.
 */
int
generate_password(char *password_string, int length, char *mode)
{
	int i = 0;
	int j = 0;
	int pass_mode;
	char *str_pointer;
	int random_weight[94];
	int max_weight = 0;
	int max_weight_element_number = 0;

	pass_mode = construct_mode(mode);

	if (length > MAX_PASSWORD_LENGTH || length < 1) {
		return -1;
	}

	for (i = 0; i <= 93; i++) {
		random_weight[i] = 0;
	}

	str_pointer = password_string;

	for (i = 0; i < length; i++) {
		/* Assign random weight in weight array if mode is present */
		for (j = 0; j <= 93 ; j++) {
			if ( ( (pass_mode & smbl[j].type) > 0) &&
					!( (S_RS & smbl[j].type) > 0)) {
				random_weight[j] = 1 + arc4random_uniform(20000);
			}
		}

		/* Find an element with maximum weight */
		for (j = 0; j <= 93; j++) {
			if (random_weight[j] > max_weight) {
				max_weight = random_weight[j];
				max_weight_element_number = j;
			}
		}

		/* Get password symbol */
		*str_pointer = smbl[max_weight_element_number].ch;
		str_pointer++;
		max_weight = 0;
		max_weight_element_number = 0;
		for (j = 0; j <= 93; j++) {
			random_weight[j] = 0;
		}
	}

	*str_pointer = '\0';

	return length;
}

