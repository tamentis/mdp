#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <strings.h>

#include "str.h"
#include "utils.h"


static int
join_wrapper(char **av)
{
	const char sep = av[2][0];
	const char *base = av[3];
	const char *suffix = av[4];
	char *output;

	output = join(sep, base, suffix);

	printf("%s\n", output);

	return EXIT_SUCCESS;
}


static int
join_path_wrapper(char **av)
{
	const char *base = av[2];
	const char *suffix = av[3];
	char *output;

	output = join_path(base, suffix);

	printf("%s\n", output);

	return EXIT_SUCCESS;
}


static int
file_exists_wrapper(char **av)
{
	const char *filename = av[2];

	if (file_exists(filename)) {
		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}


int
main(int ac, char **av)
{
	(void)(ac);

	if (strcmp(av[1], "join") == 0) {
		return join_wrapper(av);
	} else if (strcmp(av[1], "join_path") == 0) {
		return join_path_wrapper(av);
	} else if (strcmp(av[1], "file_exists") == 0) {
		return file_exists_wrapper(av);
	} else {
		return EXIT_FAILURE;
	}
}
