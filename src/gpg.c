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
 *
 *
 * This file contains all the tools to read and write gpg files.
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
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
#include "gpg.h"


extern wchar_t	 cfg_gpg_path[MAXPATHLEN];
extern wchar_t	 cfg_gpg_key_id[MAXPATHLEN];
extern int	 cfg_gpg_timeout;
extern int	 cfg_backup;
extern wchar_t	 passwords_path[MAXPATHLEN];


#define GPG_KEY_MAX_LENGTH 128
#define GPG_CMD_LENGTH (MAXPATHLEN * 2 + GPG_KEY_MAX_LENGTH + 16)


/*
 * Ensures gpg exists, runs and is configured.
 */
void
gpg_check(void)
{
	struct stat sb;
	char cmd[MAXPATHLEN];
	char mbs_path[MAXPATHLEN];

	wcstombs(mbs_path, cfg_gpg_path, MAXPATHLEN);

	if (stat(mbs_path, &sb) != 0)
		err(1, "gpg_check: wrong gpg path %s",
				mbs_path);

	snprintf(cmd, sizeof(cmd), "%ls --version > /dev/null", cfg_gpg_path);
	if (system(cmd) != 0) {
		errx(1, "gpg_check: unable to run gpg.");
	}

	snprintf(cmd, sizeof(cmd), "%ls --list-secret-keys | grep -q sec",
			cfg_gpg_path);
	if (system(cmd) != 0) {
		errx(1, "gpg_check: no gpg key found, rtfm.");
	}
}


/*
 * Opens the gpg process and stream from its output.
 *
 * Returns NULL if the password file was not found.
 */
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

	if (!file_exists(mbs_passwords_path)) {
		debug("gpg_open password file does not exist (yet)");
		return NULL;
	}

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
		/* Parent process. NOP'ing. */
		break;
	}

	/* We are the parent. Close the child side of the pipe. */
	if (close(pout[1]) != 0)
		err(1, "close(pout[1])");

	fp = fdopen(pout[0], "r");

	/*
	 * Since we spawned a new process, we keep track of it and shut it down
	 * by force if it takes too long.
	 */
	set_pid_timeout(pid, cfg_gpg_timeout);

	return fp;
}


/*
 * Close the gpg output stream and the process.
 *
 * Returns the exit code from GnuPG.
 */
int
gpg_close(FILE *fp)
{
	int x, status;
	int retcode;

	debug("gpg_close");

	if (fclose(fp) != 0)
		err(1, "gpg_close fclose()");

	x = wait(&status);

	if (WIFSIGNALED(status))
		errx(1, "gpg_close gpg interrupted");

	if (x == -1)
		err(1, "gpg_close wait()");

	retcode = WEXITSTATUS(status);
	debug("gpg_close return code: %d", retcode);

	cancel_pid_timeout();

	return retcode;
}


/*
 * Saves the file back though gnupg.
 */
void
gpg_encrypt(char *tmp_path)
{
	char cmd[GPG_CMD_LENGTH];
	char cmd_key[GPG_KEY_MAX_LENGTH] = "";
	char new_tmp_path[MAXPATHLEN];
	char mbs_passwords_path[MAXPATHLEN];
	char mbs_passbak_path[MAXPATHLEN];

	/* Encrypt the temp file and delete it. */
	if (wcslen(cfg_gpg_key_id) == 8)
		snprintf(cmd_key, sizeof(cmd_key), "-r %ls", cfg_gpg_key_id);

	snprintf(cmd, sizeof(cmd), "%ls %s -e %s", cfg_gpg_path, cmd_key,
			tmp_path);

	debug("gpg_encrypt system(%s)", cmd);
	if (system(cmd) != 0)
		errx(1, "gpg_encrypt system(%s) != 0", cmd);

	/* Generate the backup filename. */
	wcstombs(mbs_passwords_path, passwords_path, MAXPATHLEN);
	snprintf(mbs_passbak_path, MAXPATHLEN, "%s.bak",
			mbs_passwords_path);

	if (file_exists(mbs_passwords_path)) {
		/* Backup the previous password file. */
		if (cfg_backup) {
			debug("gpg_encrypt backup: %s", mbs_passbak_path);

			/* Delete the previous backup. */
			if (unlink(mbs_passbak_path) != 0) {
				if (errno != ENOENT)
					err(1, "gpg_encrypt backup unlink");
			}

			/* Create a physical link. */
			if (link(mbs_passwords_path, mbs_passbak_path) != 0)
				err(1, "gpg_encrypt backup link");
		}

		/* Unlink the previous location, keeping only the backup. */
		if (unlink(mbs_passwords_path) != 0)
			err(1, "gpg_encrypt unlink(passwords_path)");
	}

	/* Move the newly encrypted file to its new location. */
	snprintf(new_tmp_path, MAXPATHLEN, "%s.gpg", tmp_path);

	if (link(new_tmp_path, mbs_passwords_path) != 0)
		err(1, "gpg_encrypt link(new_tmp_path, password_path)");
	else {
		if (chmod(mbs_passwords_path, S_IRUSR | S_IWUSR) !=0)
			err(1, "gpg_encrypt chmod");
	}

	if (unlink(new_tmp_path) != 0)
		err(1, "gpg_encrypt unlink(new_tmp_path)");
}
