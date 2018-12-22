
all: shell 

mycopy: shell.c
	gcc shell.c -Wall -g -o shell

clean:
	rm -f shell 

dist:
	tar -cvf proj7.tar Makefile shell.c
