#include<stdlib.h>
#include<string.h>
#include<stdio.h>

#include"parser.h"
#include"semantic.h"
#include"error.h"
#include"common.h"

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


void compile_function(Token* xd, Function* func, Program* prog, State* state);
void process_function(Token* xd, Function* func, Program* prog, State* state);
void* process_stmt(Token* t, Function* func, Program* prog, State* state);
char* formatvar(Variable* var);
void print_func(Function* func);
void print_type(Type* t);
void print_prog(Program* prog);
Type* proc_type(char* ident, Program* prog);
Variable* mkvar(Function* func, Type* t);
Variable* mknvar(Function* func, char* str, Type* t);
Statement* mkinit(Variable* v);
Variable* proc_var(char* str, Function* func);
Function* proc_func(char* funcname, Program* prog);
char* clone(char* str);
void add_stmt_func(Statement* stmt, Function* func);
Type* arr_subtype(Type* arr, Program* p);
void print_type_helper(TypeStructure* ts);
void process_type(Token* t, Type* y, Program* prog, State* state);
void compile_type(Token* t, Type* y, Program* prog, State* state);
void write_structure(TypeStructure* write, Token* src, Program* prog, State* state);
int findoffset(Type* t, char* id);
int findoffsettag(Type* t, char* id);
Type* findtype(Type* t, char* id);
int bytes(TypeStructure* t);


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

Program* process_program(Token* t, State* state){
    Program* po = malloc(sizeof(struct Program));
    int num_nativefuncs = 1;
    int num_nativetypes = 2;
    po->func_count = num_nativefuncs;
    po->type_count = num_nativetypes; // (num_nativetypes)
    for(int jk = 0; jk < t->subtoken_count; jk++){
        if(t->subtokens[jk].type == T_TYPEDEF){
            po->type_count += 1;
        }
        else if(t->subtokens[jk].type == T_FUNCTION){
            po->func_count += 1;
        }
        else{
            exit(55); // impossible probably (tm)
        }
    }
    po->funcs = malloc(po->func_count*sizeof(struct Function));
    po->types = malloc(sizeof(struct TypeList));
    Type* Int = malloc(sizeof(struct Type));
    Type* Byte = malloc(sizeof(struct Type));
    Int->width = 64;
    Int->identifier = "Int";
    Int->llvm = "i64";
    Int->ts = NULL;
    Byte->width = 8;
    Byte->identifier = "Byte";
    Byte->llvm = "i8";
    Byte->ts = NULL; 
    po->types->type = Int;
    po->types->next = malloc(sizeof(struct TypeList));
    po->types->next->type = Byte;
    po->types->next->next = NULL;
    po->funcs[0].name = "syscall";
    po->funcs[0].write_name = "syscall";
    po->funcs[0].retval = Int; // i64
    po->funcs[0].params = NULL;
    po->funcs[0].vars = NULL;
    po->funcs[0].body = NULL;
    po->funcs[0].param_count = 0;
    po->funcs[0].var_count = 0;
    po->funcs[0].native = 1;
    
    // proc types first
    int c12 = 0;
    TypeList* curt = po->types->next;
    
    while(c12 < t->subtoken_count){
        while(t->subtokens[c12].type != T_TYPEDEF){
            c12 += 1;
            if(c12 >= t->subtoken_count){
                break;
            }
        }
        // double break;
        if(c12 >= t->subtoken_count)
            break;
        Type* proct = malloc(sizeof(struct Type));
        process_type(&t->subtokens[c12], proct, po, state);
        TypeList* new = malloc(sizeof(TypeList));
        new->next = NULL;
        new->type = proct;
        curt->next = new;
        c12 += 1;
        curt = new;
    }   
    c12 = 0;
    curt = po->types->next->next;
    while(c12 < t->subtoken_count){
        while(t->subtokens[c12].type != T_TYPEDEF){
            c12 += 1;
            if(c12 >= t->subtoken_count){
                break;
            }
        }
        // double break;
        if(c12 >= t->subtoken_count)
            break;
        compile_type(&t->subtokens[c12], curt->type, po, state);
        curt = curt->next;
        c12 += 1;
    }
    int c0 = 0;
    for(int i = num_nativefuncs; i < po->func_count; i++){
        po->funcs[i].native = 0;
        while(t->subtokens[c0].type != T_FUNCTION){
            c0 += 1;
        }
        process_function(&t->subtokens[c0], &po->funcs[i], po, state);
        c0 += 1;
    }
    
    
    for(int i = 0; i < po->func_count; i++){
        Function* f = &po->funcs[i];
        if(f->native){
            continue;
        }
        if(strcmp(f->name, ENTRYFUNC) == 0){
            f->write_name = "_start";
        }
        else{
            f->write_name = malloc(6+20);//20 digits = max of int
            f->max_offset = 0;
            sprintf(f->write_name, "func_%i", i);
        }
    }
    // seperate proccessing function header from body to enable calling funcs declared after.
    int jk = 0;
    for(int i = num_nativefuncs; i < po->func_count+po->type_count-4; i++){
        if(t->subtokens[i-num_nativefuncs].type == T_TYPEDEF){
            continue;
        }
        jk += 1;
        for(int j = 0; j < i; j++){
            if(t->subtokens[j].type == T_TYPEDEF){
                continue;
            }
            if(i-num_nativefuncs == j){
                continue;
            }
            if(strcmp(t->subtokens[i-num_nativefuncs].subtokens[0].str, t->subtokens[j].subtokens[0].str) == 0){
                char* msg = format("function %s redefined", t->subtokens[j].subtokens[0].str);
                add_error(state, DUPDEFFUNC, t->subtokens[i-num_nativefuncs].subtokens[0].line, msg);
            }
        }
        compile_function(&t->subtokens[i-num_nativefuncs], &po->funcs[jk], po, state);
    }
    if(state->verbose){
        print_prog(po);
    }
    return po;
}

