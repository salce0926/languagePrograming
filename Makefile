#
# Makefile for LP
#
OBJS=token-list.o id-list.o scan.o

all: scan

scan: ${OBJS}
	${CC} -o $@ $^

clean:
	${RM} *.o scan *~