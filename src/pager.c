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
 *
 * The pager is a fullscreen curses app with a built-in timeout which ensures
 * passwords do not linger on screen.
 */

#include <sys/ioctl.h>
#include <sys/param.h>

#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>
#include <err.h>
#include <signal.h>
#include <string.h>
#include <curses.h>
#include <inttypes.h>

#include "array.h"
#include "config.h"
#include "keywords.h"
#include "mdp.h"
#include "pager.h"
#include "results.h"
#include "ui-curses.h"


#define RESULTS_MAX_LEN 128
#define MSG_TOO_MANY "Too many results, please refine your search."


/*
 * Display all the results.
 *
 * This function assumes curses is initialized.
 */
static void
refresh_listing(void)
{
	int top_offset, left_offset;
	unsigned int len = results_visible_length();
	struct result *result;

	if (len >= window_height || len >= RESULTS_MAX_LEN) {
		wmove(screen, window_height / 2,
				(window_width - sizeof(MSG_TOO_MANY) - 1) / 2);
		waddstr(screen, MSG_TOO_MANY);
		refresh();
		return;
	}

	top_offset = (window_height - len) / 2;
	if (window_width < get_max_length()) {
		left_offset = 0;
	} else {
		left_offset = (window_width - get_max_length()) / 2;
	}

	/*
	 * Place the lines on screen. Since curses will automatically wrap
	 * longer lines, we need to force a new-line on lines following them.
	 */
	for (unsigned int i = 0; i < ARRAY_LENGTH(&results); i++) {
		result = ARRAY_ITEM(&results, i);

		if (!result->visible)
			continue;

		wmove(screen, top_offset, left_offset);
		waddstr(screen, result->mbs_value);

		if (result->wcs_len > window_width) {
			top_offset += (result->wcs_len / window_width);
		}

		top_offset++;
	}

	refresh();
}


/*
 * Request search keywords from the user.
 */
static void
keyword_prompt(void)
{
	char kw[50] = "";

	curs_set(1);

	wmove(screen, window_height - 1, 0);
	waddstr(screen, "Keywords: ");

	refresh();
	echo();
	getnstr(kw, 50);

	keywords_load_from_char(kw);
}


/*
 * Take a finite amount of results and show them full-screen.
 *
 * If the number of results is greater than the available lines on screen,
 * display a prompt to refine the keywords.
 */
void
_pager(bool start_with_prompt)
{
	init_curses();

	for (;;) {
		clear();

		if (start_with_prompt) {
			start_with_prompt = false;
			keyword_prompt();
			filter_results();
			continue;
		}

		refresh_listing();

		/* Wait for any keystroke, a slash or a timeout. */
		if (getch() == '/') {
			keyword_prompt();
			filter_results();
			continue;
		}

		break;
	}

	shutdown_curses();
}
