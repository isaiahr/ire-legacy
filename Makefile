compile:
	gcc -Wall -g -o irec irec.c compiler.c writer.c datastructs.c precompiler.c common.c error.c parser.c

all: compile

clean:
	rm irec

test: compile
	./tests.sh
