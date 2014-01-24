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
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <wchar.h>
#include <errno.h>
#include <err.h>

#include "config.h"
#include "debug.h"
#include "utils.h"
#include "str.h"
#include "gpg.h"
#include "xmalloc.h"


static pid_t gpg_pid;


/*
 * Ensures gpg exists, runs and is configured. Also makes sure we have a
 * recipient key configured or passed via the command-line argument.
 */
void
gpg_check(void)
{
	char *cmd;

	/* Doesn't run, doesn't exist. */
	cmd = join(' ', cfg_gpg_path, "--version > /dev/null");
	if (system(cmd) != 0) {
		errx(EXIT_FAILURE, "unable to run gpg (check gpg_path)");
	}
	xfree(cmd);

	/* No key configured at all. */
	cmd = join(' ', cfg_gpg_path, "--list-secret-keys | grep -q sec");
	if (system(cmd) != 0) {
		errx(EXIT_FAILURE, "no gpg key found");
	}

	/* No key defined in the configuration/cmd-line. */
	if (cfg_gpg_key_id == NULL) {
		errx(EXIT_FAILURE, "no gpg_key_id (use -k or ~/.mdp/config).");
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
	int pout[2];	// {read, write}
	FILE *fp;

	if (!file_exists(cfg_password_file)) {
		debug("gpg_open password file does not exist (yet)");
		return NULL;
	}

	debug("gpg_open %s %s", cfg_gpg_path, cfg_password_file);

	if (pipe(pout) != 0)
		err(EXIT_FAILURE, "gpg_decode pipe(pout)");

	gpg_pid = fork();

	switch (gpg_pid) {
	case -1:
		err(EXIT_FAILURE, "gpg_decode fork");
		break;
	case 0:
		/* Child process pipe dance. */
		if (close(pout[0]))
			err(EXIT_FAILURE, "child close(pout[0])");

		if (dup2(pout[1], STDOUT_FILENO) == -1)
			err(EXIT_FAILURE, "dup2 (child stdout)");

		if (pout[1] != STDOUT_FILENO)
			if (close(pout[1]))
				err(EXIT_FAILURE, "child close(pipe_out[1])");

		debug("gpg_open child pid: %d", getpid());

		execlp(cfg_gpg_path, "-q", "--decrypt", cfg_password_file,
				NULL);
		err(EXIT_FAILURE, "couldn't execute");
		/* NOTREACHED */
	default:
		/* Parent process. NOP'ing. */
		break;
	}

	/* We are the parent. Close the child side of the pipe. */
	if (close(pout[1]) != 0) {
		err(EXIT_FAILURE, "close(pout[1])");
	}

	fp = fdopen(pout[0], "r");

	/*
	 * Since we spawned a new process, we keep track of it and shut it down
	 * by force if it takes too long.
	 */
	set_pid_timeout(gpg_pid, cfg_gpg_timeout);

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

	if (fclose(fp) != 0) {
		err(EXIT_FAILURE, "gpg_close fclose()");
	}

	x = waitpid(gpg_pid, &status, 0);

	if (WIFSIGNALED(status)) {
		errx(EXIT_FAILURE, "gpg_close gpg interrupted");
	}

	if (x == -1) {
		err(EXIT_FAILURE, "gpg_close wait()");
	}

	retcode = WEXITSTATUS(status);
	debug("gpg_close return code: %d", retcode);

	cancel_pid_timeout();

	return retcode;
}


/*
 * Saves the file back though GnuPG by saving to a temp file.
 */
void
gpg_encrypt(const char *tmp_path)
{
	char *cmd, *backup_path, *tmp_encrypted_path;

	xasprintf(&cmd, "%s -r %s -e %s", cfg_gpg_path, cfg_gpg_key_id,
			tmp_path);

	debug("gpg_encrypt system(%s)", cmd);
	if (system(cmd) != 0) {
		errx(EXIT_FAILURE, "gpg_encrypt system(%s) != 0", cmd);
	}

	/* Generate the backup filename. */
	xasprintf(&backup_path, "%s.bak", cfg_password_file);

	if (file_exists(cfg_password_file)) {
		/* Backup the previous password file. */
		if (cfg_backup) {
			debug("gpg_encrypt backup: %s", backup_path);

			/* Delete the previous backup. */
			if (unlink(backup_path) != 0) {
				if (errno != ENOENT) {
					err(EXIT_FAILURE, "gpg_encrypt backup "
							"unlink");
				}
			}

			/* Create a physical link. */
			if (link(cfg_password_file, backup_path) != 0) {
				err(EXIT_FAILURE, "gpg_encrypt backup link");
			}
		}

		/* Unlink the previous location, keeping only the backup. */
		if (unlink(cfg_password_file) != 0) {
			err(EXIT_FAILURE, "gpg_encrypt unlink(cfg_password_file)");
		}
	}

	/* Move the newly encrypted file to its new location. */
	xasprintf(&tmp_encrypted_path, "%s.gpg", tmp_path);
	if (link(tmp_encrypted_path, cfg_password_file) != 0) {
		err(EXIT_FAILURE, "gpg_encrypt link(tmp_encrypted_path=%s, "
				"cfg_password_file=%s)", tmp_encrypted_path,
				cfg_password_file);
	} else {
		if (chmod(cfg_password_file, S_IRUSR | S_IWUSR) !=0) {
			err(EXIT_FAILURE, "gpg_encrypt chmod");
		}
	}

	if (unlink(tmp_encrypted_path) != 0) {
		err(EXIT_FAILURE, "gpg_encrypt unlink(tmp_encrypted_path)");
	}
}
