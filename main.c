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

#include <sys/termios.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <wchar.h>
#include <limits.h>
#include <errno.h>
#include <err.h>
#include <locale.h>
#include <inttypes.h>
#include <curses.h>


#define WHITESPACE	L" \t\r\n"
#define QUOTE		L"\""
#define RESULTS_MAX_LEN	64


wchar_t  cfg_gpg_path[MAXPATHLEN] = L"/usr/bin/gpg";
wchar_t  cfg_gpg_key_id[MAXPATHLEN] = L"";
wchar_t  cfg_editor[MAXPATHLEN] = L"";
int	 cfg_timeout = 10;

wchar_t	 home[MAXPATHLEN];
wchar_t	 passwords_path[MAXPATHLEN];
wchar_t	 editor[MAXPATHLEN];
int	 window_width = 0;
int	 window_height = 0;
WINDOW	*screen;


/* Utility functions from OpenBSD/SSH in separate files (ISC license) */
size_t		 wcslcpy(wchar_t *, const wchar_t *, size_t);
wchar_t		*strdelim(wchar_t **);
size_t		 strlcpy(char *, const char *, size_t);

void		 shutdown_curses();


enum action_mode {
	MODE_PAGER,
	MODE_RAW,
	MODE_EDIT,
	MODE_CREATE
};

/*
 * Emergency exit. Panic, scream, run for your life.
 */
void
fatal(const char *fmt,...)
{
        va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(-1);
}


/*
 * Sets the value of the given variable, also do some type check
 * just in case.
 */
void
set_variable(wchar_t *name, wchar_t *value, int linenum)
{
	/* set gpg_path <string> */
	if (wcscmp(name, L"gpg_path") == 0) {
		if (value == NULL || *value == '\0') {
			*cfg_gpg_path = '\0';
			return;
		}
		wcslcpy(cfg_gpg_path, value, MAXPATHLEN);

	/* set gpg_key_id <string> */
	} else if (wcscmp(name, L"gpg_key_id") == 0) {
		if (value == NULL || *value == '\0') {
			*cfg_gpg_key_id = '\0';
			return;
		}
		wcslcpy(cfg_gpg_key_id, value, MAXPATHLEN);

	/* set editor <string> */
	} else if (wcscmp(name, L"editor") == 0) {
		if (value == NULL || *value == '\0') {
			*cfg_editor = '\0';
			return;
		}
		wcslcpy(cfg_editor, value, MAXPATHLEN);

	/* set timeout <integer> */
	} else if (wcscmp(name, L"timeout") == 0) {
		if (value == NULL || *value == '\0')
			fatal("config:%d: invalid value for timeout\n");

		cfg_timeout = wcstoumax(value, NULL, 10);

	/* ??? */
	} else {
		fatal("config: unknown variable for set on line %d.\n",
				linenum);
	}
}


/*
 * Strip trailing whitespace.
 */
void
strip_trailing_whitespaces(wchar_t *s)
{
	int len;

	for (len = wcslen(s) - 1; len > 0; len--) {
		if (wcschr(WHITESPACE, s[len]) == NULL)
			break;
		s[len] = '\0';
	}
}


/*
 * Parse a single line of the configuration file.
 *
 * Returns 0 on success or anything else if an error occurred, it will be rare
 * since most fatal errors will quit the program with an error message anyways.
 */
int
process_config_line(wchar_t *config_path, wchar_t *line, int linenum)
{
	wchar_t *keyword, *name, *value;

	strip_trailing_whitespaces(line);

	/* Get the keyword (each line is supposed to begin with a keyword). */
	if ((keyword = strdelim(&line)) == NULL)
		return 0;

	/* Ignore leading whitespace. */
	if (*keyword == '\0')
		keyword = strdelim(&line);

	if (keyword == NULL || !*keyword || *keyword == '\n' || *keyword == '#')
		return 0;

	/* set varname value */
	if (wcscmp(keyword, L"set") == 0) {
		if ((name = strdelim(&line)) == NULL) {
			fatal("%ls: set without variable name on line %d.\n",
					config_path, linenum);
			return -1;
		}
		value = strdelim(&line);
		set_variable(name, value, linenum);

	/* Unknown operation... Code help us. */
	} else {
		fatal("%ls: unknown command on line %d.\n", config_path,
				linenum);
		return -1;
	}

	return 0;
}

