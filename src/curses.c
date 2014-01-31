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

#include <sys/ioctl.h>
#include <sys/param.h>

#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>
#include <err.h>
#include <signal.h>
#include <curses.h>
#include <stdarg.h>

#include "config.h"
#include "curses.h"
#include "str.h"
#include "xmalloc.h"


unsigned int		 window_width = 0, window_height = 0;
WINDOW			*screen = NULL;


/*
 * Shuts down curses.
 */
void
shutdown_curses()
{
	if (screen != NULL) {
		clear();
		endwin();
	}
}


/*
 * Called when the terminal is resized.
 */
void
resize(int signal)
{
	/* Ignore unused parameter, this is only called from SIGWINCH. */
	(void)(signal);

	clear();
	shutdown_curses();
	errx(EXIT_FAILURE, "terminal resize");
}


/*
 * Wrapper around waddstr which automatically converts wide-char strings.
 */
int
waddwcs(WINDOW *win, const wchar_t *str)
{
	int ret;
	char *mbs;

	mbs = wcs_duplicate_as_mbs(str);
	ret = waddstr(win, mbs);
	xfree(mbs);

	return ret;
}


/*
 * Starts curses, obtains term size, set colors.
 */
void
init_curses()
{
	struct winsize ws;

	/* terminal size stuff */
	signal(SIGWINCH, resize);
	if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) != -1) {
		window_width = ws.ws_col;
		window_height = ws.ws_row;
	}

	/* curses screen init */
	screen = initscr();
	noecho();
	// cbreak();
	curs_set(0);
	// nodelay(screen, TRUE);

	timeout(cfg_timeout * 1000);
}
