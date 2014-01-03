/*
 * Copyright (c) 2013 Bertrand Janin <b@janin.com>
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

#ifndef _CMD_H_
#define _CMD_H_

enum action_mode {
	MODE_EXIT,
	MODE_VERSION,
	MODE_USAGE,
	MODE_PAGER,
	MODE_RAW,
	MODE_EDIT,
	MODE_GENERATE,
	MODE_QUERY
};

extern char		*cmd_config_path;
extern char		*cmd_gpg_key_id;
extern bool		 cmd_regex;
extern unsigned int	 cmd_password_length;

enum action_mode	 cmd_parse(int, char **);

#endif /* _CMD_H_ */
