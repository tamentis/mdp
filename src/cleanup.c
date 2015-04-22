/*
 * Copyright (c) 2013-2015 Bertrand Janin <b@janin.com>
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

#include <sys/types.h>

#include <dirent.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cleanup.h"
#include "debug.h"
#include "editor.h"
#include "lock.h"
#include "results.h"
#include "ui-curses.h"
#include "utils.h"


void
cleanup_tmp_path(void)
{
	/*
	 * Ignore the return of rm_overwrite, even if it fails, we need to
	 * at least delete the file, additionally rm_overwrite emits a warn()
	 * message with the actual error.
	 */
	rm_overwrite(editor_tmp_path);

	if (editor_tmp_path != NULL && unlink(editor_tmp_path) != 0) {
		err(EXIT_FAILURE, "unable to remove '%s'", editor_tmp_path);
	}
}


/*
 * General cleanup function, called when the program terminates.
 */
void
cleanup(void)
{
	/* Delete all the results stored in memory. */
	clear_results();

	/* Make sure the temporary file is gone. */
	cleanup_tmp_path();

	lock_unset();

	/* Just in case we error'd out somewhere during the pager. */
	shutdown_curses();
}


/*
 * startup_cleanup is run from the read_config() function, it ensures all the
 * temporary files that could have been left behind on a previous crash or
 * reboot are removed on the next run.
 */
void
startup_cleanup(char *config_dir)
{
	DIR *dirp;
	struct dirent *dp;

	dirp = opendir(config_dir);
	if (dirp == NULL) {
		err(EXIT_FAILURE, "opendir() failed on '%s'", config_dir);
	}

	while ((dp = readdir(dirp)) != NULL) {
		if (strncmp(dp->d_name, TMPFILE_PREFIX,
				strlen(TMPFILE_PREFIX)) == 0) {
			warn("deleting previous tmp file: %s", dp->d_name);
			rm_overwrite(dp->d_name);
		}
	}

	closedir(dirp);
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
