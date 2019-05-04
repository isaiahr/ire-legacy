#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include"datastructs.h"
#include"lexer.h"
#include"parser.h"
#include"compiler.h"
#include"writer.h"
#include"common.h"
#include"error.h"
#include"parser.h"


void compile(State* state, char* data, long sz){
    parse_program(lex(data));
}

/**
void process_token(Token* token, int line, State* state){
    int type = token->type;
    debug(state, ", type %i\n", type);
    // yes i am aware switch exists, but i think this is more readable.
    if(type == COMMENT || type == IMPORT){
        return;
    }
    if(type == ASM){
        annotate(state, "# asm block\n");
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
        annotate(state, "# definition of function %s\n", f->name);
        state->writ_return = 0;
        state->currentfunc = f;
        write_funcdef(f, state);
    }
    if(type == FUNCTION_RETURN){
        annotate(state, "# function %s return\n", state->currentfunc->name);
        if(token->t1 != NULL){
            process_token(token->t1, line, state);
        }
        state->writ_return = 1;
        write_funcreturn(state);
    }
    if(type == INT){
        write_int(token->nt, state);
    }
    if(type == CHAR){
        write_byte(token->chr, state);
    }
    if(type == FUNCTION_END){
        if(!state->writ_return){
            annotate(state, "# function %s return\n", state->currentfunc->name);
            write_int(0, state);
            write_funcreturn(state);
            state->writ_return = 1;
        }
    }
    if(type == STRING){
        annotate(state, "# immediate string %s\n", token->str);
        write_string(token->str, token->nt, state);
    }
    if(type == VARIABLE_REF){
        write_varref(token->var1, state);
    }
    if(type == VARIABLE_DEFN){
        if(ref_var(state->currentfunc, token->str, state) != NULL){
            error(DUPDEFTYPE, line, token->str);
        }
        Variable* var = add_var(state->currentfunc, token->str, token->t, state);
        annotate(state, "# initialize var %s\n", var->name);
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
        annotate(state, "# assign to var %s\n", token->var1->name);
        process_token(token->t1, line, state);
        write_varassign(token->var1, state);
    }
    if(type == ARRAY_ADD){
        annotate(state, "# %s += something\n", token->var1->name);
        process_token(token->t1, line, state);
        write_arradd(token->var1, state);
    }
    if(type == ARRAY_SET){
        annotate(state, "# %s[i] = n\n", token->var1->name);
        process_token(token->t1, line, state); // t1 = i
        Variable* v = add_fakevar(state->currentfunc, state);
        write_varinit(v, state);
        write_varassign(v, state);
        process_token(token->t2, line, state);
        write_arrset(token->var1, v, state);
    }
    if(type == ARRAY_INDEX){
        process_token(token->t1, line, state);
        write_arrind(token->var1, state);
    }

}
*/
