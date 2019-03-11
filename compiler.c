#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include"datastructs.h"
#include"compiler.h"
#include"writer.h"
#include"common.h"

void compile(State* state, char* data, long sz){
    int index0 = 0;
    int index1 = -1;
    int multiline = 0;
    for(int i = 0; i < sz; i++){
        if(data[i] == '`'){
            if(index1 == -1){
                index1 = i;
                multiline = 1;
            }
            else{
                char* token = malloc(i-index1);
                memcpy(token, &data[index1], i-index1-1);
                token[i-index1-1] = 0;
                process_token(token, state);
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
                char* token = malloc(i-index0+1);
                memcpy(token, &data[index0], i-index0);
                token[i-index0] = 0;
                process_token(token, state);
                index0 = i+1;
            }    
        }
    }
}

// example input "((()))"
int match_paren(char* input){
    int numpar = 1;
    int i = 1;
    while(numpar != 0){
        if(input[i] == '(') numpar++;
        if(input[i] == ')') numpar--;
        i++;
    }
    return i;
}


void process_token(char* token, State* state){
    if(token[0] == ' '){
        return process_token(token + sizeof(char), state);
    }
    int type = get_token_type(token);
    debug(state, "token %i, %s \n", type, token);
    if(type == COMMENT || type == IMPORT){
        return;
    }
    if(type == VARIABLE_DEFN){
        //ie: int a
        int i = nextnonwhite(token);
        int j = endofvarname(&token[i]);
        char* varn = copy(token, i, i+j);
        add_var(state->currentfunc->name, varn, VARTYPE_INTEGER, state);
    }
    if(type == ASSIGNMENT){
        // types of assignments
        // a = 2 (var <- immediate)
        // a = b (var <- var)
        // a = func
        int i = 0;
        while(token[i] != ' ' && token[i] != '=') i++;
        char* a = copy(token, 0, i);
        while(token[i] != '=') i++;
        i = i+1;
        if(token[i] == ' '){
            i = nextnonwhite(&token[i]) + i;
        }
        int j = endofvarname(&token[i]);
        char* b = copy(token, i, i+j);
        debug(state, "assign %s = %s\n", a, b);
        annotate(state, "# %s = %s\n", a, b);
        if(ISNUMERIC(b[0])){
            Variable* vara = ref_var(state->currentfunc->name, a, state);
            write_iassign(vara, b, state);
        }
        else if(get_token_type(b) == FUNCTION_CALL){
            process_token(b, state);
            Variable* vara = ref_var(state->currentfunc->name, a, state);
            write_fassign(vara, state);
        }
        else{
            Variable* vara = ref_var(state->currentfunc->name, a, state);
            Variable* varb = ref_var(state->currentfunc->name, b, state);
            write_vassign(vara, varb, state);
        }
    }
    if(type == FUNCTION_CALL){
        int i = 0;
        while(token[i] != ' ' && token[i] != '(') i++;
        char* funct = copy(token, 0, i);
        Function* fun = ref_func(funct, state);
        if(fun == NULL){
            //assume function will be declared later
            fun = add_func(funct, 0, state);
        }
        while(token[i] != '(') i++;
        int j = match_paren(&token[i]);
        char* new_token = (char*) malloc(j-1);
        memcpy(new_token, &token[i+1], j-2);
        new_token[j-1] = 0;
        annotate(state, "# call function %s \n", funct);
        if(get_token_type(new_token) != INVALID){// TODO bugfix space = bug here
            process_token(new_token, state);
        }
        else{
            Variable *var = ref_var(state->currentfunc->name, new_token, state);
            if(var == NULL){
                if(new_token[0] == 0){
                    new_token = "0";
                }
                write_iref(new_token, state);
            }else{
                write_varref(var, state);
            }
        }
        write_funcall(fun, state);
    }
    if(type == ASM){
        annotate(state, "# asm block\n");
        write_asm(token + sizeof(char), state);
        annotate(state, "# end of asm block\n");
    }
    if(type == FUNCTION_DEFN){
        // example: def func { } 
        int i = nextnonwhite(token);
        int j = i+1;
        while(token[j] != ' ' && token[j] != '{') j++;
        char* function = (char*) malloc(j-i+1);
        memcpy(function, &token[i], j-i);
        function[j-i] = 0;
        Function* f = ref_func(function, state);
        if(f != NULL){
            //TODO error on duplicate defn.
            f->defined = 1;
        }
        else{
            f = add_func(function, 1,  state);
        }
        annotate(state, "# declaration of function %s\n", function);
        write_funcdef(f, state);
        state->currentfunc = f;
        state->writ_return = 0;
    }
    if(type == FUNCTION_RETURN){
        annotate(state, "# return %s\n", token);
        if(strlen(token) == strlen("return")){
            return;
        }
        int i = nextnonwhite(token);
        if(token[i] == 0){
            //just "return"
            return; //process this token in func end.
        }
        int j = endofvarname(&token[i]);
        char* var = copy(token, i, i+j);
        Variable* var0 = ref_var(state->currentfunc->name, var, state);
        if(var0 == NULL){
            write_iref(var, state);
        } else {
            write_varref(var0, state);
        }
        write_funcreturn(state);
        state->writ_return = 1;
    }
    if(type == FUNCTION_END){
        if(!state->writ_return){
            write_funcreturn(state);
        }
        state->currentfunc = NULL;
    }  
}


