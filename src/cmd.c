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
#include <err.h>

#include "array.h"
#include "cmd.h"
#include "debug.h"
#include "keywords.h"
#include "str.h"
#include "xmalloc.h"


wchar_t		*cmd_add_prefix = NULL;
char		*cmd_config_path = NULL;
char		*cmd_gpg_key_id = NULL;
char		*cmd_profile_name = NULL;
bool		 cmd_regex = false;
bool		 cmd_raw = false;
unsigned int	 cmd_character_count = 0;
unsigned int	 cmd_password_count = 0;


/*
 * Test if a value starts like the given command. It will return true even if
 * the given value is one character long, as long as this character is the
 * same as the command.
 */
bool
command_match(const char *s, const char *name, size_t min)
{
	size_t len = strlen(s);

	if (strncmp(s, name, len) == 0) {
		if (len < min) {
			errx(EXIT_FAILURE, "ambiguous command '%s'", s);
		}
		return true;
	}

	return false;
}


/*
 * Core usage and parse (everything before the command).
 */

void
cmd_usage_core(void)
{
	printf("usage: mdp [-Vh] [-c config] command [command args ...]\n");
}


void
cmd_usage_core_with_commands(void)
{
	cmd_usage_core();
	printf("\n");
	printf("The mdp commands are:\n");
	// printf("   add        Add a new random password at the end of your file.\n");
	printf("   edit       Edit your passwords.\n");
	printf("   generate   Generate random passwords.\n");
	printf("   get        Get passwords by keywords or regexes.\n");
	printf("   prompt     Interactive prompt session.\n");
	printf("\n");
	printf("'mdp <command> -h' returns this command's usage.\n");
}


int
cmd_parse_core(int argc, char **argv)
{
	int opt;
	int command_index;

	if (argc < 2) {
		cmd_usage_core();
		exit(EXIT_FAILURE);
	}

	while ((opt = getopt(argc, argv, "+hVdc:egrq")) != -1) {
		switch (opt) {
		case 'h':
			cmd_usage_core_with_commands();
			exit(EXIT_FAILURE);
			break;
		case 'V':
			printf("mdp-%s\n", MDP_VERSION);
			exit(EXIT_SUCCESS);
			break;
		case 'd':
			debug_enabled = true;
			break;
		case 'c':
			cmd_config_path = strdup(optarg);
			break;
		case 'e':
			errx(EXIT_FAILURE, "this flag is deprecated, use "
					"'mdp edit' instead");
			break;
		case 'g':
			errx(EXIT_FAILURE, "this flag is deprecated, use "
					"'mdp gen' instead");
			break;
		case 'r':
			errx(EXIT_FAILURE, "this flag is deprecated, use "
					"'mdp get -r' instead");
			break;
		case 'q':
			errx(EXIT_FAILURE, "this flag is deprecated, use "
					"'mdp prompt' instead");
			break;
		default:
			exit(EXIT_FAILURE);
			break;
		}
	}

	/* Reset optind for other parsers to work properly. */
	command_index = optind;
	optind = 1;

	return command_index;
}


/*
 * 'mdp add' - usage and parse
 */

static void
cmd_usage_add(void)
{
	printf("usage: mdp a[dd] [-h] [-p profile] [-n count] [-l length]\n");
	printf("                 [-k key_id] [keywords ...]\n");
}

void
cmd_parse_add(int argc, char **argv)
{
	int opt;

	while ((opt = getopt(argc, argv, "hp:l:n:k:")) != -1) {
		switch (opt) {
		case 'h':
			cmd_usage_add();
			exit(EXIT_FAILURE);
		case 'p':
			cmd_profile_name = strdup(optarg);
			break;
		case 'l':
			cmd_character_count = strtoumax(optarg, NULL, 10);
			break;
		case 'n':
			cmd_password_count = strtoumax(optarg, NULL, 10);
			break;
		case 'k':
			cmd_gpg_key_id = strdup(optarg);
			break;
		default:
			exit(EXIT_FAILURE);
			break;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc > 0) {
		char *s = join_list(' ', argc, argv);
		cmd_add_prefix = mbs_duplicate_as_wcs(s);
		xfree(s);
	}
}


/*
 * mdp edit usage and parse
 */

static void
cmd_usage_edit(void)
{
	printf("usage: mdp edit [-h] [-k key_id]\n");
}


void
cmd_parse_edit(int argc, char **argv)
{
	int opt;

	while ((opt = getopt(argc, argv, "hk:")) != -1) {
		switch (opt) {
		case 'h':
			cmd_usage_edit();
			exit(EXIT_FAILURE);
		case 'k':
			cmd_gpg_key_id = strdup(optarg);
			break;
		default:
			exit(EXIT_FAILURE);
			break;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc > 0) {
		cmd_usage_edit();
		exit(EXIT_FAILURE);
	}
}


/*
 * mdp generate usage and parse
 */

static void
cmd_usage_generate(void)
{
	printf("usage: mdp gen[erate] [-h] [-p profile] [-n count] "
			"[-l length]\n");
}

void
cmd_parse_generate(int argc, char **argv)
{
	int opt;

	while ((opt = getopt(argc, argv, "hp:l:n:")) != -1) {
		switch (opt) {
		case 'h':
			cmd_usage_generate();
			exit(EXIT_FAILURE);
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
			exit(EXIT_FAILURE);
			break;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc > 0) {
		cmd_usage_generate();
		exit(EXIT_FAILURE);
	}
}


/*
 * mdp get usage and parse
 */

static void
cmd_usage_get(void)
{
	printf("usage: mdp get [-hrE] keyword ...\n");
}

void
cmd_parse_get(int argc, char **argv)
{
	int opt;

	while ((opt = getopt(argc, argv, "hrE")) != -1) {
		switch (opt) {
		case 'h':
			cmd_usage_get();
			exit(EXIT_FAILURE);
		case 'r':
			cmd_raw = true;
			break;
		case 'E':
			cmd_regex = true;
			break;
		default:
			exit(EXIT_FAILURE);
			break;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc == 0) {
		cmd_usage_get();
		exit(EXIT_FAILURE);
	}

	keywords_load_from_argv(argv);
}


/*
 * mdp prompt usage and parse
 */

static void
cmd_usage_prompt(void)
{
	printf("usage: mdp prompt [-h]\n");
}

void
cmd_parse_prompt(int argc, char **argv)
{
	int opt;

	while ((opt = getopt(argc, argv, "h")) != -1) {
		switch (opt) {
		case 'h':
			cmd_usage_get();
			exit(EXIT_FAILURE);
		default:
			exit(EXIT_FAILURE);
			break;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc > 0) {
		cmd_usage_prompt();
		exit(EXIT_FAILURE);
	}
}
