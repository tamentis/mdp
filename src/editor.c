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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <err.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

#include "array.h"
#include "config.h"
#include "debug.h"
#include "editor.h"
#include "gpg.h"
#include "results.h"
#include "str.h"
#include "utils.h"
#include "xmalloc.h"


char *editor_tmp_path = NULL;


/*
 * Return a copy of the $EDITOR environment variable or NULL if not found.
 *
 * If the chosen editor appears to be vim, we add '-n' to the command line
 * arguments (unless already present), which disable the creation of a swap
 * file.
 */
void
editor_init(const char *home)
{
	char *s;

	s = getenv("EDITOR");
	if (s == NULL) {
		return;
	}

	cfg_editor = strdup(s);

	editor_tmp_path = join_path(home, ".mdp/tmp_edit.XXXXXXXX");
}


/*
 * Check if the given editor command is vim.
 */
bool
editor_is_vim(const char *command)
{
	const char *s;

	/*
	 * The command is either before a space (in case there are other
	 * paramters) or at the end of the string.
	 */
	s = strpbrk(command, " \0\t");
	if (s == NULL) {
		s = command + strlen(command);
	}

	/* There aren't enough characters for 'vim' to fit. */
	if (s - command < 3) {
		return false;
	}

	if (strncmp(s - 3, "vim", 3) == 0) {
		return true;
	}

	return false;
}


/*
 * Spawn the editor on a file.
 */
static void
spawn_editor(char *path)
{
	char *cmd;
	char *editor;

	if (editor_is_vim(cfg_editor)) {
		editor = join(' ', cfg_editor, "-n");
	} else {
		editor = strdup(cfg_editor);
	}

	xasprintf(&cmd, "%s \"%s\"", editor, path);

	debug("spawn_editor: %s", cmd);

	if (system(cmd) != 0) {
		err(EXIT_FAILURE, "unable to spawn editor: %s", cmd);
	}

	xfree(editor);
	xfree(cmd);
}


/*
 * Check if the given path has the right sum and size.
 *
 * This is far from perfect, but for the purpose of detecting change, this is
 * just fine. Returns 1 if it matches.
 */
static int
has_changed(char *path)
{
	FILE *fp;
	uint32_t previous_sum, previous_size;

	/*
	 * Keep track of the previous sum/size so we can check if anything
	 * changed.
	 */
	previous_sum = result_sum;
	previous_size = result_size;

	fp = fopen(path, "r");
	load_results_fp(fp);
	fclose(fp);

	if (previous_sum != result_sum || previous_size != result_size)
		return 1;

	return 0;
}


/*
 * Edit the passwords.
 *
 * This function dumps all the plain-text passwords ("results") in a temporary
 * file in your ~/.mdp/ folder, fires your editor and save the output back to
 * your password file.
 */
void
edit_results(void)
{
	int tmp_fd = -1;
	struct result *result;

	/* Create the temporary file for edit mode. */
	tmp_fd = mkstemp(editor_tmp_path);
	if (tmp_fd == -1) {
		err(EXIT_FAILURE, "edit_results mkstemp()");
	}

	/* Iterate over the results and dump them in this file. */
	for (unsigned int i = 0; i < ARRAY_LENGTH(&results); i++) {
		result = ARRAY_ITEM(&results, i);
		if (write(tmp_fd, result->mbs_value, result->mbs_len) == -1)
			err(EXIT_FAILURE, "edit_results write");
		if (write(tmp_fd, "\n", 1) == -1)
			err(EXIT_FAILURE, "edit_results write (new-line)");
	}

	if (close(tmp_fd) != 0) {
		err(EXIT_FAILURE, "edit_results close(tmp_fd)");
	}

	spawn_editor(editor_tmp_path);

	if (has_changed(editor_tmp_path)) {
		gpg_encrypt(editor_tmp_path);
	} else {
		fprintf(stderr, "No changes, exiting...\n");
	}
}
