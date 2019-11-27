#include<string.h>
#include<stdlib.h>
#include"parser.h"
#include"parseutils.h"
#include"parser_stmt.h"
#include"parser_expr.h"


// note: not a stmt on its own, but used in other stmts.
// type = (identifier, [ "[", "]"]) | void
Lextoken* parse_type(Lextoken* p, Token* e){
    int i = match(p, IDENTIFIER);
    if(!i){
        return NULL;
    }
    e->type = T_TYPE;
    e->str = malloc(strlen(p->str)+3);
    memcpy(e->str, p->str, strlen(p->str)+1);
    int j = match(next(p), LEFT_SQPAREN);
    j = j && match(next(next(p)), RIGHT_SQPAREN);
    if(j){
        e->str[strlen(p->str)] = '[';
        e->str[strlen(p->str)+1] = ']';
        e->str[strlen(p->str)+2] = 0;
        return next(next(next(p)));
    }
    e->lnt = 0;
    return next(p);
}

// like parse_type, but allows void also.
// typically we do not allow void, so these are different funcs
// to simplify ast type checking.
Lextoken* parse_type_void(Lextoken* p, Token* e){
    if(match(p, VOID)){
        e->type = T_TYPE;
        e->str = format("%s", "void");
        return next(p);
    }
    return parse_type(p, e);
}

// varinit = type, identifier, "=", expression
Lextoken* parse_varinit(Lextoken* p, Token* e){
    Token* child1 = allocate_child_token(e, p->line);
    Token* child2 = allocate_child_token(e, p->line);
    Lextoken* l = parse_type(p, child1);
    if(match(l, IDENTIFIER) && match(next(l), EQUALS)){
        Lextoken* l2 = parse_expression(next(next(l)), child2);
        if(l2 == NULL){
            destroy_children(e);
            return NULL;
        }
        e->str = malloc(strlen(l->str)+1);
        memcpy(e->str, l->str, strlen(l->str)+1);
        e->type = T_VARINIT;
        return l2;
    }
    destroy_children(e);
    return NULL;
}


// assignment = identifier, "=", expression
Lextoken* parse_assignment(Lextoken* p, Token* e){
    int i = match(p, IDENTIFIER);
    i = i && match(next(p), EQUALS);
    Token* child = allocate_child_token(e, p->line);
    Lextoken* l = parse_expression(next(next(p)), child);
    if(i && l != NULL){
        e->type = T_ASSIGNMENT;
        e->str = malloc(strlen(p->str)+1);
        memcpy(e->str, p->str, strlen(p->str)+1);
        return l;
    }
    destroy_children(e);
    return NULL;
}

// arrset = arrind, "=", expression
Lextoken* parse_arrset(Lextoken* p, Token* e){
    Token* child1 = allocate_child_token(e, p->line);
    Token* child2 = allocate_child_token(e, p->line);
    Lextoken* a = parse_arrind(p, child1);
    if(match(a, EQUALS)){
        Lextoken* p = parse_expression(next(a), child2);
        if(p != NULL){
            e->type = T_INDSET;
            return p;
        }
    }
    destroy_children(e);
    return NULL;
}

// addeq = expression, "+=", expression
Lextoken* parse_addeq(Lextoken* p, Token* e){
    Token* child1 = allocate_child_token(e, p->line);
    Token* child2 = allocate_child_token(e, p->line);
    Lextoken* a = parse_expression(p, child1);
    if(match(a, ADDEQ)){
        Lextoken* p = parse_expression(next(a), child2);
        if(p != NULL){
            e->type = T_ADDEQ;
            return p;
        }
    }
    destroy_children(e);
    return NULL;
}

// return = return, [expression]
Lextoken* parse_return(Lextoken* p, Token* e){
    if(!match(p, RETURN)){
        return NULL;
    }
    Token* child = allocate_child_token(e, p->line);
    Lextoken* k = next(p);
    Lextoken* a = parse_expression(k, child);
    if(a == NULL){
        // empty return; valid in void functions.
        destroy_children(e);
        e->type = T_RETURN;
        return k;
    }
    e->type = T_RETURN;
    return a;
}


// setmember = accessor = expr
Lextoken* parse_setmember(Lextoken* p, Token* e){
    Token* child = allocate_child_token(e, p->line);
    p = parse_accessor(p, child);
    if(p == NULL || (!match(p, EQUALS))){
        destroy_children(e);
        return NULL;
    }
    Token* child2 = allocate_child_token(e, p->line);
    p = parse_expression(next(p), child2);
    if(p == NULL){
        destroy_children(e);
        return NULL;
    }
    Token* child3 = allocate_child_token(e, p->line);
    
    // weird hack to split apart the accessor, to make it easy for semantic to proccess.
    int ind = 0;
    int lastdot = -1;
    while(child->str[ind] != 0){
        if(child->str[ind] == '.')
            lastdot = ind;
        ind += 1;
    }
    if(lastdot == -1){
        // corner case: one level of nesting. ("a.b")
        // "lift" the accessor subtoken onto setmember subtoken, to remove accessor.
        child3->type = T_MEMBERIDENT;
        child3->str = child->str;
        e->type = T_SETMEMBER;
        
        e->subtokens = child->subtokens;
        // TODO this probably won't work?
        // set child3 since its only supposed to overwrite first 2
        //memcpy(e->subtokens, e->subtokens[0].subtokens, sizeof(struct Token));
        return p;
    }
    child->str[lastdot] = 0;
    child3->type = T_MEMBERIDENT;
    child3->str = &child->str[lastdot+1];
    e->type = T_SETMEMBER;
    return p;
}

