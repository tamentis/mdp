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
#include "curses.h"
#include "config.h"
#include "query.h"


extern int	 window_width;
extern int	 window_height;
extern WINDOW	*screen;
char		 status_message[STATUS_MESSAGE_LEN] = "";


/*
 * Take a finite amount of results and show them full-screen.
 *
 * If the number of results is greater than the available lines on screen,
 * display a prompt to refine the keywords.
 */
int
query(char **keywords)
{
	init_curses();

	/* Find the smallest left offset to fit everything. */
	/*
	for (i = 0; i < len; i++) {
		offset = (window_width - wcslen(results[i])) / 2;
		if (left_offset > offset)
			left_offset = offset;
	}
	*/


	shutdown_curses();

	return MODE_EXIT;
}
