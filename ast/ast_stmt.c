#define BEGIN_HANDLER switch(t->type){
#define HANDLE(x, y) case x: return y(t, func, scope, prog, state); break;
#define HANDLEV(x, y) case x: y(t, func, scope, prog, state); return NULL; break;
#define END_HANDLER default: printf("bug detected"); exit(34); }
#define SETUP_VARS(t, v, s) Statement* stmt = malloc(sizeof(struct Statement)); \
                        stmt->type = s; \
                        stmt->stmt = malloc(sizeof(struct t)); \
                        t* v = (t*) stmt->stmt;

#include<stdlib.h>
#include<string.h>
#include"ast_stmt.h"
#include"core/error.h"
#include"ast_manip.h"
#include"ast_types.h"

void p_assignment(Token* t, Function* func, Scope* scope, Program* prog, State* state);
void p_varinit(Token* t, Function* func, Scope* scope, Program* prog, State* state);
void p_return(Token* t, Function* func, Scope* scope, Program* prog, State* state);
void p_indset(Token* t, Function* func, Scope* scope, Program* prog, State* state);
void p_addeq(Token* t, Function* func, Scope* scope, Program* prog, State* state);
void p_setmember(Token* t, Function* func, Scope* scope, Program* prog, State* state);
void p_ifblk(Token* t, Function* func, Scope* scope, Program* prog, State* state);
Variable* p_boolean(Token* t, Function* func, Scope* scope, Program* prog, State* state);
Variable* p_variable(Token* t, Function* func, Scope* scope, Program* prog, State* state);
Variable* p_char(Token* t, Function* func, Scope* scope, Program* prog, State* state);
Variable* p_string(Token* t, Function* func, Scope* scope, Program* prog, State* state);
Variable* p_int(Token* t, Function* func, Scope* scope, Program* prog, State* state);
Variable* p_funcall(Token* t, Function* func, Scope* scope, Program* prog, State* state);
Variable* p_indget(Token* t, Function* func, Scope* scope, Program* prog, State* state);
Variable* p_accessor(Token* t, Function* func, Scope* scope, Program* prog, State* state);
Variable* p_cardinality(Token* t, Function* func, Scope* scope, Program* prog, State* state);
Variable* p_newarr(Token* t, Function* func, Scope* scope, Program* prog, State* state);
Variable* p_invert(Token* t, Function* func, Scope* scope, Program* prog, State* state);
Variable* p_arith(Token* t, Function* func, Scope* scope, Program* prog, State* state);
Variable* p_constructor(Token* t, Function* func, Scope* scope, Program* prog, State* state);
Variable* p_gettag(Token* t, Function* func, Scope* scope, Program* prog, State* state);
                        
// preprocessor hacks. i think it looks a bit more readable this way. 
void* process_stmt(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    BEGIN_HANDLER
    HANDLEV(T_ASSIGNMENT, p_assignment)
    HANDLEV(T_INDSET, p_indset)
    HANDLEV(T_ADDEQ, p_addeq)
    HANDLEV(T_IFBLK, p_ifblk)
    HANDLEV(T_SETMEMBER, p_setmember)
    HANDLEV(T_VARINIT, p_varinit)
    HANDLEV(T_RETURN, p_return)
    HANDLE(T_BOOLEAN, p_boolean)
    HANDLE(T_CHAR, p_char)
    HANDLE(T_STRING, p_string)
    HANDLE(T_INT, p_int)
    HANDLE(T_FUNCALL, p_funcall)
    HANDLE(T_INDGET, p_indget)
    HANDLE(T_CARDINALITY, p_cardinality)
    HANDLE(T_NEWARR, p_newarr)
    HANDLE(T_INVERT, p_invert)
    HANDLE(T_ARITH, p_arith)
    HANDLE(T_CONSTRUCTOR, p_constructor)
    HANDLE(T_ACCESSOR, p_accessor)
    HANDLE(T_VARIABLE, p_variable)
    HANDLE(T_GETTAG, p_gettag)
    END_HANDLER
}

