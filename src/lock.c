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
 */

#include <sys/param.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <errno.h>
#include <err.h>

#include "utils.h"
#include "lock.h"


extern wchar_t	 lock_path[MAXPATHLEN];


int
lock_exists()
{
	char mbs_lock_path[MAXPATHLEN];

	wcstombs(mbs_lock_path, lock_path, MAXPATHLEN);

	return file_exists(mbs_lock_path);
}


void
lock_set()
{
	FILE *fp;
	char mbs_lock_path[MAXPATHLEN];

	wcstombs(mbs_lock_path, lock_path, MAXPATHLEN);

	if (lock_exists())
		errx(1, "locked (%ls)", lock_path);

	fp = fopen(mbs_lock_path, "w");
	fprintf(fp, "%d", getpid());
	fclose(fp);
}


void
lock_unset()
{
	char mbs_lock_path[MAXPATHLEN];

	wcstombs(mbs_lock_path, lock_path, MAXPATHLEN);

	if (lock_exists())
		unlink(mbs_lock_path);
}
