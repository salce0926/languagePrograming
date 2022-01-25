#
# Makefile for LP
#
OBJS=token-list.o id-list.o scan.o parse.o #library.o
CFLAGS=-g -O0 -std=c89 -pedantic-errors -Werror=pedantic --coverage

all: parse

parse: ${OBJS}
	${CC} ${CFLAGS} -o $@ $^

clean:
	${RM} *.o scan *~