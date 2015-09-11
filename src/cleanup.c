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

#include <unistd.h>
#include <stdlib.h>
#include <err.h>

#include "cleanup.h"
#include "debug.h"
#include "editor.h"
#include "lock.h"
#include "ui-curses.h"


/*
 * General cleanup function, called when the program terminates.
 */
void
cleanup(void)
{
	lock_unset();

	if (editor_tmp_path != NULL && unlink(editor_tmp_path) != 0) {
		err(EXIT_FAILURE, "unable to remove '%s'", editor_tmp_path);
	}

	/* Just in case we error'd out somewhere during the pager. */
	shutdown_curses();
}


/*
 * Cleanup callback given to atexit().
 */
void
atexit_cleanup(void)
{
	debug("atexit_cleanup (PID: %d)", getpid());
	cleanup();
}


/*
 * Cleanup callback assigned to SIGINT.
 */
void
sig_cleanup(int dummy)
{
	/* Avoid unused parameter warning. */
	(void)(dummy);

	debug("sig_cleanup (PID: %d)", getpid());
	cleanup();
}