/*
 * Creates and/or check the configuration directory.
 *
 * Exits program with error message if anything is wrong.
 */
void
check_config_directory(wchar_t *path)
{
	struct stat sb;
	char mbs_path[MAXPATHLEN];

	wcstombs(mbs_path, path, MAXPATHLEN);

	if (stat(mbs_path, &sb) != 0) {
		if (errno == ENOENT) {
			if (mkdir(mbs_path, 0700) != 0) {
				errx(1, "can't create %ls: %s", path,
						strerror(errno));
			}
			if (stat(mbs_path, &sb) != 0) {
				errx(1, "can't stat newly created %ls: %s",
						path, strerror(errno));
			}
		} else {
			errx(1, "can't access %ls: %s", path, strerror(errno));
		}
	}

	if (!S_ISDIR(sb.st_mode))
		errx(1, "%ls is not a directory", path);

	if (sb.st_uid != 0 && sb.st_uid != getuid())
		errx(1, "bad owner on %ls", path);

	if ((sb.st_mode & 022) != 0)
		errx(1, "bad permissions on %ls", path);
}


/*
 * Check the configuration file.
 *
 * Exits program with error message if anything is wrong.
 */
void
check_config_file(wchar_t *path)
{
	struct stat sb;
	char mbs_path[MAXPATHLEN];

	wcstombs(mbs_path, path, MAXPATHLEN);

	if (stat(mbs_path, &sb) != 0) {
		/* User hasn't created a config file, that's perfectly fine. */
		if (errno == ENOENT) {
			return;
		} else {
			errx(1, "can't access %ls: %s", path, strerror(errno));
		}
	}

	if (!S_ISREG(sb.st_mode))
		errx(1, "%ls is not a regular file", path);

	if (sb.st_uid != 0 && sb.st_uid != getuid())
		errx(1, "bad owner on %ls", path);

	if ((sb.st_mode & 022) != 0)
		errx(1, "bad permissions on %ls", path);
}


/*
 * Prepare and check the configuration paths.
 *
 * Create the ~/.mdp/ directory if it doesn't exist yet, then make sure it has
 * the right permissions, including all the relevant files within.
 */
void
check_config()
{
	FILE *fp;
	char line[128];
	wchar_t wline[128];
	int linenum = 1;
	wchar_t path[MAXPATHLEN];
	char mbs_path[MAXPATHLEN];

	swprintf(passwords_path, MAXPATHLEN, L"%ls/.mdp/passwords", home);
	check_config_file(passwords_path);

	swprintf(path, MAXPATHLEN, L"%ls/.mdp", home);
	check_config_directory(path);

	swprintf(path, MAXPATHLEN, L"%ls/.mdp/config", home);
	check_config_file(path);

	wcstombs(mbs_path, path, MAXPATHLEN);
	fp = fopen(mbs_path, "r");
	if (fp == NULL)
		return;

	while (fgets(line, sizeof(line), fp)) {
		mbstowcs(wline, line, 128);
		process_config_line(path, wline, linenum++);
	}

	fclose(fp);
}

/*
 * Open the file and feed each line one by one to process_config_line.
 */
void
read_config()
{
	FILE *fp;
	char line[128];
	wchar_t wline[128];
	int linenum = 1;
	char path[MAXPATHLEN];
	wchar_t wcs_path[MAXPATHLEN];

	snprintf(path, MAXPATHLEN, "%ls/.mdp/config", home);
	mbstowcs(wcs_path, path, MAXPATHLEN);

	fp = fopen(path, "r");
	if (fp == NULL)
		return;

	while (fgets(line, sizeof(line), fp)) {
		mbstowcs(wline, line, 128);
		process_config_line(wcs_path, wline, linenum++);
	}

	fclose(fp);
}

/* resize - called when the terminal is resized ... */
void
resize(int signal)
{
	clear();
	shutdown_curses();
	errx(1, "terminal resize, exiting...");
}


/*
 * Starts curses, obtains term size, set colors.
 */
