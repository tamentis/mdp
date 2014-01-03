#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <strings.h>

#include "lock.h"

#define action_is(x) strcmp(av[2], x) == 0

int
main(int ac, char **av)
{
	(void)(ac);

	lock_path = strdup(av[1]);

	if (action_is("lock_exists")) {
		printf("%d\n", lock_exists());
	} else if (action_is("lock_set")) {
		lock_set();
	} else if (action_is("lock_unset")) {
		lock_unset();
	}

	return EXIT_SUCCESS;
}
