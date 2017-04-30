CFLAGS=$(shell pkg-config --cflags gio-2.0 purple) -g -Wall
LDFLAGS+=$(shell pkg-config --libs gio-2.0 purple)

all:: irisd

irisd: purple.o gdbus.o irisd.o

test:: irisd
	G_MESSAGES_DEBUG=all ./irisd -c config -d

install:
	install -D irisd ${DESTDIR}/usr/sbin/irisd
	install -D iris-tellall.sh ${DESTDIR}/usr/bin/iris-tellall
	install -D dbus/iris.conf ${DESTDIR}/etc/dbus-1/net.utopiabound.iris.conf
	install -d ${DESTDIR}/var/lib/iris
	@if [ -d /usr/lib/systemd/ ]; then \
		echo "install irisd.service"; \
		install -D irisd.service ${DESTDIR}/usr/lib/systemd/system/irisd.service; \
	else \
		echo "install init.d/irisd"; \
		install -D iris.initd ${DESTDIR}/etc/init.d/irisd; \
	fi

clean::
	rm -f *.o iris irisd *~ core.*
