#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include"datastructs.h"
#include"parser.h"
#include"compiler.h"
#include"writer.h"
#include"common.h"
#include"error.h"


void compile(State* state, char* data, long sz){
    int index0 = 0;
    int index1 = -1;
    int multiline = 0;
    int line = 1;
    for(int i = 0; i < sz; i++){
        if(data[i] == '`'){
            if(index1 == -1){
                index1 = i;
                multiline = 1;
            }
            else{
                char* candidate = malloc(i-index1);
                memcpy(candidate, &data[index1], i-index1-1);
                candidate[i-index1-1] = 0;
                debug(state, "processing %s", candidate);
                Token* token = tokenize(candidate, line, state);
                process_token(token, line, state);
                multiline = 0;
                index1 = -1;
            }
            index0 = -1;
        }
        if(data[i] == '\n' && !multiline){
            if(index0 == -1){
                index0 = i+1;
            }
            else{
                char* candidate = malloc(i-index0+1);
                memcpy(candidate, &data[index0], i-index0);
                candidate[i-index0] = 0;
                debug(state, "processing %s", candidate);
                Token* token = tokenize(candidate, line, state);
                process_token(token, line, state);
                index0 = i+1;
            }
        }
        if(data[i] == '\n'){
            line = line+1;
        }
    }
}


void process_token(Token* token, int line, State* state){
    int type = token->type;
    debug(state, ", type %i\n", type);
    // yes i am aware switch exists, but i think this is more readable.
    if(type == COMMENT || type == IMPORT){
        return;
    }
    if(type == ASM){
        write_asm(token->str, state);
    }
    if(type == FUNCTION_DEFN){
        Function *f = ref_func(token->str, state);
        if(f != NULL && f->defined == 1){   
            error(DUPDEFFUNC, line, token->str);
        }
        if(f == NULL){
            f = add_func(token->str, 1, state);
        }
        else{
            f->defined = 1;
        }
        state->writ_return = 0;
        state->currentfunc = f;
        write_funcdef(f, state);
    }
    if(type == FUNCTION_RETURN){
        if(token->t1 != NULL){
            process_token(token->t1, line, state);
        }
        state->writ_return = 1;
        write_funcreturn(state);
    }
    if(type == IMMEDIATE){
        write_immediate(token->nt, state);
    }
    if(type == FUNCTION_END){
        if(!state->writ_return){
            write_immediate(0, state);
            write_funcreturn(state);
            state->writ_return = 1;
        }
    }
    if(type == VARIABLE_REF){
        write_varref(token->var1, state);
    }
    if(type == VARIABLE_DEFN){
        if(ref_var(state->currentfunc, token->str, state) != NULL){
            error(DUPDEFTYPE, line, token->str);
        }
        Variable* var = add_var(state->currentfunc, token->str, token->t, state);
        write_varinit(var, state);
    }
    if(type == FUNCTION_CALL){
        if(token->func == NULL){
            token->func = add_func(token->str, 0,  state);
        }
        process_token(token->t1, line, state);
        write_funcall(token->func, state);
    }
    if(type == ASSIGNMENT){
        process_token(token->t1, line, state);
        write_varassign(token->var1, state);
    }

}
