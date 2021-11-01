#
# Makefile for LP
#
OBJS=token-list.o id-list.o scan.o parse.o
CFLAGS=-std=c89 -pedantic-errors -Werror=pedantic

all: parse

parse: ${OBJS}
	${CC} ${CFLAGS} -o $@ $^

clean:
	${RM} *.o scan *~