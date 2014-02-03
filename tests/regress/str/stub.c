#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "str.h"
#include "utils.h"


static int
join_list_wrapper(char **av)
{
	const char sep = av[2][0];
	int count = atoi(av[3]);
	char *output;

	output = join_list(sep, count, av + 4);

	printf("%s\n", output);

	return EXIT_SUCCESS;
}


static int
join_wrapper(char **av)
{
	const char sep = av[2][0];
	char *base = av[3];
	char *suffix = av[4];
	char *output;

	output = join(sep, base, suffix);

	printf("%s\n", output);

	return EXIT_SUCCESS;
}


int
main(int ac, char **av)
{
	(void)(ac);

	if (strcmp(av[1], "join") == 0) {
		return join_wrapper(av);
	} else if (strcmp(av[1], "join_list") == 0) {
		return join_list_wrapper(av);
	} else {
		return EXIT_FAILURE;
	}
}
