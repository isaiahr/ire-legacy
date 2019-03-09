#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<getopt.h>
#include<sys/types.h>
#include<sys/wait.h>
#include"irec.h"
#include"writer.h"

void write_header(State* state){
    fprintf(state->fp, ".global _start\n.text\n"); 
}

void write_varref(char* ref, State* state){
    if(ISNUMERIC(ref[0])){
        //move immediate number to eax.
        fprintf(state->fp, "movq $%s, %%rax\n", ref);
    }
    else{} //todo support referencing vars.
}

void write_funcreturn(State* state){
    fprintf(state->fp, "%s\n", "ret");
}

void write_funcdef(char* func, State* state){
    fprintf(state->fp, "%s:\n", func);
}
void write_asm(char* str, State* state){
    fprintf(state->fp, "%s\n", str);
}

void write_funcall(char* token, State* state){
    int i = 0;
    while(token[i] != '(') i++;
    char* func = malloc(i+1);
    memcpy(func, token, i);
    func[i] = 0;
    fprintf(state->fp, "call %s\n", func);
}
