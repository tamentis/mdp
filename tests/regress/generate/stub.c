#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <strings.h>

#include "generate.h"

int
main(int ac, char **av)
{
	(void)(ac);

	generate_passwords(atoi(av[1]), atoi(av[2]));

	return EXIT_SUCCESS;
}
