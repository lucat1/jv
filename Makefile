all:
	gcc -o jv jv.c

run: all
	cat example.json | ./jv
