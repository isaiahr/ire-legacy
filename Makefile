all: irec.c
	gcc -Wall -o irec irec.c compiler.c writer.c

clean:
	rm irec
