#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <strings.h>

#include "editor.h"


static int
editor_is_vim_wrapper(char **av)
{
	char *command = av[2];
	if (editor_is_vim(command)) {
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
}


int
main(int ac, char **av)
{
	(void)(ac);

	if (strcmp(av[1], "editor_is_vim") == 0) {
		return editor_is_vim_wrapper(av);
	} else {
		return EXIT_FAILURE;
	}
}
