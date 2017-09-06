CC=gcc
CFLAGS=-lm -lpthread
INC=

LIBUTILS=out/stringutils.o out/list.o out/base64.o
LIBNET=out/tcpcon.o out/tcpserv.o out/message.o
LIBCORE=out/airplane.o out/collisiondetection.o

default: all

utils: prereq
	$(CC) $(INC) -g -c common/utils/stringutils.c -o out/stringutils.o
	$(CC) $(INC) -g -c common/utils/list.c -o out/list.o
	$(CC) $(INC) -g -c common/utils/base64.c -o out/base64.o

net: prereq
	$(CC) $(INC) -g -c common/net/tcpcon.c -o out/tcpcon.o
	$(CC) $(INC) -g -c common/net/tcpserv.c -o out/tcpserv.o
	$(CC) $(INC) -g -c common/net/message.c -o out/message.o

core: prereq utils net
	$(CC) $(INC) -g -c core/airplane.c -o out/airplane.o $(CFLAGS)
	$(CC) $(INC) -g -c core/collisiondetection.c -o out/collisiondetection.o $(CFLAGS)

pilot: prereq utils net core
	$(CC) $(INC) -g -c core/pilot.c -o out/pilot.o $(CFLAGS)
	$(CC) $(INC) -o out/pilot ${LIBUTILS} ${LIBNET} ${LIBCORE} out/pilot.o $(CFLAGS)

controller: prereq utils net core
	$(CC) $(INC) -g -c core/controller.c -o out/controller.o $(CFLAGS)
	$(CC) $(INC) -o out/controller ${LIBUTILS} ${LIBNET} ${LIBCORE} out/controller.o $(CFLAGS)

mon: prereq utils net core
	$(CC) $(INC) -g -c core/mon.c -o out/mon.o $(CFLAGS)
	$(CC) $(INC) -o out/mon ${LIBUTILS} ${LIBNET} ${LIBCORE} out/mon.o $(CFLAGS)

prereq:
	mkdir -p out

all: prereq controller mon pilot clean

clean:
	rm -f out/*.o

cleanall:
	rm -rf out
