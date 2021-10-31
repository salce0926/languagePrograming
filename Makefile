#
# Makefile for LP
#
OBJS=token-list.o id-list.o scan.o parse.o

all: parse

parse: ${OBJS}
	${CC} -o $@ $^

clean:
	${RM} *.o scan *~