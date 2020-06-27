CC = gcc
CFLAGS += -Wall

CFLAGS += $(shell pkg-config --cflags json-c)
LDFLAGS += $(shell pkg-config --libs json-c)

all:
	$(CC) $(CFLAGS) -o jv jv.c $(LDFLAGS)

run: all
	cat example.json | ./jv