void
init_curses()
{
	struct winsize ws;

	/* terminal size stuff */
	signal(SIGWINCH, resize);
	if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) != -1) {
		window_width = ws.ws_col;
		window_height = ws.ws_row;
	}

	/* curses screen init */
	screen = initscr();
	noecho();
	// cbreak();
	curs_set(0);
	// nodelay(screen, TRUE);
}


/*
 * Shuts down curses.
 */
void
shutdown_curses()
{
	endwin();
}


/*
 * Spawn the editor on a file.
 *
 * Not quite implemented...
 */
void
spawn_editor(char *path)
{
	char s[MAXPATHLEN];

	if (wcslen(cfg_editor) == 0) {
		if (wcslen(editor) == 0) {
			wcslcpy(cfg_editor, L"/usr/bin/vi", MAXPATHLEN);
		} else {
			wcslcpy(cfg_editor, editor, MAXPATHLEN);
		}
	}

	snprintf(s, MAXPATHLEN, "%ls \"%s\"", cfg_editor, path);
	system(s);
}


/*
 * Check if the line matches all the keywords.
 */
int
line_matches(wchar_t *line, char **keywords)
{
	int matches = 1;
	char **raw_keyword = keywords;
	wchar_t keyword[128];

	while (*raw_keyword != NULL) {
		mbstowcs(keyword, *raw_keyword, 128);
		if (wcsstr((const wchar_t *)line,
					(const wchar_t *)keyword) == NULL) {
			matches = 0;
			break;
		}
		raw_keyword++;
	}

	return matches;
}


void
show_results_in_pager(int len, wchar_t **results)
{
	int top_offset, left_offset, i, offset;
	char line[256];

	init_curses();

	top_offset = (window_height - len) / 2;
	left_offset = window_width;

	if (len >= window_height || len >= RESULTS_MAX_LEN) {
		shutdown_curses();
		errx(1, "too many results, please refine your search");
	}

	/* Find the smallest left offset to fit everything. */
	for (i = 0; i < len; i++) {
		offset = (window_width - wcslen(results[i])) / 2;
		if (left_offset > offset)
			left_offset = offset;
	}

	/* Place the lines on screen. */
	for (i = 0; i < len; i++) {
		wcstombs(line, results[i], MAXPATHLEN);
		wmove(screen, top_offset, left_offset);
		wprintw(screen, line);
		top_offset++;
	}

	refresh();

	/* Wait for any keystroke or timeout. */
	timeout(cfg_timeout * 1000);
	getch();

	shutdown_curses();
}