//returns copy including ind0, but not including ind1
char* copy(char* token, int ind0, int ind1){
    char* result = malloc(ind1-ind0+1);
    memcpy(result, &token[ind0], ind1-ind0);
    result[ind1-ind0] = 0;
    return result;
}

//returns next whitespace/bracket/
int endofvarname(char* str){
    int i = 0;
    char v = str[i];
    while(v != ' ' && v != 0){
        i++;
        v = str[i];
    }
    return i;
}
// returns index of first whitespace.
int nextwhite(char* str){
    int i = 0;
    while(str[i] != ' ')i++;
    return i;
}
//returns index following next whitespace gap.
int nextnonwhite(char* str){
    
    int i = nextwhite(str);
    while(str[i] == ' ')i++;
    return i;
}

int get_token_type(char* token){// TODO ADD TYPE=CONDITIONAL
    int i = 0;
    if(token[0] == '`'){
        return ASM;
    }
    if(token[0] == '}'){
        return FUNCTION_END;
    }
    if(token[0] == '/' && token[1] == '/'){
        return COMMENT;
    }
    if(!ISALPHA(token[0])){
        return INVALID; // invalid char
    }
    if(token[0] == 'r' && token[1] == 'e' && token[2] == 't' && token[3] == 'u'
    && token[4] == 'r' && token[5] == 'n') return FUNCTION_RETURN;
    if(token[0] == 'd' && token[1] == 'e' && token[2] == 'f' && token[3] == ' '){
        return FUNCTION_DEFN;
    }
    if(token[0] == 'i' && token[1] == 'n' && token[2] == 't' && token[3] == ' '){
        return VARIABLE_DEFN;
    }
    while(token[i] != '\0'){
        if(token[i] == '='){
            return ASSIGNMENT;
        }
        if(token[i] == '('){
            return FUNCTION_CALL;
        }
        i += 1;
    }
    return INVALID;
}



char* proc_str(char* data, long max, int* indexed){

   int esc_next = 0;
   int stringalloc = 32;
   int stringind = 0;
   char* string = (char*) malloc(stringalloc);
   for(long i=0; i < max; i++){
      
      char c = data[i];
      if(c == '"' && (esc_next == 0)){
        string[stringind] = '\0';
        esc_next = 0;
        (*indexed) = i;
        return string;
      }
      else if(c == '"' && (esc_next == 1)){
        string[stringind] = c;
        stringind = stringind + 1;
        if(stringind == stringalloc){//allocate more space
             stringalloc = stringalloc * 2;
             string = (char*) realloc(string, stringalloc);
        }
        esc_next = 0;
      }
      else{
         if(c == '\\' && esc_next == 0){
             esc_next = 1;
         }
         else {
            string[stringind] = c;
            stringind=stringind+1;
            if(stringind == stringalloc){
                stringalloc = stringalloc * 2;
                string = (char*) realloc(string, stringalloc);
            }
            esc_next = 0;
         }
      }
      
   }
   return NULL;

}
