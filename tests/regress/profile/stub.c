#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <strings.h>

#include "array.h"
#include "str.h"
#include "profile.h"

static int
fprint_passwords(char **av)
{
	struct profile *p;

	p = profile_new("wat");
	p->character_count = atoi(av[2]);
	p->password_count = atoi(av[3]);
	p->character_set = mbs_duplicate_as_wcs(av[4]);

	profile_fprint_passwords(stdout, p);

	return EXIT_SUCCESS;
}

static int
get_from_name(char **av)
{
	struct profile *p;

	p = profile_new("banana");
	profile_register(p);

	p = profile_get_from_name(av[2]);
	if (p == NULL) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int
main(int ac, char **av)
{
	(void)(ac);

	if (strcmp(av[1], "fprint_passwords") == 0) {
		return fprint_passwords(av);
	} else {
		return get_from_name(av);
	}
}