void p_assignment(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    SETUP_VARS(Assignment, an, S_ASSIGNMENT);
    an->to = proc_var(t->str, scope, func);
    // resolve from into a var
    an->from = process_stmt(t->subtokens, func, scope, prog, state);
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
    add_stmt_func(stmt, func, scope);
}

void p_varinit(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    if(proc_var(t->str, scope, func) != NULL){
        char* msg = format("variable %s redefined", t->str);
        add_error(state, DUPDEFVAR, t->line, msg);
    }
    SETUP_VARS(VarInit, v, S_VARINIT);
    v->var = mknvar(func, scope, t->str, proc_type(t->subtokens[0].str, prog));
    add_stmt_func(stmt, func, scope);
    Assignment* an = malloc(sizeof(struct Assignment));
    Statement* stmt2 = malloc(sizeof(struct Statement));
    stmt2->stmt = an;
    stmt2->type = S_ASSIGNMENT;
    an->to = v->var;
    an->from = process_stmt(&t->subtokens[1], func, scope, prog, state);
    if(an->from == NULL){
        // probably be silent because it already errored out ???
        // keep for now to be consistent with p_assignment
        add_error(state, UNDEFVAR, t->line, "reference of undeclared variable");
        return;
    }
    if(an->to->type != an->from->type){
        char* to_tyname = an->to->type->identifier;
        char* from_tyname = an->from->type->identifier;
        char* msg = format("assigning %s to declared variable of type %s", from_tyname, to_tyname);
        add_error(state, INCOMPATTYPE, t->line, msg);
    }
    add_stmt_func(stmt2, func, scope);
}

void p_return(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    SETUP_VARS(Return, ret, S_RETURN);
    if(t->subtoken_count == 0){
        ret->var = NULL;
        if(strcmp(func->retval->identifier, "void") != 0){
            char* msg = format("void return from function of type %s", func->retval->identifier);
            add_error(state, INCOMPATTYPE, t->line, msg);
        }
    }
    else{
        ret->var = process_stmt(&t->subtokens[0], func, scope, prog, state);
        if(ret->var == NULL){
            return;
        }
        if(ret->var->type != func->retval){
            char* msg = format("returning %s from function of type %s", ret->var->type->identifier, func->retval->identifier);
            add_error(state, INCOMPATTYPE, t->line, msg);
        }
    }
    add_stmt_func(stmt, func, scope);
}

Variable* p_boolean(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    SETUP_VARS(ConstantAssignment, ca, S_CONSTANTASSIGNMENT);
    ca->type = S_CONST_BOOLEAN;
    ca->lnt = t->lnt;
    ca->to = mkvar(func, scope, proc_type("Boolean", prog));
    add_stmt_func(mkinit(ca->to), func, scope);
    add_stmt_func(stmt, func, scope);
    return ca->to;
}

Variable* p_char(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    SETUP_VARS(ConstantAssignment, ca, S_CONSTANTASSIGNMENT);
    ca->type = S_CONST_BYTE;
    ca->byte = t->chr;
    ca->to = mkvar(func, scope, proc_type("Byte", prog));
    add_stmt_func(mkinit(ca->to), func, scope);
    add_stmt_func(stmt, func, scope);
    return ca->to;
}

Variable* p_string(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    SETUP_VARS(ConstantAssignment, ca, S_CONSTANTASSIGNMENT);
    ca->type = S_CONST_STRING;
    ca->string = clone(t->str);
    ca->to = mkvar(func, scope, proc_type("Byte[]", prog));
    add_stmt_func(mkinit(ca->to), func, scope);
    add_stmt_func(stmt, func, scope);
    return ca->to;
}

Variable* p_int(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    SETUP_VARS(ConstantAssignment, ca, S_CONSTANTASSIGNMENT);
    ca->type = S_CONST_INT;
    ca->lnt = t->lnt;
    ca->to = mkvar(func, scope, proc_type("Int", prog));
    add_stmt_func(mkinit(ca->to), func, scope);
    add_stmt_func(stmt, func, scope);
    return ca->to;
}

