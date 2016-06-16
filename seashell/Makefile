CC= gcc
CFLAGS= -w -O  -g
SRCS= seashell.c 
OBJS= seashell.o
PROGRAM= seashell
MKDEP= gccmakedep

${PROGRAM}:	${OBJS}
	${CC} ${CFLAGS} ${OBJS} -o ${PROGRAM}

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f ${OBJS} ${PROGRAM}

depend:
	${MKDEP} ${CFLAGS} ${SRCS}
