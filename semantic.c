#include<stdlib.h>
#include<string.h>
#include<stdio.h>

#include"parser.h"
#include"semantic.h"

/**
 * semantic.c -- does semantic analysis of the program.
 *  this involves taking the parsed sequence of tokens 
 *  and assigning variables / functions to tokens where applicable.
 *  after this step it can be passed to the compiler.
 *  this step also "flattens" the AST
 * 
 *  
 * 
 */


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

Program* process_program(Token* t){
    Program* po = malloc(sizeof(struct Program));
    int num_nativefuncs = 1;
    po->func_count = t->subtoken_count+num_nativefuncs;
    po->funcs = malloc(po->func_count*sizeof(struct Function));
    po->type_count = 4;
    po->types = malloc(4* sizeof(struct Type));
    po->types[0].width = 64;
    po->types[0].identifier = "Int";
    po->types[0].subtypes = NULL;
    po->types[0].subtype_count_per = NULL;
    po->types[0].subtype_count = 0;
    po->types[1].width = 8;
    po->types[1].identifier = "Byte";
    po->types[1].subtypes = NULL;
    po->types[1].subtype_count_per = NULL;
    po->types[1].subtype_count = 0;   
    po->types[2].width = 0; // width invalid. 
    po->types[2].identifier = "Int[]";
    po->types[2].subtypes = NULL;
    po->types[2].subtype_count_per = NULL;
    po->types[2].subtype_count = 0;
    po->types[3].width = 0; // width invalid. 
    po->types[3].identifier = "Byte[]";
    po->types[3].subtypes = NULL;
    po->types[3].subtype_count_per = NULL;
    po->types[3].subtype_count = 0;
    po->funcs[0].name = "syscall";
    po->funcs[0].write_name = "syscall";
    po->funcs[0].retval = NULL;
    po->funcs[0].params = NULL;
    po->funcs[0].vars = NULL;
    po->funcs[0].body = NULL;
    po->funcs[0].param_count = 0;
    po->funcs[0].var_count = 0;
    po->funcs[0].native = 1;
    for(int i = num_nativefuncs; i < po->func_count; i++){
        po->funcs[i].native = 0;
        process_function(&t->subtokens[i-num_nativefuncs], &po->funcs[i], po);
    }
    print_prog(po);
    return po;
}


void process_function(Token* xd, Function* func, Program* prog){
    Token def = xd->subtokens[0];
    Token body = xd->subtokens[1];
    func->name = clone(def.str);
    func->retval = proc_type(def.subtokens[0].str, prog);
    func->params = NULL;
    if(def.subtoken_count == 2 && def.subtokens[1].subtoken_count == 0){
        func->param_count = 0; // for empty varparam
    } else {
        func->param_count = (def.subtoken_count-1);
    }
    func->var_count = 0;
    func->vars = NULL;
    func->body = NULL;
    for(int i = 1; i < def.subtoken_count; i++){
        if(def.subtokens[i].str == NULL){
            continue; // empty varparam
        }
        char* ident = clone(def.subtokens[i].str);
        Type* t = proc_type(def.subtokens[i].subtokens[0].str, prog);
        Variable* var = malloc(sizeof(struct Variable));
        var->type = t;
        var->identifier = ident;
        func->params = add_varlist(func->params, var);
    }
    for(int i = 0; i < body.subtoken_count; i ++){
        process_stmt(&body.subtokens[i], func, prog);
    }
}

