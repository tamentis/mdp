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

#ifndef _RESULTS_H_
#define _RESULTS_H_

#define KEYWORD_LENGTH 50

struct result {
	bool visible;
	wchar_t *wcs_value;
	char *mbs_value;
	size_t wcs_len;
	size_t mbs_len;
};

extern struct wlist results;
extern uint32_t result_sum;
extern uint32_t result_size;

ARRAY_DECL(wlist, struct result *);

struct result	*result_new(const wchar_t *);
void		 result_kill(struct result *);
unsigned int	 results_visible_length(void);
unsigned int	 get_max_length(void);
void		 filter_results(void);
int		 load_results_gpg(void);
int		 load_results_fp(FILE *);
void		 print_results(void);

#endif /* _RESULTS_H_ */
