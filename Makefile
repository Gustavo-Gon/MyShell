CC=gcc
CFLAGS=-I.

mysh: myshell.c
	$(CC) -o mysh myshell.c $(CFLAGS)

clean:
	rm -f mysh