void* process_stmt(Token* t, Function* func, Program* prog){
    Statement* stmt = malloc(sizeof(struct Statement));
    switch(t->type){
        case T_ASSIGNMENT:
            stmt->type = S_ASSIGNMENT;
            stmt->stmt = malloc(sizeof(struct Assignment));
            Assignment* an = (Assignment*) stmt->stmt;
            an->to = proc_var(t->str, func);
            // resolve from into a var
            an->from = process_stmt(t->subtokens, func, prog);
            add_stmt_func(stmt, func);
            break;
        case T_VARINIT:
            stmt->type = S_VARINIT;
            stmt->stmt = malloc(sizeof(struct VarInit));
            VarInit* v = (VarInit*) stmt->stmt;
            v->var = mknvar(func, t->str, proc_type(t->subtokens[0].str, prog));
            add_stmt_func(stmt, func);
            break;
        case T_RETURN:
            stmt->type = S_RETURN;
            stmt->stmt = malloc(sizeof(struct Return));
            Return* ret = (Return*) stmt->stmt;
            ret->var = process_stmt(&t->subtokens[0], func, prog);
            add_stmt_func(stmt, func);
            break;
        case T_CHAR:
            stmt->type = S_CONSTANTASSIGNMENT;
            stmt->stmt = malloc(sizeof(struct ConstantAssignment));
            ConstantAssignment* ca = (ConstantAssignment*) stmt->stmt;
            ca->type = S_CONST_BYTE;
            ca->byte = t->chr;
            ca->to = mkvar(func, proc_type("Byte", prog));
            add_stmt_func(mkinit(ca->to), func);
            add_stmt_func(stmt, func);
            return ca->to;
            break;
        case T_STRING:
            stmt->type = S_CONSTANTASSIGNMENT;
            stmt->stmt = malloc(sizeof(struct ConstantAssignment));
            ConstantAssignment* ca1 = (ConstantAssignment*) stmt->stmt;
            ca1->type = S_CONST_STRING;
            ca1->string = clone(t->str);
            ca1->to = mkvar(func, proc_type("Byte[]", prog));
            add_stmt_func(mkinit(ca1->to), func);
            add_stmt_func(stmt, func);
            return ca1->to;
            break;
        case T_INT:
            stmt->type = S_CONSTANTASSIGNMENT;
            stmt->stmt = malloc(sizeof(struct ConstantAssignment));
            ConstantAssignment* ca2 = (ConstantAssignment*) stmt->stmt;
            ca2->type = S_CONST_INT;
            ca2->lnt = t->lnt;
            ca2->to = mkvar(func, proc_type("Int", prog));
            add_stmt_func(mkinit(ca2->to), func);
            add_stmt_func(stmt, func);
            return ca2->to;
            break;
        case T_FUNCALL:
            stmt->type = S_FUNCTIONCALL;
            stmt->stmt = malloc(sizeof(struct FunctionCall));
            FunctionCall* fn = (FunctionCall*) stmt->stmt;
            fn->func = proc_func(t->str, prog);
            fn->var_count = t->subtoken_count;
            fn->vars = NULL;
            for(int i = 0; i < t->subtoken_count; i++){
                Variable* new = process_stmt(&t->subtokens[i], func, prog);
                fn->vars = add_varlist(fn->vars, new);
            }
            if(!fn->func->native){
                fn->to = mkvar(func, fn->func->retval);
                add_stmt_func(mkinit(fn->to), func);
            }
            else{
                fn->to = NULL;
            }
            add_stmt_func(stmt, func);
            return fn->to;
            break;
        case T_VARIABLE:
            ; // empty statement because compiler doesnt like declaration following case
            Variable* v32 = proc_var(t->str, func);
            return v32;
            break;
        default: exit(34);
            // error
    }
    // does not resolve into variable (return, varinit, assignment)
    return NULL;
}

char* formatvar(Variable* var){
    if(var == NULL){
        return "nullvar";
    }
    if(var->identifier != NULL){
        return var->identifier;
    }
    return "unnamed";
}

void print_func(Function* func){
    printf("%s %s (", func->retval->identifier, func->name);
    VarList* p = func->params;
    while(p != NULL){
            printf("%s %s, ", p->var->type->identifier, p->var->identifier);
            p = p->next;
    }
    printf(")\n");
    Body* cur = func->body;
    while(cur != NULL){
        switch(cur->stmt->type){
            case S_ASSIGNMENT:
                ; // nessecary, see above for details
                Assignment* a = (Assignment*) cur->stmt->stmt;
                printf("    %s = %s\n", formatvar(a->to), formatvar(a->from));
                break;
                
            case S_CONSTANTASSIGNMENT:
                ;
                ConstantAssignment* ca = (ConstantAssignment*) cur->stmt->stmt;
                printf("    %s = (const)\n", formatvar(ca->to));
                break;
            case S_FUNCTIONCALL:
                ;
                FunctionCall* fc = (FunctionCall*) cur->stmt->stmt;
                printf("    %s = %s (", formatvar(fc->to), fc->func->name);
                VarList* vl = fc->vars;
                while(vl != NULL){
                    printf("%s, ", formatvar(vl->var));
                    vl = vl->next;
                }
                printf(")\n");
                break;
            case S_VARINIT:
                ;
                VarInit* vi = (VarInit*) cur->stmt->stmt;
                printf("    init %s\n", formatvar(vi->var));
                break;
            case S_RETURN:
                ;
                Return* ret = (Return*) cur->stmt->stmt;
                printf("    return %s\n", formatvar(ret->var));
                break;
        }
        cur = cur->next;
    }
}

void print_type(Type* t){
    printf("   %s, width %i\n", t->identifier, t->width);
}


void print_prog(Program* prog){
    printf("PROGRAM\n");
    printf("FUNCTIONS\n");
    for(int i = 0; i < prog->func_count; i++){
        if(!prog->funcs[i].native){
            print_func(&prog->funcs[i]);
        }
    }
    printf("TYPES\n");
    for(int i = 0; i < prog->type_count; i++){
        print_type(&prog->types[i]);
    }
}

Type* proc_type(char* ident, Program* prog){
    for(int i = 0; i < prog->type_count; i++){
        if(strcmp(prog->types[i].identifier, ident) == 0){
            return &prog->types[i];
        }
    }
    // no type found, TODO error
    exit(55);
    return NULL;
}

// make temp var
Variable* mkvar(Function* func, Type* t){
    Variable* var = malloc(sizeof(struct Variable));
    var->identifier = NULL;
    var->type = t;
    func->vars = add_varlist(func->vars, var);
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
Variable* mknvar(Function* func, char* str, Type* t){
    if(proc_var(str, func) != NULL){
        // error
        exit(27);
    }
    func->var_count += 1;
    Variable* data = malloc(sizeof (struct Variable));
    func->vars = add_varlist(func->vars, data);
    data->type = t;
    data->identifier = str;
    return data;
}

Variable* proc_var(char* str, Function* func){
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

char* clone(char* str){
    char* b = malloc(strlen(str)+1);
    memcpy(b, str, strlen(str)+1);
    return b;
}

void add_stmt_func(Statement* stmt, Function* func){
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