Variable* p_funcall(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    SETUP_VARS(FunctionCall, fn, S_FUNCTIONCALL);
    fn->func = proc_func(t->str, prog);
    fn->vars = NULL;
    int error = 0;
    if(fn->func == NULL){
        char* msg = format("function %s undefined", t->str);
        add_error(state, UNDEFFUNC, t->line, msg);
        fn->to = NULL;
        return fn->to;
    }
    VarList* comp = fn->func->params;
    for(int i = 0; i < t->subtoken_count; i++){
        Variable* new = process_stmt(&t->subtokens[i], func, scope, prog, state);
        if(!new){
            error = 1;
        }
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
            if(comp3->var == NULL){
                // possible.
                free(o);
                return fn->to;
            }
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
    if(!fn->func->native && fn->func->retval == proc_type("void", prog)){
        fn->to = voidval(func, scope, fn->func->retval);
    }
    else if(!fn->func->native){
        fn->to = mkvar(func, scope, fn->func->retval);
        add_stmt_func(mkinit(fn->to), func, scope);
    }
    else{
        fn->to = NULL;
    }
    add_stmt_func(stmt, func, scope);
    return fn->to;
}

Variable* p_indget(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    SETUP_VARS(Index, in, S_INDEX);
    in->arr = process_stmt(&t->subtokens[0], func, scope, prog, state);
    in->ind = process_stmt(&t->subtokens[1], func, scope, prog, state);
    in->to = mkvar(func, scope, arr_subtype(in->arr->type, prog));
    add_stmt_func(mkinit(in->to), func, scope);
    add_stmt_func(stmt, func, scope);
    return in->to;
}

void p_indset(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    SETUP_VARS(IndexEquals, ie, S_INDEXEQUALS);
    ie->arr = process_stmt(&t->subtokens[0].subtokens[0], func, scope, prog, state);
    ie->ind = process_stmt(&t->subtokens[0].subtokens[1], func, scope, prog, state);
    ie->eq = process_stmt(&t->subtokens[1], func, scope, prog, state);
    add_stmt_func(stmt, func, scope);
}

void p_addeq(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    SETUP_VARS(AddEquals, ae, S_ADDEQUALS); 
    ae->var = process_stmt(&t->subtokens[0], func, scope, prog, state);
    ae->delta = process_stmt(&t->subtokens[1], func, scope, prog, state);
    add_stmt_func(stmt, func, scope);
}

Variable* p_cardinality(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    SETUP_VARS(Cardinality, card, S_CARDINALITY);
    card->from = process_stmt(t->subtokens, func, scope, prog, state);
    // TODO un hardcode type here
    card->to = mkvar(func, scope, prog->types->type);
    add_stmt_func(mkinit(card->to), func, scope);
    add_stmt_func(stmt, func, scope);
    return card->to;
}

Variable* p_newarr(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    SETUP_VARS(NewArray, new, S_NEWARRAY);
    new->size = process_stmt(&t->subtokens[1], func, scope, prog, state);
    new->to = mkvar(func, scope, proc_type(t->subtokens[0].str, prog));
    add_stmt_func(mkinit(new->to), func, scope);
    add_stmt_func(stmt, func, scope);
    return new->to;
}

Variable* p_invert(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    SETUP_VARS(Invert, inv, S_INVERT);
    inv->from = process_stmt(&t->subtokens[0], func, scope, prog, state);
    inv->to = mkvar(func, scope, proc_type("Boolean", prog));
    if(inv->from == NULL){
        return NULL;
    }
    if(inv->from->type == NULL || inv->from->type != inv->to->type){
        // possibly segv? type == null shouldnt happen though i think. (tm)
        char* msg = format("Operation ! wants boolean, but got %s", inv->from->type->identifier);
        add_error(state, INCOMPATTYPE, t->line, msg);
    }
    add_stmt_func(mkinit(inv->to), func, scope);
    add_stmt_func(stmt, func, scope);
    return inv->to;
}

Variable* p_arith(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    SETUP_VARS(Arithmetic, arith, S_ARITHMETIC);
    arith->left = process_stmt(&t->subtokens[0], func, scope, prog, state);
    arith->right = process_stmt(&t->subtokens[1], func, scope, prog, state);
    arith->operation = t->lnt;
    if(arith->left == NULL || arith->right == NULL){
        return NULL;
    }
    int err = 0;
    char* op;
    Type* t_int = proc_type("Int", prog);
    Type* t_bool = proc_type("Boolean", prog);
    switch (arith->operation){
        case PLUS:
            arith->to = mkvar(func, scope, t_int);
            op = "+";
            err = verify_types(t_int, arith->left, arith->right, NULL);
            break;
        case SUBTRACT:
            arith->to = mkvar(func, scope, t_int);
            op = "-";
            err = verify_types(t_int, arith->left, arith->right, NULL);
            break;
        case MULT:
            arith->to = mkvar(func, scope, t_int);
            op = "*";
            err = verify_types(t_int, arith->left, arith->right, NULL);
            break;
        case DOUBLEEQUALS:
            arith->to = mkvar(func, scope, t_bool);
            op = "==";
            if(arith->left == NULL || arith->right == NULL)
                err = 1;
            else if(arith->left->type == NULL)
                err = 1;
            else if(arith->left->type != arith->right->type){
                err = 1;
            }
            break;
        case GREATER:
            arith->to = mkvar(func, scope, t_bool);
            op = ">";
            err = verify_types(t_int, arith->left, arith->right, NULL);
            break;
        case LESS:
            arith->to = mkvar(func, scope, t_bool);
            op = "<";
            err = verify_types(t_int, arith->left, arith->right, NULL);
            break;
        case AMPERSAND:
            // logical and
            arith->to = mkvar(func, scope, t_bool);
            op = "&";
            err = verify_types(t_bool, arith->left, arith->right, NULL);
            break;
        case PIPE:
            // logical or
            arith->to = mkvar(func, scope, t_bool);
            op = "|";
            err = verify_types(t_bool, arith->left, arith->right, NULL);
            break;
        case PERCENT:
            arith->to = mkvar(func, scope, t_int);
            op = "%";
            err = verify_types(t_int, arith->left, arith->right, NULL);
            break;
        case FSLASH:
            arith->to = mkvar(func, scope, t_int);
            op = "/";
            err = verify_types(t_int, arith->left, arith->right, NULL);
            break;
    }
    if(err){
        char* msg = format("in operation %s", op);
        add_error(state, INCOMPATTYPE, t->line, msg);
    }
    // metadata update
    else if((arith->left->meta != NULL && arith->left->meta->tags != NULL) || (arith->right->meta != NULL && arith->right->meta->tags != NULL)){
        // NOTE: right now, the presecence of a tag is taken as valid, so the "valid" boolean does nothing.
        // this is because there is no way to go from invalid -> valid, and it simplifies things a bit if we
        // dont malloc invalid states.
        if(arith->operation == AMPERSAND){
            arith->to->meta = malloc(sizeof(struct VarMetadata));
            arith->to->meta->tags = malloc(sizeof(struct TagList));
            TagList* cur = arith->to->meta->tags;
            if(arith->left->meta != NULL && arith->left->meta->tags != NULL){
                TagList* append = arith->left->meta->tags;
                while(append != NULL){
                    cur->tag = clone(append->tag);
                    cur->var = append->var;
                    cur->valid = 1;
                    cur->next = NULL;
                    if(append->next != NULL){
                        cur->next = malloc(sizeof(struct TagList));
                        cur = cur->next;    
                    }
                    append = append->next;
                }
                if(arith->right->meta != NULL && arith->right->meta->tags != NULL){
                    // setup cur for more
                    cur->next = malloc(sizeof(struct TagList));
                    cur = cur->next;
                }
            }
            if(arith->right->meta != NULL && arith->right->meta->tags != NULL){
                TagList* append = arith->right->meta->tags;
                while(append != NULL){
                    cur->tag = clone(append->tag);
                    cur->var = append->var;
                    cur->valid = 1;
                    cur->next = NULL;
                    if(append->next != NULL){
                        cur->next = malloc(sizeof(struct TagList));
                        cur = cur->next;    
                    }
                    append = append->next;
                }
            }
        }
    }
    add_stmt_func(mkinit(arith->to), func, scope);
    add_stmt_func(stmt, func, scope);
    return arith->to;
}

Variable* p_constructor(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    SETUP_VARS(Constructor, cons, S_CONSTRUCTOR);
    cons->type = proc_type(t->subtokens->str, prog);
    cons->to = mkvar(func, scope, cons->type);
    add_stmt_func(mkinit(cons->to), func, scope);
    add_stmt_func(stmt, func, scope);
    // now set each member.
    for(int i6 = 1; i6 < t->subtoken_count; i6++){
        Token* tsc = &t->subtokens[i6];
        for(int io = 0; io < tsc->subtoken_count; io++){
            // tsc is array of segconstructs.
            // now iterate through each constructassign
            Token* consassign = &tsc->subtokens[io];
            Setmember* setm = malloc(sizeof(struct Setmember));
            setm->from = process_stmt(consassign->subtokens, func, scope, prog, state);
            setm->dest = cons->to;
            setm->offsetptr = findoffset(cons->type, consassign->str);
            Statement* stmt1 = malloc(sizeof(struct Statement));
            stmt1->stmt = setm;
            stmt1->type = S_SETMEMBER;
            add_stmt_func(stmt1, func, scope);
        }
        if(tsc->str != NULL){
            // have to set flag ("tag") yet
            SetTag* settag = malloc(sizeof(struct SetTag));
            settag->dest = cons->to;
            settag->offsetptr = findoffsettag(cons->type, tsc->str);
            Statement* stmt2 = malloc(sizeof(struct Statement));
            stmt2->stmt = settag;
            stmt2->type = S_SETTAG;
            add_stmt_func(stmt2, func, scope);
        }
    }
    return cons->to;
}

Variable* p_accessor(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    int ind = 0;
    int init = 0;
    // proc first expr (ex "func(args)" then .something.something etc)
    Variable* cur = process_stmt(t->subtokens, func, scope, prog, state);
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
            Type* ty01 = findtype(cur->type, ident);
            if(ty01 == NULL){
                char* msg = format("member %s of type %s not found", ident, cur->type->identifier);
                add_error(state, MEMBERNOTFOUND, t->line, msg);
                return NULL;
            }
            typedacc->to = mkvar(func, scope, ty01);
            typedacc->offsetptr = findoffset(cur->type, ident);
            if(!check_valid_access(cur, ident, scope)){
                char* msg = format("unsafe access of member %s of type %s", ident, cur->type->identifier);
                add_error(state, FRAUDULENTACCESS, t->line, msg);
                return NULL;
            }
            add_stmt_func(mkinit(typedacc->to), func, scope);
            add_stmt_func(sta, func, scope);
            last = typedacc->to;
            cur = typedacc->to;
            init = ind+1;
        }
        if(t->str[ind] == 0)
            break;
        ind += 1;
    }
    return last;
}

