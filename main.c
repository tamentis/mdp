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
#define MAX_RESULTS	10


wchar_t  cfg_gpg_path[MAXPATHLEN] = L"/usr/bin/gpg";
wchar_t  cfg_gpg_key_id[MAXPATHLEN] = L"";
wchar_t  cfg_editor[MAXPATHLEN] = L"/usr/bin/vi";
int	 cfg_timeout = 10;

wchar_t	 home[MAXPATHLEN];
wchar_t	 editor[MAXPATHLEN];
int	 window_width = 0;
int	 window_height = 0;


/* Utility functions from OpenBSD/SSH in separate files (ISC license) */
size_t		 wcslcpy(wchar_t *, const wchar_t *, size_t);
wchar_t		*strdelim(wchar_t **);
size_t		 strlcpy(char *, const char *, size_t);


enum action_mode {
	MODE_SHOW,
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
process_config_line(char *config_path, wchar_t *line, int linenum)
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
			fatal("%s: set without variable name on line %d.\n",
					config_path, linenum);
			return -1;
		}
		value = strdelim(&line);
		set_variable(name, value, linenum);

	/* Unknown operation... Code help us. */
	} else {
		fatal("%s: unknown command on line %d.\n", config_path,
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
check_config_directory(char *path)
{
	struct stat sb;

	if (stat(path, &sb) != 0) {
		if (errno == ENOENT) {
			if (mkdir(path, 0700) != 0) {
				errx(1, "can't create %s: %s", path,
						strerror(errno));
			}
			if (stat(path, &sb) != 0) {
				errx(1, "can't stat newly created %s: %s",
						path, strerror(errno));
			}
		} else {
			errx(1, "can't access %s: %s", path, strerror(errno));
		}
	}

	if (!S_ISDIR(sb.st_mode))
		errx(1, "%s is not a directory", path);

	if (sb.st_uid != 0 && sb.st_uid != getuid())
		errx(1, "bad owner on %s", path);

	if ((sb.st_mode & 022) != 0)
		errx(1, "bad permissions on %s", path);
}


/*
 * Check the configuration file.
 *
 * Exits program with error message if anything is wrong.
 */
void
check_config_file(char *path)
{
	struct stat sb;

	if (stat(path, &sb) != 0) {
		/* User hasn't created a config file, that's perfectly fine. */
		if (errno == ENOENT) {
			return;
		} else {
			errx(1, "can't access %s: %s", path, strerror(errno));
		}
	}

	if (!S_ISREG(sb.st_mode))
		errx(1, "%s is not a regular file", path);

	if (sb.st_uid != 0 && sb.st_uid != getuid())
		errx(1, "bad owner on %s", path);

	if ((sb.st_mode & 022) != 0)
		errx(1, "bad permissions on %s", path);
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
	char path[MAXPATHLEN];

	snprintf(path, MAXPATHLEN, "%ls/.mdp", home);
	check_config_directory(path);

	snprintf(path, MAXPATHLEN, "%ls/.mdp/passwords", home);
	check_config_file(path);

	snprintf(path, MAXPATHLEN, "%ls/.mdp/config", home);
	check_config_file(path);

	fp = fopen(path, "r");
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

	snprintf(path, MAXPATHLEN, "%ls/.mdp/config", home);

	fp = fopen(path, "r");
	if (fp == NULL)
		return;

	while (fgets(line, sizeof(line), fp)) {
		mbstowcs(wline, line, 128);
		process_config_line(path, wline, linenum++);
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
WINDOW *
init_curses()
{
	WINDOW *screen;
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

	return screen;
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
#if 0
char *
spawn_editor(struct hook *hook, char *in)
{
	char *argv[128];
	char out[4096];
	int pipe_in_fd[2];	// {read, write}
	int pipe_out_fd[2];
	int status;
	size_t len;
	int i;

	/* Parse the cmd to argv for direct execution (no shell), this will
	 * alter the hook's handler, make sure you are aware of this if one day
	 * you decide to make this dispatcher survive one command. */
	if (hook->use_shell != 1) {
		for (i = 0; i < 128; i++) {
			argv[i] = strsep(&hook->handler, "\t ");
			if (argv[i] == NULL) break;
		}
	}

	len = strlen(in);

	pipe(pipe_in_fd);
	pipe(pipe_out_fd);

	switch (fork()) {
	case -1:
		errx(1, "spawn: %s", strerror(errno));
		;;
	case 0:
		/* Child process pipe dance. */
		if (dup2(pipe_in_fd[0], STDIN_FILENO) == -1)
			errx(1, "dup2 (child stdin): %s", strerror(errno));
		if (pipe_in_fd[0] != STDIN_FILENO)
			close(pipe_in_fd[0]);
		close(pipe_in_fd[1]);

		if (dup2(pipe_out_fd[1], STDOUT_FILENO) == -1)
			errx(1, "dup2 (child stdout): %s", strerror(errno));
		if (pipe_out_fd[1] != STDOUT_FILENO)
			close(pipe_out_fd[1]);
		close(pipe_out_fd[0]);

		if (hook->use_shell == 1) {
			execl("/bin/sh", "/bin/sh", "-c", hook->handler,
					(char*)NULL);
		} else {
			execvp(argv[0], argv);
		}
		errx(1, "couldn't execute '%s': %s", hook->handler,
				strerror(errno));
	default:
		/* Parent process. */
		write(pipe_in_fd[1], in, len);
		close(pipe_in_fd[0]);
		close(pipe_in_fd[1]);
	}

	len = read(pipe_out_fd[0], out, sizeof out);
	out[len] = '\0';
	close(pipe_out_fd[0]);
	close(pipe_out_fd[1]);

	wait(&status);

	return xstrdup(out);
}
#endif


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
gpg_decode(char **keywords, int use_pager)
{
	// char argv[128][128] = { "-q", "--decrypt", "/tmp/passwords.gpg"};
	char in[128] = "woot";
	wchar_t *results[MAX_RESULTS];
	/* in/out from the child's perspective */
	int pout[2];	// {read, write}
	int status;
	size_t len = 0;
	int i;
	WINDOW *screen;

	len = strlen(in);

	if (pipe(pout) != 0)
		err(1, "gpg_decode pipe(pout)");

	switch (fork()) {
	case -1:
		err(1, "gpg_decode fork");
		;;
	case 0:
		/* Child process pipe dance. */
		if (close(pout[0]))
			err(1, "child close(pout[0])");

		if (dup2(pout[1], STDOUT_FILENO) == -1)
			err(1, "dup2 (child stdout)");

		if (pout[1] != STDOUT_FILENO)
			if (close(pout[1]))
				err(1, "child close(pipe_out_fd[1])");

		execlp("/usr/local/bin/gpg", "-q", "--decrypt",
				"/tmp/passwords.gpg", NULL);
		err(1, "couldn't execute");
		/* NOTREACHED */
	default:
		/* Parent process. Nopping. */
		break;
	}

	/* We are the parent. Close the child side of the pipe. */
	if (close(pout[1]) != 0)
		err(1, "close(pout[1])");

	int idx = 0;
	char line[256];
	wchar_t wline[256];
	FILE *fp = fdopen(pout[0], "r");

	while (fgets(line, sizeof(line), fp)) {
		mbstowcs(wline, line, 128);
		strip_trailing_whitespaces(wline);
		if (line_matches(wline, keywords)) {
			if (use_pager) {
				if (idx < MAX_RESULTS - 1)
					results[idx] = wcsdup(wline);
				idx++;

			} else {
				printf("%ls\n", wline);
			}
		}
	}

	if (fclose(fp) != 0)
		err(1, "close(pout[0])");

	if (wait(&status) == -1)
		err(1, "wait()");

	if (use_pager) {
		screen = init_curses();

		if (idx >= MAX_RESULTS || idx >= window_height)
			errx(1, "too many results, please refine your search");

		int left_offset = (window_width - wcslen(wline)) / 2;
		int top_offset = (window_height - idx) / 2;

		wcstombs(line, wline, 256);

		wmove(screen, top_offset, left_offset);
		wprintw(screen, line);
		refresh();
		sleep(5);

		shutdown_curses();
	}
}


void
usage()
{
	printf("usage: mdp [-ec] [keyword ...]\n");
	exit(-1);
}


int
main(int ac, char **av)
{
	char *t;
	int opt, mode = MODE_SHOW;
	extern int optind, optreset;

	if (ac < 2)
		usage();

	setlocale(LC_ALL, "");

	/* Populate $HOME */
	t = getenv("HOME");
	mbstowcs(home, t, MAXPATHLEN);
	if (home == NULL || *home == '\0')
		errx(0, "Unknown variable '$HOME'.");

	/* Populate $EDITOR */
	t = getenv("EDITOR");
	mbstowcs(editor, t, MAXPATHLEN);

	check_config();

	read_config();

	while ((opt = getopt(ac, av, "ecr")) != -1) {
		switch (opt) {
		case 'e':
			mode = MODE_EDIT;
			printf("EDIT!\n");
			break;
		case 'c':
			mode = MODE_CREATE;
			printf("CREATE\n");
			break;
		case 'r':
			mode = MODE_RAW;
			printf("RAW\n");
			break;
		default:
			usage();
		}
	}

	ac -= optind;
	av += optind;

	if (ac == 0)
		usage();

	/* Decide if we use the internal pager or just dump to screen. */
	if (mode == MODE_RAW) {
		gpg_decode(av, 0);
	} else {
		gpg_decode(av, 1);
	}

	return 0;
}
