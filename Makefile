CC = gcc
CFLAGS += -Wall

PREFIX = /usr/local

CFLAGS += $(shell pkg-config --cflags json-c)
LDFLAGS += $(shell pkg-config --libs json-c)

all:
	$(CC) $(CFLAGS) -o jv jv.c $(LDFLAGS)

test: all
	cat example.json | ./jv

install: all
	@echo installing to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f jv ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/jv

uninstall:
	@echo uninstalling from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/jv

.PHONY: all test install uninstall
