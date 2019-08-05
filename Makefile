SRC = irec.c \
common/compiler.c common/common.c common/error.c \
precompiler/precompiler.c \
parser/parser.c parser/lexer.c parser/parser_arith.c parser/parser_expr.c \
parser/parser_stmt.c parser/parser_type.c parser/parseutils.c \
ast/semantic.c \
codegen/writer.c codegen/codegenasm.c codegen/codegenllvm.c



compile:
	@mkdir -p build
	@cd runtime && \
	xxd -i pre.s > ../build/pre_s.h && \
	xxd -i pre.ll > ../build/pre_ll.h
	@echo -n "#define COMMIT_ID \"" > build/commitid.h
	@git rev-parse --short HEAD | tr -d '\n' >> build/commitid.h
	@echo "\"" >> build/commitid.h
	gcc -I./ -Wall -g -o irec $(SRC)

all: compile

clean:
	rm irec

test: compile
	@cd tests && \
	./tests.sh

install: compile
	cp irec /usr/bin/irec

uninstall:
	rm /usr/bin/irec
