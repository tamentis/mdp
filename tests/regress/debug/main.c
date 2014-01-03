#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "debug.h"

int
main(int ac, char **av)
{
	(void)(ac);

	debug_enabled = false;
	debug("nope--%s--", av[1]);
	debug_enabled = true;
	debug("yup--%s--", av[1]);

	return EXIT_SUCCESS;
}
