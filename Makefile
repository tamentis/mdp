BINARY=mdp
OBJECTS=main.o gpg.o pager.o utils.o config.o wcslcpy.o strdelim.o strlcpy.o \
	lock.o randpass.o curses.o xmalloc.o results.o keywords.o
PREFIX?=/usr/local
CC?=clang

CFLAGS+=-Wall --std=c99

# Debug
# CFLAGS+=-ggdb -O1

# Linux
# LDFLAGS+=-lbsd

all: ${BINARY}

${BINARY}: ${OBJECTS}
	$(CC) -lncurses ${LDFLAGS} -o ${BINARY} ${OBJECTS}

install: ${BINARY}
	install -m 755 ${BINARY} ${PREFIX}/bin
	install -m 644 ${BINARY}.1 ${PREFIX}/man/man1

mantest:
	nroff -man mdp.1 | less -R

clean:
	rm -f ${BINARY} ${OBJECTS} test.o run_tests *core
