BINARY=mdp
PREFIX?=/usr/local
CC?=clang

CFLAGS+=-Wall --std=c99

CFLAGS+=-ggdb -O1

all: ${BINARY}

${BINARY}: main.o wcslcpy.o strdelim.o strlcpy.o
	$(CC) -lncurses -o ${BINARY} main.o wcslcpy.o strdelim.o strlcpy.o

mdp_tests: main.c wcslcpy.o strdelim.o strlcpy.o test.c
	$(CC) ${CFLAGS} -D TESTING -c main.c -o main.o
	$(CC) ${CFLAGS} -c test.c -o test.o
	$(CC) -ggdb -o run_tests main.o wcslcpy.o strdelim.o strlcpy.o test.o

tests: mdp_tests
	./run_tests

main.o: main.c
	$(CC) ${CFLAGS} -c main.c -o main.o
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
	rm -f ${BINARY} main.o wcslcpy.o strdelim.o strlcpy.o test.o run_tests *core
