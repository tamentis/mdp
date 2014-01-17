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
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>

#include "array.h"
#include "keywords.h"
#include "debug.h"
#include "cmd.h"


char		*cmd_config_path = NULL;
char		*cmd_gpg_key_id = NULL;
char		*cmd_profile_name = NULL;
bool		 cmd_regex = false;
bool		 cmd_raw = false;
unsigned int	 cmd_character_count = 0;
unsigned int	 cmd_password_count = 0;


enum command
cmd_parse(int argc, char **argv)
{
	int opt;
	enum command command = COMMAND_GET;

	if (argc < 2) {
		return COMMAND_USAGE;
	}

	while ((opt = getopt(argc, argv, "eErqgVhdc:k:p:l:n:")) != -1) {
		switch (opt) {
		case 'e':
			command = COMMAND_EDIT;
			break;
		case 'E':
			cmd_regex = true;
			break;
		case 'r':
			cmd_raw = true;
			break;
		case 'q':
			command = COMMAND_QUERY;
			break;
		case 'g':
			command = COMMAND_GENERATE;
			break;
		case 'V':
			command = COMMAND_VERSION;
			break;
		case 'd':
			debug_enabled = true;
			break;
		case 'c':
			cmd_config_path = strdup(optarg);
			break;
		case 'k':
			cmd_gpg_key_id = strdup(optarg);
			break;
		case 'p':
			cmd_profile_name = strdup(optarg);
			break;
		case 'l':
			cmd_character_count = strtoumax(optarg, NULL, 10);
			break;
		case 'n':
			cmd_password_count = strtoumax(optarg, NULL, 10);
			break;
		default:
			command = COMMAND_USAGE;
		}
	}

	argv += optind;

	keywords_load_from_argv(argv);

	return command;
}
