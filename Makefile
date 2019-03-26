SRC = irec.c compiler.c writer.c datastructs.c precompiler.c common.c error.c parser.c

compile:
	@echo -n "#define COMMIT_ID \"" > commitid.h
	@git rev-parse --short HEAD | tr -d '\n' >> commitid.h
	@echo "\"" >> commitid.h
	gcc -Wall -g -o irec $(SRC)

all: compile

clean:
	rm irec

test: compile
	./tests.sh

install: compile
	cp irec /usr/bin/irec

uninstall:
	rm /usr/bin/irec
