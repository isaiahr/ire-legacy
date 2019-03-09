#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<getopt.h>
#include<sys/types.h>
#include<sys/wait.h>
#include"irec.h"
#include"compiler.h"
#include"writer.h"

void compile(State* state, char* data, long sz){
    write_header(state);
    int index0 = -1;
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
    printf("token %i, %c \n", type, token[0]);
    if(type == FUNCTION_CALL){
        int i = 0;
        while(token[i] != '(') i++;
        int j = match_paren(&token[i]);
        char* new_token = (char*) malloc(j-1);
        memcpy(new_token, &token[i+1], j-2);
        new_token[j-1] = 0;
        printf("inner: %s\n", new_token);
        if(get_token_type(new_token) != INVALID){
            process_token(new_token, state);
        }
        else{
            write_varref(new_token, state);
        }
        write_funcall(token, state);
    }
    if(type == ASM){
        write_asm(token + sizeof(char), state);
    }
    if(type == FUNCTION_DEFN){
        // example: def func { } 
        int i = nextnonwhite(token);
        int j = i+1;
        while(token[j] != ' ' && token[j] != '{') j++;
        char* function = (char*) malloc(j-i+1);
        memcpy(function, &token[i], j-i);
        function[j-i] = 0;
        write_funcdef(function, state);
    }
    if(type == FUNCTION_RETURN){
        int i = nextnonwhite(token);
        write_varref(&token[i], state); 
        write_funcreturn(state);
    }
    if(type == FUNCTION_END){} // TODO 
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
    if(!ISALPHA(token[0])){
        return INVALID; // invalid char
    }
    if(token[0] == 'r' && token[1] == 'e' && token[2] == 't' && token[3] == 'u'
    && token[4] == 'r' && token[5] == 'n') return FUNCTION_RETURN;
    if(token[0] == 'd' && token[1] == 'e' && token[2] == 'f' && token[3] == ' '){
        return FUNCTION_DEFN;
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
