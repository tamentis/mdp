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

#include <sys/stat.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>

#include "debug.h"
#include "str.h"
#include "utils.h"
#include "xmalloc.h"


pid_t watcher_pid = 0;


/*
 * Portable dirname wrapper.
 *
 * Since some dirname implementation happen to alter the provided path, we
 * create a copy of the source path. We also create a copy of the result since
 * depending on the implementation that coult be located in internal storage.
 */
char *
xdirname(const char *path)
{
	char *path_copy, *dir, *s;

	path_copy = strdup(path);

	dir = dirname(path_copy);
	if (dir == NULL) {
		err(EXIT_FAILURE, "xdirname");
	}

	s = strdup(dir);

	xfree(path_copy);

	return s;
}


/*
 * Return a copy of the $HOME environment variable. Be generous in flaming the
 * user if their environment is in poor condition.
 */
char *
get_home(void)
{
	char *s;

	s = getenv("HOME");

	if (s == NULL) {
		errx(EXIT_FAILURE, "unknown variable '$HOME'");
	}

	if (!file_exists(s)) {
		errx(EXIT_FAILURE, "your $HOME does not exist");
	}

	return strdup(s);
}


/*
 * Create a new path with the two parts given as parameter.
 */
char *
join_path(const char *base, const char *suffix)
{
	return join('/', base, suffix);
}


/*
 * Check if a file exists.
 */
int
file_exists(const char *filepath)
{
	struct stat sb;

	if (stat(filepath, &sb) != 0) {
		if (errno == ENOENT) {
			return 0;
		}
		err(EXIT_FAILURE, "file_exists stat()");
	}

	return 1;
}


/*
 * Stop the process watch timeout.
 */
void
cancel_pid_timeout()
{
	if (watcher_pid == 0)
		return;

	debug("cancel_pid_timeout on %d", watcher_pid);

	if (kill(watcher_pid, SIGINT) != 0) {
		if (errno != ESRCH) {
			err(EXIT_FAILURE, "cancel_pid_timeout");
		}
	}

	watcher_pid = 0;
}


/*
 * Automatically kill a process after X seconds.
 *
 * Sets the global pid_t for the watcher process, you need to run
 * cancel_pid_timeout once the child process is known to have completed.
 */
void
set_pid_timeout(pid_t pid, int timeout)
{
	watcher_pid = fork();

	switch (watcher_pid) {
	case -1:
		err(EXIT_FAILURE, "set_pid_timeout fork()");
		break;

	case 0: /* Child process */
		/* Reset signals since the parent uses them to cleanup. */
		signal(SIGINT, SIG_DFL);
		signal(SIGKILL, SIG_DFL);

		debug("set_pid_timeout sleep(%d)", timeout);
		sleep(timeout);

		/*
		 * If the parent died in the mean time, don't bother trying to
		 * kill anything, don't even mention anything to the user
		 * unless in debug mode.
		 */
		if (kill(getppid(), 0) != 0) {
			debug("set_pid_timeout parent is gone, aborting");
			_Exit(0);
		}

		debug("set_pid_timeout kill(%d, SIGINT)", pid);
		fprintf(stderr, "gpg timed out after %d seconds, aborting\n",
				timeout);
		if (kill(pid, SIGINT) != 0) {
			err(EXIT_FAILURE, "set_pid_timeout child kill");
		}
		/* Avoid atexit() to run on the child. */
		_Exit(0);

		/* NOTREACHED */

	default:
		/* Parent process. NOP'ing. */
		break;
	}

	debug("set_pid_timeout parent pid=%d, watcher pid=%d", getpid(),
			watcher_pid);
}
