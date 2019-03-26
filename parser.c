#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include"datastructs.h"
#include"writer.h"
#include"common.h"
#include"error.h"
#include"parser.h"


// turns a token candidate (typically a line) into a token.
Token* tokenize(char* str, int line, State* state){
    if(str[0] == ' '){
        return tokenize(&str[1], line, state);
    }
    Token* t = malloc(sizeof (Token));
    if(beginswith("import ", str)){
        t->type = IMPORT;
        return t;
    }
    if(beginswith("//", str)){
        t->type = COMMENT;
        return t;
    }
    if(beginswith("def ", str)){
        t->type = FUNCTION_DEFN;
        t->str = copy(str+4, " ", " {");
        if(t->str == NULL){
            error(SYNTAXERROR, line, str);
        }
        validname(t->str, line);
        return t;
    }
    if(beginswith("return ", str)){
        t->type = FUNCTION_RETURN;
        char* ret = copy(str+7, " ", " ");
        t->t1 = tokenize(ret, line, state);
        return t;
    }
    if(str[0] == '}'){
        t->type = FUNCTION_END;
        return t;
    }
    if(str[0] == '`'){
        t->type = ASM;
        t->str = oldcopy(str, 1, strlen(str));
        return t;
    }
    if(ISNUMERIC(str[0])){
        t->type = IMMEDIATE;
        char* b = copy(str, "", " ");
        t->nt = strtol(b, NULL, 10);
        return t;
    }
    char* arr = copy(str, "", "[");
    Variable* v0 = NULL;
    if(arr != NULL && str[strlen(arr)] == '['){
        v0 = ref_var(state->currentfunc, arr, state);
    }
    if(v0 != NULL){
        //array operation
        int indxe = indexchr(str, '=');
        int indxb = indexchr(str, ']');
        if((indxb != -1 && indxe != -1) && (indxe > indxb)){
            t->type = ARRAY_SET;
            char* inner = match_sqparen(str);
            if(inner == NULL){
                error(SYNTAXERROR, line, str);
            }
            t->t1 = tokenize(inner, line, state);
            t->t2 = tokenize(copy(&str[indxe+1], " ", " "), line, state);
            t->var1 = v0;
            return t;
        }
        if(indxb == -1){
            error(SYNTAXERROR, line, str);
        }
        t->type = ARRAY_INDEX;
        t->var1 = v0;
        char* inner = match_sqparen(str);
        if(inner == NULL){
            error(SYNTAXERROR, line, str);
        }
        t->t1 = tokenize(inner, line, state);
        return t;
    }
    char* begin = copy(str, "", " ");
    if(begin != NULL){
        Type* ty = ref_type(begin, state);
        if(ty != NULL){
            t->type = VARIABLE_DEFN;
            t->t = ty;
            char* varname = copy(str+strlen(begin), " ", " ");
            validname(varname, line);
            t->str = varname;
            return t;
        }
        int indxb = indexchr(str, '(');
        int indxe = indexchr(str, '=');
        if(indxb != -1 && indxe == -1){
            t->type = FUNCTION_CALL;
            char* fname = copy(str, "", " (");
            validname(fname, line);
            Function* fun = ref_func(fname, state);
            t->str = fname;
            t->func = fun;// might be null, that is OK.
            char* inner = match_paren(str);
            if(inner == NULL){
                error(SYNTAXERROR, line, str);
            }
            t->t1 = tokenize(inner, line, state);
            return t;
        }
        if(indxe != -1){
            if(indexstr(str, "+=") == indxe-1){
                t->type = ARRAY_ADD;
                char* v = copy(str, "", " +");
                Variable* var = ref_var(state->currentfunc, v, state);
                char* end = copy(str+strlen(v), " +=", "");
                t->var1 = var;
                t->t1 = tokenize(end, line, state);
                return t;
            }
            t->type = ASSIGNMENT;
            Variable* var = ref_var(state->currentfunc, begin, state);
            if(var == NULL){
                error(UNDEFVAR, line, str);
            }
            t->var1 = var;
            char* assigner = copy(&str[indxe+1], " ", " ");
            t->t1 = tokenize(assigner, line, state);
            return t;
        }
        Variable* v = ref_var(state->currentfunc, begin, state);
        if(v != NULL){
            t->type = VARIABLE_REF;
            t->var1 = v;
            return t;
        }
    }
    t->type = INVALID;
    return t;
    

}
int indexchr(char* str, char chr){
    int i = 0;
    while(str[i] != 0){
        if(str[i] == chr)
            return i;
        i++;
    }
    return -1;
}

void validname(char* name, int line){
    if(strlen(name) < 1)
        error(SYNTAXERROR, line, name);
    if(!ISALPHA(name[0]))
        error(SYNTAXERROR, line, name);
    int i = 1;
    while(name[i] != 0){
        if(!(ISALPHA(name[i]) || ISNUMERIC(name[i]))){
            error(SYNTAXERROR, line, name);
        }
        i+= 1;
    }   
}

// example input "((()))"
char* match_paren(char* input){
    int numpar = 1;
    int first = -1;
    int i = 0;
    while(input[i] != 0){
        if(input[i] == '(' && first == -1){
            first = i;
        }
        else if (input[i] == '('){
            numpar += 1;
        }
        else if (input[i] == ')'){
            numpar -= 1;
        }
        if(numpar == 0){
            char* x = oldcopy(input, first+1, i);
            return x;
        }
        i++;
    }
    return NULL;
}

char* match_sqparen(char* input){
    int numpar = 1;
    int first = -1;
    int i = 0;
    while(input[i] != 0){
        if(input[i] == '[' && first == -1){
            first = i;
        }
        else if (input[i] == '['){
            numpar += 1;
        }
        else if (input[i] == ']'){
            numpar -= 1;
        }
        if(numpar == 0){
            char* x = oldcopy(input, first+1, i);
            return x;
        }
        i++;
    }
    return NULL;
}

// returns index of first occurance of st2 in st1.
// so indexstr (a += b) (+=) -> 2
int indexstr(char* str1, char* str2){
    // index
    int i = 0;
    // match index
    int mi = 0;
    while(str1[i+mi] != 0){
        if(str2[mi] == 0){
            return i;
        }
        if(str1[i+mi] == str2[mi]){
            mi += 1;
        }
        else{
            mi = 0;
            i++;
        }
    }
    return -1;

}

char* copy(char* token, char* pass, char* end){
    int i = 0;
    int ind = -1;
    while(token[i] != 0){
        if(ind == -1 && strchr(pass, token[i]) == NULL){
            // first char we dont pass on. set start to here.
            if(ind == -1){
                ind = i;
            }
        }
        else if(strchr(end, token[i]) != NULL && ind != -1){
            // start index is set, and matches end.
            return oldcopy(token, ind, i);
        }
        i++;
    }
    if(ind == -1){
        return NULL;
    }
    return oldcopy(token, ind, i);
}

//returns copy including ind0, but not including ind1
char* oldcopy(char* token, int ind0, int ind1){
    char* result = malloc(ind1-ind0+1);
    memcpy(result, &token[ind0], ind1-ind0);
    result[ind1-ind0] = 0;
    return result;
}


int beginswith(char* begin, char* token){

    int matches = 1;
    int j = 0;
    while(matches && token[j] != 0){
        matches = matches && (begin[j] == token[j]);
        j += 1;
        if(begin[j] == 0)
            return matches;
    }
    return 0;
}