Variable* p_gettag(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    SETUP_VARS(GetTag, gett, S_GETTAG);
    gett->src = process_stmt(t->subtokens, func, scope, prog, state);
    if(gett->src == NULL){
        return NULL;
    }
    gett->offsetptr = findoffsettag(gett->src->type, t->str);
    if(gett->offsetptr == -1){
        char* msg = format("tag %s of type %s not found", t->str, gett->src->type->identifier);
        add_error(state, TAGNOTFOUND, t->line, msg);
        return NULL;
    }
    gett->dest = mkvar(func, scope, proc_type("Boolean", prog));
    // attach meta to gett->dest
    gett->dest->meta = malloc(sizeof(struct VarMetadata));
    gett->dest->meta->tags = malloc(sizeof(struct TagList));
    gett->dest->meta->tags->tag = clone(t->str);
    gett->dest->meta->tags->var = gett->src;
    gett->dest->meta->tags->valid = 1;
    gett->dest->meta->tags->next = NULL;
    add_stmt_func(mkinit(gett->dest), func, scope);
    add_stmt_func(stmt, func, scope);
    return gett->dest;
}

void p_setmember(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    SETUP_VARS(Setmember, setm, S_SETMEMBER);
    setm->dest = process_stmt(t->subtokens, func, scope, prog, state);
    setm->from = process_stmt(&t->subtokens[1], func, scope, prog, state);
    setm->offsetptr = findoffset(setm->dest->type, t->subtokens[2].str);
    if(setm->offsetptr == -1){
        char* msg = format("member %s of type %s not found", t->subtokens[2].str, setm->dest->type->identifier);
        add_error(state, MEMBERNOTFOUND, t->line, msg);
        return;
    }
    if(!check_valid_access(setm->dest, t->subtokens[2].str, scope)){
        char* msg = format("unsafe setting member %s of type %s", t->subtokens[2].str, setm->dest->type->identifier);
        add_error(state, FRAUDULENTACCESS, t->line, msg);
        return;
    }
    Type* destty = findtype(setm->dest->type, t->subtokens[2].str);
    if(destty != setm->from->type){
        char* msg = format("incompatible types in member set %s = %s", destty->identifier, setm->from->type->identifier);
        add_error(state, INCOMPATTYPE, t->line, msg);
    }
    else {
        add_stmt_func(stmt, func, scope);
    }
}