void process_type(Token* t, Type* y, Program* prog, State* state){
    y->identifier = clone(t->str);
    y->llvm = "i64";
    y->width = 64;
}

void compile_type(Token* t, Type* y, Program* prog, State* state){
    // make sure type not redefined
    TypeList* cur = prog->types;
    while(cur != NULL){
        Type* cmp = cur->type;
        if((cmp != y) && strcmp(cmp->identifier, t->str) == 0){
            char* msg = format("type %s redefined", t->str);
            add_error(state, DUPDEFTYPE, t->line, msg);
        }
        cur = cur->next;
    }
    y->ts = malloc(sizeof(struct TypeStructure));
    y->ts->next = NULL;
    y->ts->sub = NULL;
    write_structure(y->ts, t->subtokens, prog, state);
    y->internal_width = bytes(y->ts);
}

void write_structure(TypeStructure* write, Token* src, Program* prog, State* state){
    int mode = 100000;
    switch(src->type){
        case T_ANDTYPE: mode = S_MODE_AND; break;
        case T_XORTYPE: mode = S_MODE_XOR; break;
        case T_ORTYPE: mode = S_MODE_OR; break;
        case T_SEGMENT: mode = S_MODE_TYPE; break;
    }
    write->mode = mode;
    if(write->mode == S_MODE_TYPE){
        // src = ident, so type is src->subtoken
        write->sbs = proc_type(src->subtokens->subtokens->str, prog);
        write->identifier = clone(src->subtokens->str);
        if(src->str != NULL){
            write->segment = clone(src->str);
        }
        if(write->sbs == NULL){
            char* msg = format("unknown type %s", src->subtokens->str);
            add_error(state, UNDEFTYPE, src->line, msg);
        }
        return;
    }
    TypeStructure* subcur = malloc(sizeof(struct TypeStructure));
    subcur->identifier = NULL;
    subcur->segment = NULL;
    subcur->next = NULL;
    subcur->sub = NULL;
    subcur->sbs = NULL;
    write->sub = subcur;
    TypeStructure* prev = NULL;
    for(int i = 0; i < src->subtoken_count; i++){
        subcur->sub = NULL; // for well-definedness, possibly overwritten.
        write_structure(subcur, &src->subtokens[i], prog, state);
        if(write->mode == T_ANDTYPE){
            subcur->segment = NULL;
        }
        else{
            subcur->segment = src->subtokens[i].str;
        }
        if(prev != NULL){
            prev->next = subcur;
        }
        prev = subcur;
        subcur = malloc(sizeof(struct TypeStructure));
        subcur->identifier = NULL;
        subcur->segment = NULL;
        subcur->next = NULL;
        subcur->sub = NULL;
        subcur->sbs = NULL;
    }
    subcur->next = NULL;
}


