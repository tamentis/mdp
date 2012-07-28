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

#include <sys/termios.h>
#include <sys/ioctl.h>
#include <sys/param.h>

#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>
#include <err.h>
#include <signal.h>
#include <string.h>
#include <curses.h>

#include "array.h"
#include "mdp.h"
#include "query.h"
#include "curses.h"
#include "config.h"
#include "results.h"
#include "keywords.h"


extern int	 window_width;
extern int	 window_height;
extern WINDOW	*screen;
extern char	 status_message[STATUS_MESSAGE_LEN];
extern struct wlist results;
extern struct wlist keywords;


/*
 * Display all the results.
 *
 * This function assumes curses is initialized.
 */
void
refresh_listing()
{
	int top_offset, left_offset, i;
	int len = results_visible_length();
	struct result *result;
	char line[256];

	top_offset = (window_height - len) / 2;
	left_offset = window_width;

	if (len >= window_height || len >= RESULTS_MAX_LEN) {
		wmove(screen, window_height / 2,
				(window_width - strlen(status_message))/ 2);
		wprintw(screen, "Too many results, please refine "
				"your search.");
		refresh();
		return;
	}

	left_offset = (window_width - get_widest_result()) / 2;

	/* Place the lines on screen. */
	for (i = 0; i < ARRAY_LENGTH(&results); i++) {
		result = ARRAY_ITEM(&results, i);

		if (result->status != RESULT_SHOW)
			continue;

		wcstombs(line, result->value, MAXPATHLEN);
		wmove(screen, top_offset, left_offset);
		wprintw(screen, line);
		top_offset++;
	}

	refresh();
}


void
keyword_prompt(void)
{
	char kw[50] = "";

	curs_set(1);

	wmove(screen, window_height - 1, 0);
	wprintw(screen, "Keywords: ");

	refresh();
	echo();
	getnstr(kw, 50);

	load_keywords_from_char(kw);
}


/*
 * Take a finite amount of results and show them full-screen.
 *
 * If the number of results is greater than the available lines on screen,
 * display a prompt to refine the keywords.
 */
int
pager()
{
	init_curses();

	while (1) {
		clear();
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

	return MODE_EXIT;
}