Variable* p_variable(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    return proc_var(t->str, scope, func);
}

char* gen_lbl(State* state){
    char* result = malloc(33); // VERY conservative upper bound
    snprintf(result, 32, "L%i", state->lblcount);
    state->lblcount += 1;
    return result;
}

void p_ifblk(Token* t, Function* func, Scope* scope, Program* prog, State* state){
    Statement* stmt = malloc(sizeof(struct Statement));
    stmt->type = S_IF;
    IfStmt* prev = NULL;
    int check_meta = 0;
    for(int i = 0; i < t->subtoken_count; i++){
        Token* if0 = &t->subtokens[i];
        IfStmt* ifs = malloc(sizeof(struct IfStmt)); 
        if(i == 0){
            stmt->stmt = ifs;
        }
        if(if0->type != T_ELSE){
            ifs->cond = malloc(sizeof(struct Scope));
            ifs->cond->parent = scope;
            ifs->cond->body = NULL;
            ifs->cond->offset = 0;
            ifs->cond->vars = NULL;
            ifs->test = process_stmt(if0->subtokens, func, ifs->cond, prog, state);
            if(ifs->test == NULL){
                return;// error on expr, should be outputted by other func.
            }
            if(ifs->test->type == NULL || ifs->test->type != proc_type("Boolean", prog)){
                char* msg = format("%s", "ifstmt needs Boolean");
                add_error(state, INCOMPATTYPE, if0->subtokens->line, msg);
            }
            else{
                check_meta = 1;
            }
            ifs->initlbl = gen_lbl(state);
        }
        ifs->truelbl = gen_lbl(state);
        ifs->endlbl = NULL;
        if(i+1 == t->subtoken_count){
            // else.
            ifs->endlbl = gen_lbl(state);
            ifs->elsestmt = NULL;
        }
        if(prev != NULL){
            prev->elsestmt = ifs;
        }
        Scope* newscope = malloc(sizeof(struct Scope));
        newscope->parent = scope;
        newscope->body = NULL;
        newscope->offset = 0;
        newscope->vars = NULL;
        if(check_meta && ifs->test->meta != NULL && ifs->test->meta->tags != NULL){
            // attach meta to scope
            newscope->meta = malloc(sizeof(struct ScopeMetadata));
            // NOTE: possibly clone here, ptr alias might cause problems with gc?
            newscope->meta->tags = ifs->test->meta->tags;
        }
        check_meta = 0;
        Token* body = NULL;
        if(if0->type == T_ELSE){
            body = &if0->subtokens[0];
        }
        else{
            body = &if0->subtokens[1];
        }
        for(int j = 0; j < body->subtoken_count; j++){
            process_stmt(&body->subtokens[j], func, newscope, prog, state);
        }
        ifs->scope = newscope;
        prev = ifs;
    }
    add_stmt_func(stmt, func, scope);
}
