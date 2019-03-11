all: irec.c
	gcc -Wall -g -o irec irec.c compiler.c writer.c datastructs.c precompiler.c common.c

clean:
	rm irec
