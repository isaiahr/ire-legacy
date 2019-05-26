#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include"datastructs.h"
#include"pre_ll.h"
#include"irec.h"
#include"commitid.h"


void lwrite_header(State* state){
    fprintf(state->fp, "target triple = \"x86_64-pc-linux-gnu\"\n\n\n");
    fprintf(state->fp, "%s\n", pre_ll);
}

void lwrite_footer(State* state){
    fprintf(state->fp, "\n\n\n!llvm.ident = !{!0}\n");
    fprintf(state->fp, "!0 = !{!\"irec version %s build %s\"}\n", VERSION_STRING, COMMIT_ID);
}

void lwrite_varinit(Variable* var, State* state){
}

void lwrite_funcreturn(Function* func, Variable* var, State* state){
    
}
void lwrite_funcend(Function* func, State* state){
    
}
void lwrite_funcdef(Function* func, State* state){
}

void lwrite_funcall(FunctionCall* func, State* state){
}

void lwrite_varassign(Variable* to, Variable* from, State* state){
}


void lwrite_byte(Variable* to, char byte, State* state){
}

void lwrite_int(Variable* to, int immediate, State* state){
}

void lwrite_indget(Variable* arr, Variable* ind, Variable* to, State* state){
}

void lwrite_indset(Variable* arr, Variable* ind, Variable* from, State* state){
}

void lwrite_addeq(Variable* arr, Variable* delta, State* state){
}

void lwrite_string(Variable* to, char* str, int len, State* state){ 
}

void lwrite_card(Variable* to, Variable* from, State* state){
}
