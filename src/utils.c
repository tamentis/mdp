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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/fcntl.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>
#include <libgen.h>
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
bool
file_exists(const char *filepath)
{
	struct stat sb;

	if (stat(filepath, &sb) != 0) {
		if (errno == ENOENT) {
			return false;
		}
		err(EXIT_FAILURE, "file_exists stat()");
	}

	return true;
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


/*
 * Make one pass at overwriting the given file descriptor.
 *
 * 	fd: opened, writable file descriptor
 * 	len: how many byte to overwrite (file size)
 * 	buf: in memory buffer used to write to the fd
 * 	bsize: size of the above buffer.
 *
 * Returns true on success, false in case of error.
 */
static bool
pass(int fd, off_t len, char *buf, size_t bsize)
{
	size_t wlen;

	for (; len > 0; len -= wlen) {
		wlen = (size_t)len < bsize ? (size_t)len : bsize;
		arc4random_buf(buf, wlen);
		if (write(fd, buf, wlen) != (ssize_t)wlen) {
			return false;
		}
	}

	return true;
}


/*
 * rm_overwrite --
 *	Overwrite the file with varying bit patterns.
 *
 * XXX
 * This is a cheap way to *really* delete files.  Note that only regular
 * files are deleted, directories (and therefore names) will remain.
 * Also, this assumes a fixed-block file system (like FFS, or a V7 or a
 * System V file system).  In a logging file system, you'll have to have
 * kernel support.
 * Returns true for success.
 */
bool
rm_overwrite(char *file)
{
	struct stat *sbp, sb, sb2;
	size_t bsize;
	int fd = -1;
	char *buf = NULL;

	if (lstat(file, &sb)) {
		goto err;
	}
	sbp = &sb;

	/* You can't invoke this on non regular files. */
	if (!S_ISREG(sbp->st_mode)) {
		return false;
	}

	if (sbp->st_nlink > 1) {
		warnx("%s (inode %llu): not overwritten due to multiple links",
		    file, sbp->st_ino);
		return false;
	}

	if ((fd = open(file, O_WRONLY|O_NONBLOCK|O_NOFOLLOW, 0)) == -1) {
		goto err;
	}

	if (fstat(fd, &sb2)) {
		goto err;
	}

	if (sb2.st_dev != sbp->st_dev || sb2.st_ino != sbp->st_ino ||
	    !S_ISREG(sb2.st_mode)) {
		errno = EPERM;
		goto err;
	}

	/*
	 * This practical structure member allows us to find the most optimal
	 * IO buffer size on BSDs and OS X. Linux however does not have this
	 * member.
	 */
#ifdef HAS_NO_F_IOSIZE
	bsize = 1024U;
#else
	{
		struct statfs fsb;
		if (fstatfs(fd, &fsb) == -1) {
			goto err;
		}
		bsize = MAX(fsb.f_iosize, 1024);
	}
#endif

	if ((buf = malloc(bsize)) == NULL) {
		err(1, "%s: malloc", file);
	}

	if (!pass(fd, sbp->st_size, buf, bsize)) {
		goto err;
	}
	if (fsync(fd)) {
		goto err;
	}
	close(fd);
	free(buf);
	return true;

err:
	warn("%s", file);
	close(fd);
	free(buf);
	return false;
}
