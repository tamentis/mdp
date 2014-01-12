#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <strings.h>

#include "config.h"


static int
config_check_directory_wrapper(char **av)
{
	char *dirname = av[2];

	config_check_directory(dirname);

	return EXIT_SUCCESS;
}


int
main(int ac, char **av)
{
	(void)(ac);

	if (strcmp(av[1], "config_check_directory") == 0) {
		return config_check_directory_wrapper(av);
	} else {
		return EXIT_FAILURE;
	}
}