// if = if, expr, "{", body, "}"
Lextoken* parse_if(Lextoken* p, Token* e, State* state){
    if(!match(p, IF)){
        return NULL;
    }
    Token* child1 = allocate_child_token(e, p->line);
    Lextoken* a = parse_expression(next(p), child1);
    if(a == NULL || !match(a, LEFT_CRPAREN)){
        destroy_children(e);
        return NULL;
    }
    Token* child2 = allocate_child_token(e, p->line);
    Lextoken* b = parse_body(next(a), child2, state);
    if(!match(b, RIGHT_CRPAREN)){
        destroy_children(e);
        return NULL;
    }
    e->type = T_IF;
    return next(b);
}

// elseif = else, if, expr, "{", body, "}"
Lextoken* parse_elseif(Lextoken* p, Token* e, State* state){
    if(!match(p, ELSE) || !match(next(p), IF)){
        return NULL;
    }
    p = next(p);
    Token* child1 = allocate_child_token(e, p->line);
    Lextoken* a = parse_expression(next(p), child1);
    if(a == NULL || !match(a, LEFT_CRPAREN)){
        destroy_children(e);
        return NULL;
    }
    Token* child2 = allocate_child_token(e, p->line);
    Lextoken* b = parse_body(next(a), child2, state);
    if(!match(b, RIGHT_CRPAREN)){
        destroy_children(e);
        return NULL;
    }
    e->type = T_ELSEIF;
    return next(b);
}

// else = else, "{", body, "}"
Lextoken* parse_else(Lextoken* p, Token* e, State* state){
    if(!match(p, ELSE)){
        return NULL;
    }
    if(!match(next(p), LEFT_CRPAREN)){
        return NULL;
    }
    Token* child = allocate_child_token(e, p->line);
    Lextoken* b = parse_body(next(next(p)), child, state);
    if(!match(b, RIGHT_CRPAREN)){
        destroy_children(e);
        return NULL;
    }
    e->type = T_ELSE;
    return next(b);
}

// ifblk = if, { elseif, }, [else]
Lextoken* parse_ifblk(Lextoken* p, Token* t, State* state){
    Token* child1 = allocate_child_token(t, p->line);
    Lextoken* l = parse_if(p, child1, state);
    Lextoken* o_l = l;
    if(l == NULL){
        destroy_children(t);
        return NULL;
    }
    while(match(l, TERM)){
        l = l->next;
    }
    Token* child2 = allocate_child_token(t, p->line);
    Lextoken* l1 = parse_elseif(l, child2, state);
    // old l1
    Lextoken* o_l1 = l1;
    while(match(l1, TERM)){
        l1 = l1->next;
    }
    int i = 2;
    // "old old l1"
    Lextoken* o_o_l1;
    while(l1 != NULL){
        l = l1;
        child2 = allocate_child_token(t, l1->line);
        o_o_l1 = o_l1;
        l1 = parse_elseif(l, child2, state);
        o_l1 = l1;
        while(match(l1, TERM)){
            l1 = l1->next;
        }
        
        i = i + 1;
    }
    Lextoken* l2 = parse_else(l, child2, state);
    t->type = T_IFBLK;
    if(l2 == NULL){
        destroy_youngest(t);
        if(i == 2)// if
            return o_l;
        // elseif.
        return o_o_l1;
    }
    else {
        return l2;
    }
}

// statement = varinit | expression | assignment | return | arrset | setmember
Lextoken* parse_statement(Lextoken* p, Token* t, State* state){
    Lextoken* l = parse_varinit(p, t);
    if(l != NULL){
        return l;
    }
    l = parse_assignment(p, t);
    if(l != NULL){
        return l;
    }
    l = parse_return(p, t);
    if(l != NULL){
        return l;
    }
    l = parse_addeq(p, t);
    if(l != NULL){
        return l;
    }
    l = parse_arrset(p, t);
    if(l != NULL){
        return l;
    }
    l = parse_setmember(p, t);
    if(l != NULL){
        return l;
    }
    l = parse_ifblk(p, t, state);
    if(l != NULL){
        return l;
    }
    return parse_expression(p, t);
}
