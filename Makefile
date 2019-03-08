all: irec.c
	gcc -Wall -o irec irec.c

clean:
	rm irec
