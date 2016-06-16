CC= gcc
CFLAGS= -w -O  -g
SRCS= main.c queue.c
OBJS= main.o queue.o
PROGRAM= queuetest
MKDEP= gccmakedep

${PROGRAM}:	${OBJS}
	${CC} ${CFLAGS} ${OBJS} -o ${PROGRAM}

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f ${OBJS} ${PROGRAM}

depend:
	${MKDEP} ${CFLAGS} ${SRCS}
