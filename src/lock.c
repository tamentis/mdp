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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

#include "utils.h"
#include "lock.h"


char *lock_path = NULL;


/*
 * Test if the lock exist.
 */
bool
lock_exists()
{
	return file_exists(lock_path);
}


/*
 * Sets the lock. Exit the program with error status if the lock is already
 * present.
 */
void
lock_set()
{
	FILE *fp;

	if (lock_exists()) {
		errx(EXIT_FAILURE, "locked (%s)", lock_path);
	}

	fp = fopen(lock_path, "w");
	fprintf(fp, "%d", getpid());
	fclose(fp);
}


/*
 * Unset the lock. Exit silently if already gone.
 */
void
lock_unset()
{
	if (lock_exists()) {
		if (unlink(lock_path) != 0) {
			err(EXIT_FAILURE, "lock_unset:unlink()");
		}
	}
}