FILE *
gpg_open()
{
	char mbs_gpg_path[MAXPATHLEN];
	char mbs_passwords_path[MAXPATHLEN];
	int pout[2];	// {read, write}
	FILE *fp;

	wcstombs(mbs_gpg_path, cfg_gpg_path, MAXPATHLEN);
	wcstombs(mbs_passwords_path, passwords_path, MAXPATHLEN);

	if (pipe(pout) != 0)
		err(1, "gpg_decode pipe(pout)");

	switch (fork()) {
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

	return fp;
}


void
gpg_close(FILE *fp, int *status)
{
	if (fclose(fp) != 0)
		err(1, "gpg_close fclose()");

	if (wait(status) == -1)
		err(1, "gpg_close wait()");
}


/*
 * Check if the given path has the right sum and size.
 *
 * This is far from perfect, but for the purpose of detecting change, this is
 * just fine. Returns 1 if it matches.
 */
int
has_changed(char *tmp_path, uint32_t sum, uint32_t size)
{
	uint32_t new_size = 0, new_sum = 0;
	FILE *fp = fopen(tmp_path, "r");
	char line[256];
	int i;

	while (fgets(line, sizeof(line), fp)) {
		new_size += strlen(line);

		for (i = 0; i < strlen(line); i++) {
			new_sum += line[i];
		}
	}

	if (sum != new_sum || size != new_size)
		return 1;

	return 0;
}


/*
 * Saves the file back though gnupg.
 */
void
gpg_encrypt(char *tmp_path)
{
	char cmd[4096];
	char new_tmp_path[MAXPATHLEN];
	char mbs_passwords_path[MAXPATHLEN];
	char mbs_passbak_path[MAXPATHLEN];

	/* Encrypt the temp file and delete it. */
	snprintf(cmd, 4096, "%ls -r %ls -e %s", cfg_gpg_path, cfg_gpg_key_id,
			tmp_path);
	system(cmd);
	unlink(tmp_path);

	/* Backup the previous password file. */
	snprintf(mbs_passbak_path, MAXPATHLEN, "%s.bak", mbs_passwords_path);
	link(mbs_passwords_path, mbs_passbak_path);

	/* Move the newly encrypted file to its new location. */
	snprintf(new_tmp_path, MAXPATHLEN, "%s.gpg", tmp_path);
	wcstombs(mbs_passwords_path, passwords_path, MAXPATHLEN);
	unlink(mbs_passwords_path);
	link(new_tmp_path, mbs_passwords_path);
	unlink(new_tmp_path);
}


void
get_results(char **keywords, int mode)
{
	int status, idx = 0, i, tmp_fd = -1;
	uint32_t sum = 0, size = 0;
	wchar_t wline[256], *results[RESULTS_MAX_LEN];
	char tmp_path[MAXPATHLEN], line[256];
	FILE *fp;

	fp = gpg_open();

	if (mode == MODE_EDIT) {
		snprintf(tmp_path, MAXPATHLEN, "%ls/.mdp/tmp_edit.XXXXXXXX",
				home);
		tmp_fd = mkstemp(tmp_path);
		if (tmp_fd == -1)
			err(1, "get_results mkstemp()");
	}

	while (fgets(line, sizeof(line), fp)) {
		size += strlen(line);

		for (i = 0; i < strlen(line); i++) {
			sum += line[i];
		}

		if (mode == MODE_EDIT) {
			write(tmp_fd, line, strlen(line));
			continue;
		}

		mbstowcs(wline, line, 128);
		strip_trailing_whitespaces(wline);
		if (line_matches(wline, keywords)) {
			switch (mode) {
				case MODE_PAGER:
					if (idx < RESULTS_MAX_LEN - 1)
						results[idx] = wcsdup(wline);
					idx++;
					break;

				case MODE_RAW:
					printf("%ls\n", wline);
					break;

				default:
					break;
			}
		}
	}

	gpg_close(fp, &status);

	switch (mode) {
		case MODE_PAGER:
			show_results_in_pager(idx, results);
			break;

		case MODE_EDIT:
			if (close(tmp_fd) != 0)
				err(1, "get_results close(tmp_fd)");

			spawn_editor(tmp_path);

			if (has_changed(tmp_path, sum, size)) {
				gpg_encrypt(tmp_path);
			} else {
				fprintf(stderr, "No changes, exiting...\n");
			}
			break;

		default:
			break;
	}
}


void
usage()
{
	printf("usage: mdp [-ecr] [keyword ...]\n");
	exit(-1);
}


int
main(int ac, char **av)
{
	char *t;
	int opt, mode = MODE_PAGER;
	extern int optind, optreset;

	if (ac < 2)
		usage();

	setlocale(LC_ALL, "");

	/* Populate $HOME */
	t = getenv("HOME");
	if (t == NULL || *t == '\0')
		errx(0, "Unknown variable '$HOME'.");
	mbstowcs(home, t, MAXPATHLEN);

	/* Populate $EDITOR */
	t = getenv("EDITOR");
	mbstowcs(editor, t, MAXPATHLEN);

	check_config();

	read_config();

	while ((opt = getopt(ac, av, "ecr")) != -1) {
		switch (opt) {
		case 'e':
			mode = MODE_EDIT;
			break;
		case 'c':
			mode = MODE_CREATE;
			break;
		case 'r':
			mode = MODE_RAW;
			break;
		default:
			usage();
		}
	}

	ac -= optind;
	av += optind;

	/* Decide if we use the internal pager or just dump to screen. */
	switch (mode) {
		case MODE_RAW:
			if (ac == 0)
				usage();

			get_results(av, mode);
			break;

		case MODE_PAGER:
			if (ac == 0)
				usage();

			get_results(av, mode);
			break;

		case MODE_EDIT:
			if (ac != 0)
				usage();

			get_results(av, mode);
			break;

		default:
			errx(1, "unknown mode");
			break;
	}

	return 0;
}
