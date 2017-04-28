CFLAGS=$(shell pkg-config --cflags gio-2.0 purple) -g -Wall
LDFLAGS+=$(shell pkg-config --libs gio-2.0 purple)

all:: irisd

irisd: purple.o gdbus.o irisd.o

test:: irisd
	G_MESSAGES_DEBUG=all ./irisd -c config -dd

clean::
	rm -f *.o iris irisd *~ core.*
