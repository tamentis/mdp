#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "config.h"


static int
config_ensure_directory_wrapper(char **av)
{
	char *dirname = av[2];
	config_ensure_directory(dirname);
	return EXIT_SUCCESS;
}


static int
config_resolve_character_set_wrapper(char **av)
{
	char *value = av[2];
	wchar_t *output = config_resolve_character_set(value);
	printf("%ls\n", output);
	return EXIT_SUCCESS;
}


int
main(int ac, char **av)
{
	(void)(ac);

	if (strcmp(av[1], "config_ensure_directory") == 0) {
		return config_ensure_directory_wrapper(av);
	} else if (strcmp(av[1], "config_resolve_character_set") == 0) {
		return config_resolve_character_set_wrapper(av);
	} else {
		return EXIT_FAILURE;
	}
}
