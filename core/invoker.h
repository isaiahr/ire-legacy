#ifndef __INVOKER_H__
#define __INVOKER_H__

#define ASSEMBLER_PATH "/usr/bin/as"
#define LINKER_PATH    "/usr/bin/ld"
#define LLC_PATH       "/usr/bin/llc"
#define OPT_PATH       "/usr/bin/opt"

#define ASM 1
#define OPT 2
#define LLC 3

void invoke_assembler(char* source, char* dest);
void invoke_linker(char* source, char* dest);
void invoke_llc(char* source, char* dest);
void invoke_opt(char* source, char* dest, int optcode);
char* get_tempfile(int subprocesses);

#endif
