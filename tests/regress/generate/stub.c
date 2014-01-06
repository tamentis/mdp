#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <strings.h>

#include "array.h"
#include "profile.h"

int
main(int ac, char **av)
{
	(void)(ac);
	struct profile *p;

	p = profile_new("wat");
	p->character_count = atoi(av[1]);
	p->password_count = atoi(av[2]);
	p->character_set = av[3];

	profile_fprint_passwords(stdout, p);

	return EXIT_SUCCESS;
}
