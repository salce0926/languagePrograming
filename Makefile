#
# Makefile for LP
#
OBJS=token-list.o id-list.o

all: main

main: ${OBJS}
	${CC} -o $@ $^

clean:
	${RM} *.o token-list *~