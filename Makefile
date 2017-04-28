CFLAGS=$(shell pkg-config --cflags gio-2.0 purple) -g -Wall
LDFLAGS+=$(shell pkg-config --libs gio-2.0 purple)

all:: iris

iris: iris.o purple.o gdbus.o

test:: iris
	./iris -c config -dd

clean::
	rm -f *.o iris *~ core.*
