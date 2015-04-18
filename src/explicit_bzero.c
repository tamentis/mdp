/* Public domain */

/*
 * This was looted from:
 * https://github.com/mikejsavage/safebfuns
 */

#include <string.h>

#define __EXPLICIT_BZERO_INTERNAL
#include "explicit_bzero.h"

NOOPT NOINLINE void
explicit_bzero(void * const buf, const size_t n)
{
	size_t i;
	unsigned char * p = buf;

	for (i = 0; i < n; i++) {
		p[i] = 0;
	}
}
