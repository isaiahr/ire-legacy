/**
 *
 * ast_manip.c - contains funcs for manipulating the ast.
 * 
 */

#include<stdarg.h>
#include<stdlib.h>
#include<string.h>

#include"core/common.h"
#include"ast_manip.h"

VarList* add_varlist(VarList* vl, Variable* var){
    if(vl == NULL){
        VarList* vl = malloc(sizeof(struct VarList));
        vl->var = var;
        vl->next = NULL;
        return vl;
    }
    VarList* orig = vl;
    while(vl->next != NULL){
        vl = vl->next;
    }
    vl->next = malloc(sizeof(struct VarList));
    vl->next->var = var;
    vl->next->next = NULL;
    return orig;
} 



Type* proc_type(char* ident, Program* prog){    
    TypeList* cur = prog->types;
    TypeList* prev = NULL;
    while(cur != NULL){
        if(strcmp(cur->type->identifier, ident) == 0){
            return cur->type;
        }
        prev = cur;
        cur = cur->next;
    }
    int s = strlen(ident);
    if(ident[s-1] == ']' && ident[s-2] == '['){
        char* subt = malloc(s-1);
        memcpy(subt, ident, s-2);
        subt[s-2] = 0;
        Type* t = proc_type(subt, prog);
        if(t != NULL){
            // construct array type
            prog->type_count += 1;
            Type* res = malloc(sizeof(struct Type));
            res->array_subtype = t;
            res->width = 64;
            res->llvm = malloc(strlen(t->llvm)+2);
            memcpy(res->llvm, t->llvm, strlen(t->llvm));
            res->llvm[strlen(t->llvm)] = '*';
            res->llvm[strlen(t->llvm)+1] = 0;
            res->identifier = ident;
            TypeList* new = malloc(sizeof(struct TypeList));
            new->next = NULL;
            new->type = res;
            prev->next = new;
            free(subt);
            return res;
        }
        free(subt);
    }
    return NULL;
}

// make temp var
Variable* mkvar(Function* func, Scope* scope, Type* t){
    Variable* var = malloc(sizeof(struct Variable));
    var->inited = 0;
    var->identifier = NULL;
    var->meta = NULL;
    var->type = t;
    if(scope == NULL)
        func->vars = add_varlist(func->vars, var);
    else
        scope->vars = add_varlist(scope->vars, var);
    return var;
}

// make void variable.
Variable* voidval(Function* func, Scope* scope, Type* t){
    Variable* var = malloc(sizeof(struct Variable));
    var->meta = NULL;
    var->inited = 0;
    var->identifier = NULL;
    var->type = t;
    return var;
}

Statement* mkinit(Variable* var){
    Statement* stmt = malloc(sizeof(struct Statement));
    VarInit* v = malloc(sizeof(struct VarInit));
    stmt->stmt = v;
    stmt->type = S_VARINIT;
    v->var = var;
    return stmt;
}

// make named var
Variable* mknvar(Function* func, Scope* scope, char* str, Type* t){
    Variable* data = malloc(sizeof (struct Variable));
    data->inited = 0;
    data->type = t;
    data->meta = NULL;
    data->identifier = str;
    if(scope != NULL){
        scope->vars = add_varlist(scope->vars, data);
    }
    else{
        func->vars = add_varlist(func->vars, data);
    }
    return data;
}

Variable* proc_var(char* str, Scope* scope, Function* func){
    if(scope != NULL){
        VarList* l = scope->vars;
        while(l != NULL){
            if((l->var->identifier != NULL) && strcmp(str, l->var->identifier) == 0){
                return l->var;
            }
            l = l->next;
        }
        return proc_var(str, scope->parent, func);
    }
    VarList* v = func->vars;
    while(v != NULL){
        if((v->var->identifier != NULL) && strcmp(str, v->var->identifier) == 0){
            return v->var;
        }
        v = v->next;
    }
    VarList* p = func->params;
    while(p != NULL){
        if(strcmp(str, p->var->identifier) == 0){
            return p->var;
        }
        p = p->next;
    }
    return NULL;
}

Function* proc_func(char* funcname, Program* prog){
    for(int i = 0; i < prog->func_count; i++){
        if(strcmp(funcname, prog->funcs[i].name) == 0){
            return &prog->funcs[i];
        }
    }
    return NULL;
}

Type* arr_subtype(Type* arr, Program* p){
    return arr->array_subtype;
}

char* clone(char* str){
    char* b = malloc(strlen(str)+1);
    memcpy(b, str, strlen(str)+1);
    return b;
}

void add_stmt_func(Statement* stmt, Function* func, Scope* scope){
    if(scope != NULL){
        if(scope->body == NULL){
            scope->body = malloc(sizeof(struct Body));
            scope->body->stmt = stmt;
            scope->body->next = NULL;
            return;
        }  
        Body* b = scope->body;
        while(b->next != NULL){
            b = b->next;
        }
        b->next = malloc(sizeof (struct Body));
        b->next->stmt = stmt;
        b->next->next = NULL;
        return;
    }
    if(func->body == NULL){
        func->body = malloc(sizeof (struct Body));
        func->body->stmt = stmt;
        func->body->next = NULL;
        return;
    }
    Body* b = func->body;
    while(b->next != NULL){
        b = b->next;
    }
    b->next = malloc(sizeof (struct Body));
    b->next->stmt = stmt;
    b->next->next = NULL;
    return;
}

int verify_types(Type* want, ...){
    va_list args;
    va_start(args, want);
    va_end(args);
    while(1){
        Variable* t = va_arg(args, Variable*);
        if(t == NULL)
            break;
        if(t->type == NULL || t->type != want){
            va_end(args);
            return 1;
        }
    }
    va_end(args);
    return 0;
}

// 1 = valid and exists.
// 2 = non existent
// 3 = invalid and exists.
int check_valid_access_helper(Variable* var, TypeStructure* ts, char* member, TagList* tags){
    if(ts->sub == NULL){
        if(strcmp(ts->identifier, member) == 0){
            return 1;
        }
        return 2;
    }
    TypeStructure* cur = ts->sub;
    while(cur != NULL){
        int res = check_valid_access_helper(var, cur, member, tags);
        if(res == 3){
            return 3;
        }
        if(res == 2){
            cur = cur->next;
            continue;
        }
        // res = 1. check if tag is guarded
        if(ts->mode == S_MODE_AND){
           // ok.
            return 1;
        }
        // S_MODE_OR
        TagList* tag = tags;
        while(tag != NULL){
            if(var == tag->var){
                if(strcmp(tag->tag, cur->segment) == 0){
                    // NOTE: does not check validity. (ok for now)
                    return 1;
                }
            }
            tag = tag->next;
        }
        cur = cur->next;
    }
    return 2;
}

int check_valid_access(Variable* var, char* member, ScopeMetadata* meta){
    int res;
    if(meta == NULL){
        res = check_valid_access_helper(var, var->type->ts, member, NULL);
    }
    else{
        res = check_valid_access_helper(var, var->type->ts, member, meta->tags);
    }
    return res == 1;
}

