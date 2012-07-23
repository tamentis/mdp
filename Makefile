BINARY=mdp
OBJECTS=main.o gpg.o pager.o utils.o config.o wcslcpy.o strdelim.o strlcpy.o
PREFIX?=/usr/local
CC?=clang

CFLAGS+=-Wall --std=c99

CFLAGS+=-ggdb -O1

all: ${BINARY}

${BINARY}: ${OBJECTS}
	$(CC) -lncurses -o ${BINARY} ${OBJECTS}

mdp_tests: main.c wcslcpy.o strdelim.o strlcpy.o test.c
	$(CC) ${CFLAGS} -D TESTING -c main.c -o main.o
	$(CC) ${CFLAGS} -c test.c -o test.o
	$(CC) -ggdb -o run_tests ${OBJECTS} test.o

tests: mdp_tests
	./run_tests

main.o: main.c
	$(CC) ${CFLAGS} -c main.c -o main.o
config.o: config.c
	$(CC) ${CFLAGS} -c config.c -o config.o
gpg.o: gpg.c
	$(CC) ${CFLAGS} -c gpg.c -o gpg.o
utils.o: utils.c
	$(CC) ${CFLAGS} -c utils.c -o utils.o
pager.o: pager.c
	$(CC) ${CFLAGS} -c pager.c -o pager.o
wcslcpy.o: wcslcpy.c
	$(CC) ${CFLAGS} -c wcslcpy.c -o wcslcpy.o
strdelim.o: strdelim.c
	$(CC) ${CFLAGS} -c strdelim.c -o strdelim.o
strlcpy.o: strlcpy.c
	$(CC) ${CFLAGS} -c strlcpy.c -o strlcpy.o

install: ${BINARY}
	install -m 755 ${BINARY} ${PREFIX}/bin
	install -m 644 ${BINARY}.1 ${PREFIX}/man/man1

clean:
	rm -f ${BINARY} ${OBJECTS} test.o run_tests *core
