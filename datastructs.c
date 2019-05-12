#include<stddef.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"datastructs.h"



/**
* Adds function with name func as a function
* returns a pointer to the function if successful.
*/


/***
Function* add_func(char* func, int defined, State* state){
    if(ref_func(func, state) != NULL){
        return NULL;
    }
    Function* fun;
    fun = (Function*) malloc(sizeof(Function));
    fun->name = func;
    fun->max_offset = 0;
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
Variable* add_fakevar(Function* fun, State* state){
    // creates a fake variable for internal use.
    // this is not linked into the list of vars.
    List* l = state->variables;
    while(l != NULL){
        Variable* I = (Variable*) l->data;
        if(I->func == fun){
            I->offset = I->offset + 8;
            if(I->func->max_offset < I->offset){
                I->func->max_offset = I->offset;
            }
        }
        l = l->next;
    }
    Variable* var;
    var = (Variable*) malloc(sizeof(Variable));
    var->offset = 0;
    var->num = -1;
    Type* t = (Type*) malloc(sizeof(Type));
    t->id = VARTYPE_INTEGER;
    var->type = t;
    //state->tempnum += 1;
    //var->num = state->tempnum;
    return var;
}

Variable* add_var(Function* fun, char* varn, Type* type, State* state){
    if(fun == NULL || type == NULL){
        return NULL;
    }
    if(ref_var(fun, varn, state) != NULL){
        return NULL;
    }
    Variable* var;
    var = (Variable*) malloc(sizeof(Variable));
    var->name = varn;
    var->func = fun;
    var->type = type;
    var->num = -1;
    var->offset = 0;
    var->write_name = malloc(strlen(varn)+strlen(fun->name)+2);
    snprintf(var->write_name, strlen(varn)+strlen(fun->name)+2, "v%s%s", varn, fun->name);
    //state->tempnum += 1;
    //var->num = state->tempnum;
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
        Variable* I = (Variable*) l->data;
        if(I->func == fun){
            I->offset = I->offset + 8;
            if(I->func->max_offset < I->offset){
                I->func->max_offset = I->offset;
            }
        }
        l = l->next;
    }
    Variable* I = (Variable*) l->data;
    if(I->func == fun){
        I->offset = I->offset + 8;
        if(I->func->max_offset < I->offset){
            I->func->max_offset = I->offset;
        }
    }
    l->next = newv;
    return var;
}

Variable* ref_var(Function* func, char* varn, State* state){
    List* l = state->variables;
    while(l != NULL){
        Variable* v = (Variable*) l->data;
        if(strcmp(v->func->name, func->name) == 0 && strcmp(v->name, varn) == 0){
            return v;
        }
        l = l->next;
    }
    return NULL;
}
Type* add_type(char* type, int id, State* state){
    char* typea = (char*) malloc(strlen(type)+3);
    memcpy(typea, type, strlen(type));
    typea[strlen(type)] = '[';
    typea[strlen(type)+1] = ']';
    typea[strlen(type)+2] = 0;
    add_type_(typea, -id, state);
    Type* t = add_type_(type, id, state);
    return t;
}
Type* add_type_(char* type, int id, State* state){
    if(ref_type(type, state) != NULL){
        return NULL; // type already exists
    }
    Type* t = (Type*) malloc(sizeof(Type));
    t->name = type;
    t->id = id;
    t->composite = NULL;
    t->functions = NULL;
    t->operators = NULL;
    List* newl = (List*) malloc(sizeof(List));
    newl->next = NULL;
    newl->data = t;
    if(state->types == NULL){
        state->types = newl;
        return t;
    }
    List* l = state->types;
    while(l->next != NULL){
        l = l->next;
    }
    l->next = newl;
    return t;
}

Type* ref_type(char* type, State* state){
    List* l = state->types;
    while(l != NULL){
        Type* t = (Type*) l->data;
        if(strcmp(t->name, type) == 0){
            return t;
        }
        l = l->next;
    }
    return NULL;
}

*/
