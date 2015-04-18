/* Public domain */

/*
 * This was looted from:
 * https://github.com/mikejsavage/safebfuns
 */

#ifndef _EXPLICIT_BZERO_H_
#define _EXPLICIT_BZERO_H_

#ifdef HAS_NO_EXPLICIT_BZERO

#include <string.h>

#if __clang__
	/*
	 * http://clang.llvm.org/docs/LanguageExtensions.html#feature-checking-macros
	 * http://lists.cs.uiuc.edu/pipermail/cfe-dev/2014-December/040627.html
	 */
	#if __has_attribute( noinline ) && __has_attribute( optnone )
		#define NOOPT __attribute__ (( optnone ))
		#define NOINLINE __attribute__ (( noinline ))
	#else
		#error "require clang with noinline and optnone attributes"
	#endif
#elif __GNUC__
	/*
	 * http://gcc.gnu.org/onlinedocs/gcc/Function-Specific-Option-Pragmas.html
	 * http://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html
	 */
	#if __GNUC__ > 4 || ( __GNUC__ == 4 && __GNUC_MINOR__ >= 4 )
		#define NOOPT __attribute__ (( optimize( 0 ) ))
		#define NOINLINE __attribute__ (( noinline ))
	#else
		#error "require gcc >= 4.4"
	#endif
#else
	#error "unrecognised compiler"
	explode
#endif

NOOPT NOINLINE void explicit_bzero(void * const, const size_t);

#ifndef __EXPLICIT_BZERO_INTERNAL
#undef NOOPT
#undef NOINLINE
#endif /* __EXPLICIT_BZERO_INTERNAL */

#else /* HAS_NO_EXPLICIT_BZERO */
void explicit_bzero(void * const, const size_t);
#endif /* HAS_NO_EXPLICIT_BZERO */


#endif /* _EXPLICIT_BZERO_H_ */