void process_function(Token* xd, Function* func, Program* prog, State* state){
    Token def = xd->subtokens[0];
    func->name = clone(def.str);
    func->retval = proc_type(def.subtokens[0].str, prog);
    if(func->retval == NULL){
        add_error(state, UNDEFTYPE, def.subtokens->line, def.subtokens[0].str);
    }
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
        // changes width here??
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
}

void compile_function(Token* t, Function* f, Program* prog, State* state){
    for(int i = 0; i < t->subtokens[1].subtoken_count; i ++){
        process_stmt(&t->subtokens[1].subtokens[i], f, prog, state);
    }
}

void* process_stmt(Token* t, Function* func, Program* prog, State* state){
    Statement* stmt = malloc(sizeof(struct Statement));
    switch(t->type){
        case T_ASSIGNMENT:
            stmt->type = S_ASSIGNMENT;
            stmt->stmt = malloc(sizeof(struct Assignment));
            Assignment* an = (Assignment*) stmt->stmt;
            an->to = proc_var(t->str, func);
            // resolve from into a var
            an->from = process_stmt(t->subtokens, func, prog, state);
            if(an->to == NULL){
                add_error(state, UNDEFVAR, t->line, format("assigning to undeclared variable %s", t->str));
            }
            else if(an->from == NULL){
                add_error(state, UNDEFVAR, t->line, "reference of undeclared variable");
            } 
            else if(an->to->type != an->from->type){
                char* to_tyname = an->to->type->identifier;
                char* from_tyname = an->from->type->identifier;
                char* msg = format("assigning %s to declared variable of type %s", from_tyname, to_tyname);
                add_error(state, INCOMPATTYPE, t->line, msg);
            }
            add_stmt_func(stmt, func);
            break;
        case T_VARINIT:
            if(proc_var(t->str, func) != NULL){
                char* msg = format("variable %s redefined", t->str);
                add_error(state, DUPDEFVAR, t->line, msg);
            }
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
            ret->var = process_stmt(&t->subtokens[0], func, prog, state);
            if(ret->var->type != func->retval){
                char* msg = format("returning %s from function of type %s", ret->var->type->identifier, func->retval->identifier);
                add_error(state, INCOMPATTYPE, t->line, msg);
            }
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
            int error = 0;
            VarList* comp = fn->func->params;
            for(int i = 0; i < t->subtoken_count; i++){
                Variable* new = process_stmt(&t->subtokens[i], func, prog, state);
                if((!fn->func->native) && comp == NULL){
                    error = 1;
                }
                if(!error && (!fn->func->native && new->type != comp->var->type)){
                    error = 1;
                }
                fn->vars = add_varlist(fn->vars, new);
                if(comp != NULL){
                    comp = comp->next;

                } else if(!fn->func->native){
                    error = 1;
                }
            }
            if(comp != NULL){
                // still more params
                error = 1;
            }
            if(error){
                VarList* comp2 = fn->func->params;
                char* defnd = format("%s", "("); // to call free on const
                while(comp2 != NULL){
                    char* o = defnd;
                    if(comp2 == fn->func->params){
                        defnd = format("%s%s", defnd, comp2->var->type->identifier);
                    }
                    else{
                        defnd = format("%s, %s", defnd, comp2->var->type->identifier);
                    }
                    free(o);
                    comp2 = comp2->next;
                }
                VarList* comp3 = fn->vars;
                char* defnd2 = format("%s", "(");
                while(comp3 != NULL){
                    char* o = defnd2;   
                    if(comp3 == fn->vars){
                        defnd2 = format("%s%s", defnd2, comp3->var->type->identifier);
                    }
                    else{
                        defnd2 = format("%s, %s", defnd2, comp3->var->type->identifier);
                    }
                    free(o);
                    comp3 = comp3->next;
                }
                char* msg = format("calling %s with %s), but definition wants %s)", fn->func->name, defnd2, defnd);
                add_error(state, INCOMPATTYPE, t->line, msg);
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
        case T_INDGET:
            stmt->type = S_INDEX;
            stmt->stmt = malloc(sizeof(struct Index));
            Index* in = (Index*) stmt->stmt;
            in->arr = process_stmt(&t->subtokens[0], func, prog, state);
            in->ind = process_stmt(&t->subtokens[1], func, prog, state);
            in->to = mkvar(func, arr_subtype(in->arr->type, prog));
            add_stmt_func(mkinit(in->to), func);
            add_stmt_func(stmt, func);
            return in->to;
            break;
        case T_INDSET:
            stmt->type = S_INDEXEQUALS;
            stmt->stmt = malloc(sizeof(struct IndexEquals));
            IndexEquals* ie = (IndexEquals*) stmt->stmt;
            ie->arr = process_stmt(&t->subtokens[0].subtokens[0], func, prog, state);
            ie->ind = process_stmt(&t->subtokens[0].subtokens[1], func, prog, state);
            ie->eq = process_stmt(&t->subtokens[1], func, prog, state);
            add_stmt_func(stmt, func);
            break;
        case T_ADDEQ:
            stmt->type = S_ADDEQUALS;
            stmt->stmt = malloc(sizeof(struct AddEquals));
            AddEquals* ae = (AddEquals*) stmt->stmt;
            ae->var = process_stmt(&t->subtokens[0], func, prog, state);
            ae->delta = process_stmt(&t->subtokens[1], func, prog, state);
            add_stmt_func(stmt, func);
            break;
        case T_CARDINALITY:
            stmt->type = S_CARDINALITY;
            stmt->stmt = malloc(sizeof(struct Cardinality));
            Cardinality* card = (Cardinality*) stmt->stmt;
            card->from = process_stmt(t->subtokens, func, prog, state);
            // TODO un hardcode type here
            card->to = mkvar(func, prog->types->type);
            add_stmt_func(mkinit(card->to), func);
            add_stmt_func(stmt, func);
            return card->to;
        case T_NEWARR:
            stmt->type = S_NEWARRAY;
            stmt->stmt = malloc(sizeof(struct NewArray));
            NewArray* new = (NewArray*) stmt->stmt;
            new->size = process_stmt(&t->subtokens[1], func, prog, state);
            new->to = mkvar(func, proc_type(t->subtokens[0].str, prog));
            add_stmt_func(mkinit(new->to), func);
            add_stmt_func(stmt, func);
            return new->to;
        case T_ARITH:
            stmt->type = S_ARITHMETIC;
            stmt->stmt = malloc(sizeof(struct Arithmetic));
            Arithmetic* arith = (Arithmetic*) stmt->stmt;
            arith->left = process_stmt(&t->subtokens[0], func, prog, state);
            arith->right = process_stmt(&t->subtokens[1], func, prog, state);
            arith->operation = t->lnt;
            // TODO another hardcoded type.
            arith->to = mkvar(func, prog->types->type); 
            add_stmt_func(mkinit(arith->to), func);
            add_stmt_func(stmt, func);
            return arith->to;
        case T_CONSTRUCTOR:
            stmt->type = S_CONSTRUCTOR;
            stmt->stmt = malloc(sizeof(struct Constructor));
            Constructor* cons = (Constructor*) stmt->stmt;
            cons->type = proc_type(t->subtokens->str, prog);
            cons->to = mkvar(func, cons->type);
            add_stmt_func(stmt, func);
            // now set each member.
            for(int i6 = 1; i6 < t->subtoken_count; i6++){
                Token* tsc = &t->subtokens[i6];
                for(int io = 0; io < tsc->subtoken_count; io++){
                    // tsc is array of segconstructs.
                    // now iterate through each constructassign
                    Token* consassign = &tsc->subtokens[io];
                    Setmember* setm = malloc(sizeof(struct Setmember));
                    setm->from = process_stmt(consassign->subtokens, func, prog, state);
                    setm->dest = cons->to;
                    setm->offsetptr = findoffset(cons->type, consassign->str);
                    Statement* stmt1 = malloc(sizeof(struct Statement));
                    stmt1->stmt = setm;
                    stmt1->type = S_SETMEMBER;
                    add_stmt_func(stmt1, func);
                }
                if(tsc->str != NULL){
                    // have to set flag ("tag") yet
                    SetTag* settag = malloc(sizeof(struct SetTag));
                    settag->dest = cons->to;
                    settag->offsetptr = findoffsettag(cons->type, tsc->str);
                    Statement* stmt2 = malloc(sizeof(struct Statement));
                    stmt2->stmt = settag;
                    stmt2->type = S_SETTAG;
                    add_stmt_func(stmt2, func);
                }
            }
            return cons->to;
        case T_ACCESSOR:
            // make own statements.
            free(stmt); 
            int ind = 0;
            int init = 0;
            // proc first expr (ex "func(args)" then .something.something etc)
            Variable* cur = process_stmt(t->subtokens, func, prog, state);
            Variable* last;
            while(1){
                if(t->str[ind] == '.' || t->str[ind] == 0){
                    // copyinclusive
                    char* ident = malloc(ind-init+1);
                    memcpy(ident, &t->str[init], ind-init);
                    ident[ind-init] = 0; 
                    // o.k now proc. 
                    Statement* sta = malloc(sizeof (struct Statement));
                    sta->type = S_ACCESSOR;
                    sta->stmt = malloc(sizeof(struct Accessor));
                    Accessor* typedacc = (Accessor*) sta->stmt;
                    typedacc->src = cur;
                    typedacc->to = mkvar(func, findtype(cur->type, ident));
                    typedacc->offsetptr = findoffset(cur->type, ident);
                    add_stmt_func(mkinit(typedacc->to), func);
                    add_stmt_func(sta, func);
                    last = typedacc->to;
                    cur = typedacc->to;
                    init = ind+1;
                }
                if(t->str[ind] == 0)
                    break;
                ind += 1;
            }
            return last;
        case T_SETMEMBER:
            stmt->type = S_SETMEMBER;
            stmt->stmt = malloc(sizeof(struct Setmember));
            Setmember* setm = (Setmember*) stmt->stmt;
            // dest = accessor
            setm->dest = process_stmt(t->subtokens, func, prog, state);
            setm->from = process_stmt(&t->subtokens[1], func, prog, state);
            setm->offsetptr = findoffset(setm->dest->type, t->subtokens[2].str);
            add_stmt_func(stmt, func);
            break;
        case T_VARIABLE:
            ; // empty statement because compiler doesnt like declaration following case
            Variable* v32 = proc_var(t->str, func);
            return v32;
            break;
        default: exit(34);
            // error
    }
    // does not resolve into variable (return, varinit, assignment, etc)
    return NULL;
}

// returns bits needed to store number
int log_2(long long input){
    int l = 0;
    int a = 1;
    // possibly make this algo faster in future.
    while(a < input){
        l += 1;
        a = a * 2;
    }
    return l;
}

// bytes needed to store a given structure.
// NOTE: includes tag. 
int bytes(TypeStructure* ts){
    if(ts->sub == NULL){
        return ts->sbs->width;
    }
    else {
        int total = 0;
        TypeStructure* cur = ts->sub;
        int numtags = 0;
        while(cur != NULL){
            numtags += 1;
            if(ts->mode == S_MODE_XOR){
                int b = bytes(cur);
                if(b > total){
                    total = b;
                }
            }
            else{
                total += bytes(cur);
            }
            cur = cur->next;
        }
        if(ts->mode == S_MODE_XOR){
            // could log2 instead.
            total += (numtags);
        }
        else if(ts->mode == S_MODE_OR){
            // need a bit for each tag.
            total += numtags;
        }
        return total;
    }
}

Type* findtypehelper(TypeStructure* ts, char* ident){
    if(ts->sub == NULL){
        if(strcmp(ident, ts->identifier) == 0){
            return ts->sbs;
        }
        return NULL;
    }
    TypeStructure* cur = ts->sub;
    while(cur != NULL){
        Type* t = findtypehelper(cur, ident);
        if(t != NULL){
            return t;
        }
        cur = cur->next;
    }
    return NULL;
}

// finds type of variable with name "ident" in type t
Type* findtype(Type* t, char* ident){
    TypeStructure* cur = t->ts;
    return findtypehelper(cur, ident);
}

int findoffsethelper(TypeStructure* ts, char* ident){
    int cachedresult = 0;
    int numtags = 0;
    int found = 0;
    if(ts->sub == NULL){
        if(strcmp(ident, ts->identifier) == 0){
            return 0;
        }
        return -1; // not found.
    }
    
    else{
        TypeStructure* cur = ts->sub;
        int sum = 0;
        while(cur != NULL){
            if(ts->mode != S_MODE_AND)
                numtags += 1;
            int this = findoffsethelper(cur, ident);
            if(this == -1){
                if((ts->mode != S_MODE_XOR) && cachedresult == 0){
                    sum += bytes(cur);
                }
            }
            else{
                cachedresult = sum + this;
                found = 1;
            }
            cur = cur->next;
        }
    }
    if(found){
        return numtags+cachedresult;
    }
    return -1;
}

int findoffsettaghelper(TypeStructure* ts, char* ident){
    if(ts->sub == NULL){
        return -1;
    }
    TypeStructure* cur = ts->sub;
    int off = 0;
    int offt = 0;
    while(cur != NULL){
        if(ts->mode != S_MODE_AND && strcmp(cur->segment, ident) == 0){
            // found in this block. 
            return off;
        }
        int ab = findoffsettaghelper(cur, ident);
        // ab = offset inside child block.
        if(ab != -1){
            // add to offset of the child block.
            // and add to offset of tag seg of currentblock
            if(ts->mode != S_MODE_AND){
                while(cur != NULL){
                    off += 1;
                    cur = cur->next;
                }
            }
            return ab + offt + off;
        }
        offt += bytes(cur);
        cur = cur->next;
        if(ts->mode != S_MODE_AND)
            off += 1;
    }
    return -1;
}

// finds offset where "ident" should be in t.
int findoffset(Type* t, char* ident){
    // TODO: -1 check.
    return findoffsethelper(t->ts, ident);
}

// same as "findoffset" but for tags.
int findoffsettag(Type* t, char* ident){
    return findoffsettaghelper(t->ts, ident);
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
            case S_INDEX:
                ;
                Index* i = (Index*) cur->stmt->stmt;
                printf("    %s = %s[%s]\n", formatvar(i->to), formatvar(i->arr), formatvar(i->ind));
                break;
            case S_INDEXEQUALS:
                ;
                IndexEquals* ie = (IndexEquals*) cur->stmt->stmt;
                printf("    %s[%s] = %s\n", formatvar(ie->arr), formatvar(ie->ind), formatvar(ie->eq));
                break;
            case S_ADDEQUALS:
                ;
                AddEquals* ae = (AddEquals*) cur->stmt->stmt;
                printf("    %s += %s\n", formatvar(ae->var), formatvar(ae->delta));
                break;
            case S_CARDINALITY:
                ;
                Cardinality* card = (Cardinality*) cur->stmt->stmt;
                printf("    %s = |%s|\n", formatvar(card->to), formatvar(card->from));
                break;
            case S_ARITHMETIC:
                ;
                Arithmetic* arith = (Arithmetic*) cur->stmt->stmt;
                printf("    %s = %s %c %s\n", formatvar(arith->to), formatvar(arith->left), sym(arith->operation), formatvar(arith->right));
                break;
            case S_CONSTRUCTOR:
                ;
                Constructor* cons = (Constructor*) cur->stmt->stmt;
                printf("    %s = new %s\n", formatvar(cons->to), cons->type->identifier);
                break;
            case S_SETMEMBER:
                ;
                Setmember* setm = (Setmember*) cur->stmt->stmt;
                printf("    %s {%i} = %s\n", formatvar(setm->dest), setm->offsetptr, formatvar(setm->from));
                break;
            case S_SETTAG:
                ;
                SetTag* sett = (SetTag*) cur->stmt->stmt;
                printf("    settag %s bit offset %i\n", formatvar(sett->dest), sett->offsetptr);
                break;
            case S_ACCESSOR:
                ;
                Accessor* acce = (Accessor*) cur->stmt->stmt;
                printf("    %s = %s {%i}\n", formatvar(acce->to), formatvar(acce->src), acce->offsetptr);
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
    if(t->ts == NULL){
        printf("   %s, native, width %i\n", t->identifier, t->width);
        return;
    }
    printf("   %s {", t->identifier);
    print_type_helper(t->ts);
    printf("}, internalwidth %i\n", t->internal_width);
}

void print_type_helper(TypeStructure* ts){
    if(ts->mode == S_MODE_TYPE){
        if(ts->sbs == NULL){
            printf("(?)");
        }
        else {
            printf("%s %s", ts->sbs->identifier, ts->identifier);
        }
        return;
    }
    TypeStructure* t = ts->sub;
    printf("(");
    while(t != NULL){
        if(ts->mode != S_MODE_AND){
            printf("%s: ", t->segment);
        }
        print_type_helper(t);
        t = t->next;
        if(t != NULL){
            switch(ts->mode){
                case S_MODE_AND: printf(" & "); break;
                case S_MODE_OR: printf(" | "); break;
                case S_MODE_XOR: printf(" ^ "); break;
            }
        }
    }
    printf(")");
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
    TypeList* cur = prog->types;
    while(cur != NULL){
        print_type(cur->type);
        cur = cur->next;
    }
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
            res->llvm = "i64";
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

// returns subtype of array (typeof array[0])
Type* arr_subtype(Type* arr, Program* p){
    // TODO remove this func
    return arr->array_subtype;
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
