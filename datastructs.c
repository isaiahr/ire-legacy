#include<stddef.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"datastructs.h"



/**
* Adds function with name func as a function
* returns a pointer to the function if successful.
*/
Function* add_func(char* func, int defined, State* state){
    if(ref_func(func, state) != NULL){
        return NULL;
    }
    Function* fun;
    fun = (Function*) malloc(sizeof(Function));
    fun->name = func;
    fun->defined = defined;
    if(strcmp(fun->name, ENTRYFUNC) == 0){
        fun->write_name = "_start";
    }
    else{
        fun->write_name = malloc(strlen(func)+2);
        snprintf(fun->write_name, strlen(func)+2, "f%s", func);
    }
    List* newf;
    newf = (List*) malloc(sizeof(List));
    newf->next = NULL;
    newf->data = (Function*) fun;
    List* f = state->functions;
    if(f == NULL){
        state->functions = newf;
        return fun;
    }
    while(f->next != NULL){
        f = f->next;
    }
    f->next = newf;
    return fun;
}

Function* ref_func(char* func, State* state){
    List* f = state->functions;
    while(f != NULL){
        if(strcmp(((Function*)f->data)->name, func) == 0){
            return  ((Function*) f->data);
        }
        f = f->next;
    } 
    return NULL; // function not found.
}

Variable* add_var(char* func, char* varn, int type, State* state){
    Function* fun = ref_func(func, state);
    if(fun == NULL){
        return NULL;
    }
    if(ref_var(func, varn, state) != NULL){
        return NULL;
    }
    Variable* var;
    var = (Variable*) malloc(sizeof(Variable));
    var->name = varn;
    var->func = fun;
    var->type = type;
    var->write_name = malloc(strlen(varn)+strlen(func)+2);
    snprintf(var->write_name, strlen(varn)+strlen(func)+2, "v%s%s", varn, func);
    List* newv;
    newv = (List*) malloc(sizeof(List));
    newv->next = NULL;
    newv->data = (Variable*) var;
    List* l = state->variables;
    if(l == NULL){
        state->variables = newv;
        return var;
    }
    while(l->next != NULL){
        l = l->next;
    }
    l->next = newv;
    return var;
}

Variable* ref_var(char* func, char* varn, State* state){
    List* l = state->variables;
    while(l != NULL){
        Variable* v = (Variable*) l->data;
        if(strcmp(v->func->name, func) == 0 && strcmp(v->name, varn) == 0){
            return v;
        }
        l = l->next;
    }
    return NULL;
}
