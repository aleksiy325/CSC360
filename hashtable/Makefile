CC= gcc
CFLAGS= -w -O  -g
SRCS= main.c hashtable.c
OBJS= main.o hashtable.o
PROGRAM= tabletest
MKDEP= gccmakedep

${PROGRAM}:	${OBJS}
	${CC} ${CFLAGS} ${OBJS} -o ${PROGRAM}

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f ${OBJS} ${PROGRAM}

depend:
	${MKDEP} ${CFLAGS} ${SRCS}
