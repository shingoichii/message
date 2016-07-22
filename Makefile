# Makefile for network samples
# 2012-07-12 by si

CFLAGS = -g -Wall
LDFLAGS = -Wall
LIBS = -lpthread
EXECUTABLES = msgbd-server msgbd-client

ALL : ${EXECUTABLES}

clean :
	rm -f *.o *~ ${EXECUTABLES} *.exe

msgbd-server : server-skelton.o server-doit.o msgbd-util.o
	cc ${CFLAGS} ${LDFLAGS} -o $@ server-skelton.o server-doit.o msgbd-util.o ${LIBS}

msgbd-client : client-skelton.o client-doit.o msgbd-util.o
	cc ${CFLAGS} ${LDFLAGS} -o $@ client-skelton.o client-doit.o msgbd-util.o

server-skelton.o : server-skelton.c msgbd.h
server-doit.o : server-doit.c msgbd.h
client-skelton.o : client-skelton.c msgbd.h
client-doit.o : client-doit.c msgbd.h
msgbd-util.o : msgbd-util.c msgbd.h
