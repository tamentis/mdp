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

#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <wchar.h>
#include <errno.h>
#include <err.h>

#include "utils.h"


extern wchar_t	 cfg_gpg_path[MAXPATHLEN];
extern wchar_t	 cfg_gpg_key_id[MAXPATHLEN];
extern wchar_t	 passwords_path[MAXPATHLEN];


FILE *
gpg_open()
{
	char mbs_gpg_path[MAXPATHLEN];
	char mbs_passwords_path[MAXPATHLEN];
	int pout[2];	// {read, write}
	FILE *fp;
	pid_t pid;

	wcstombs(mbs_gpg_path, cfg_gpg_path, MAXPATHLEN);
	wcstombs(mbs_passwords_path, passwords_path, MAXPATHLEN);

	debug("gpg_open %s %s", mbs_gpg_path, mbs_passwords_path);

	if (pipe(pout) != 0)
		err(1, "gpg_decode pipe(pout)");

	pid = fork();

	switch (pid) {
	case -1:
		err(1, "gpg_decode fork");
		break;
	case 0:
		/* Child process pipe dance. */
		if (close(pout[0]))
			err(1, "child close(pout[0])");

		if (dup2(pout[1], STDOUT_FILENO) == -1)
			err(1, "dup2 (child stdout)");

		if (pout[1] != STDOUT_FILENO)
			if (close(pout[1]))
				err(1, "child close(pipe_out_fd[1])");

		debug("gpg_open child pid: %d", getpid());

		execlp(mbs_gpg_path, "-q", "--decrypt", mbs_passwords_path,
				NULL);
		err(1, "couldn't execute");
		/* NOTREACHED */
	default:
		/* Parent process. Nopping. */
		break;
	}

	/* We are the parent. Close the child side of the pipe. */
	if (close(pout[1]) != 0)
		err(1, "close(pout[1])");

	fp = fdopen(pout[0], "r");

	set_pid_timeout(pid);

	return fp;
}


void
gpg_close(FILE *fp, int *status)
{
	int x;

	debug("gpg_close");

	if (fclose(fp) != 0)
		err(1, "gpg_close fclose()");

	x = wait(status);

	if (WIFSIGNALED(*status))
		errx(1, "gpg_close gpg interrupted");

	if (x == -1)
		err(1, "gpg_close wait()");

	debug("exit status: %d", WEXITSTATUS(*status));

	cancel_pid_timeout();
}


/*
 * Saves the file back though gnupg.
 */
void
gpg_encrypt(char *tmp_path)
{
	char cmd[4096];
	char cmd_key[128] = "";
	char new_tmp_path[MAXPATHLEN];
	char mbs_passwords_path[MAXPATHLEN];
	char mbs_passbak_path[MAXPATHLEN];

	/* Encrypt the temp file and delete it. */
	if (wcslen(cfg_gpg_key_id) == 8)
		snprintf(cmd_key, 128, "-r %ls", cfg_gpg_key_id);

	snprintf(cmd, 4096, "%ls %s -e %s", cfg_gpg_path, cmd_key, tmp_path);

	debug("gpg_encrypt system(%s)", cmd);
	system(cmd);

	/* Backup the previous password file. */
	wcstombs(mbs_passwords_path, passwords_path, MAXPATHLEN);
	snprintf(mbs_passbak_path, MAXPATHLEN, "%s.bak", mbs_passwords_path);
	debug("gpg_encrypt backup: %s", mbs_passbak_path);
	if (unlink(mbs_passbak_path) != 0) {
		if (errno != ENOENT)
			err(1, "gpg_encrypt unlink(passback_path)");
	}
	if (link(mbs_passwords_path, mbs_passbak_path) != 0)
		err(1, "gpg_encrypt link(passwords_path, passback_path)");

	/* Move the newly encrypted file to its new location. */
	snprintf(new_tmp_path, MAXPATHLEN, "%s.gpg", tmp_path);

	if (unlink(mbs_passwords_path) != 0)
		err(1, "gpg_encrypt unlink(passwords_path)");
	if (link(new_tmp_path, mbs_passwords_path) != 0)
		err(1, "gpg_encrypt link(new_tmp_path, password_path)");
	if (unlink(new_tmp_path) != 0)
		err(1, "gpg_encrypt unlink(new_tmp_path)");
}
