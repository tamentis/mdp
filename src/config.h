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

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdbool.h>

extern bool		 cfg_backup;
extern unsigned int	 cfg_character_count;
extern wchar_t		*cfg_character_set;
extern char		*cfg_config_path;
extern char		*cfg_editor;
extern char		*cfg_gpg_path;
extern char		*cfg_gpg_key_id;
extern unsigned int	 cfg_gpg_timeout;
extern unsigned int	 cfg_password_count;
extern char		*cfg_password_file;
extern unsigned int	 cfg_timeout;

void			 config_ensure_directory(const char *);
void			 config_check_file(const char *);
void			 config_check_password_file(const char *);
void			 config_check_variables(void);
void			 config_read(void);
void			 config_set_defaults(const char *);
wchar_t			*config_resolve_character_set(const char *);

#endif /* _CONFIG_H_ */
