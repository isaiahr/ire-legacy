#include<string.h>
#include<stdlib.h>
#include"parser.h"
#include"parseutils.h"
#include"parser_stmt.h"
#include"parser_type.h"

#define FLAG_ANDTYPE 1
// typedef = "type",  identifier, "{", subtype, "}"
Lextoken* parse_typedef(Lextoken* p, Token* e, State* state){
    // clear from possible parse_function
    if(!match(p, TYPE)){
        return NULL;
    }
    if(!match(next(p), IDENTIFIER)){
        return NULL;
    }
    if(!match(next(next(p)), LEFT_CRPAREN)){
        return NULL;
    }
    e->type = T_TYPEDEF;
    Token* child = allocate_child_token(e, p->line);
    Lextoken* a = parse_subtype(next(next(next(p))), child);
    if(a == NULL || !match(a, RIGHT_CRPAREN)){
        destroy_children(e);
        return NULL;
    }
    e->str = malloc(strlen(next(p)->str)+1);
    memcpy(e->str, next(p)->str, strlen(next(p)->str)+1);
    return next(a);
}

Lextoken* parse_subtype(Lextoken* p, Token* e){
    return parse_subtype_flags(p, e, 0);
}

// subtype = andtype | ortype | xortype | typeval
Lextoken* parse_subtype_flags(Lextoken* p, Token* e, int FLAGS){
    Lextoken* l = NULL;
    if((FLAGS & FLAG_ANDTYPE) == 0){
        l = parse_andtype_flags(p, e, FLAGS);
        if(l != NULL){
            return l;
        }
    }
    l = parse_ortype(p, e);
    if(l != NULL){
        return l;
    }
    return parse_typeval(p, e);
}

// andtype = subtype, "&", subtype, { "&", subtype} 
Lextoken* parse_andtype_flags(Lextoken* p, Token* e, int FLAGS){
    Token* child = allocate_child_token(e, p->line);
    Lextoken* l = parse_subtype_flags(p, child, FLAG_ANDTYPE);
    if(l == NULL){
        destroy_children(e);
        return NULL;
    }
    while(1){
        if(!match(l, AMPERSAND)){
            // possibly ok.
            if(subtoken_count(e) == 1){
                // bad
                destroy_children(e);
                return NULL;
            }
            // reached end.
            e->type = T_ANDTYPE;
            return l;
        }
        // continue
        child = allocate_child_token(e, p->line);
        // not for left recursion, but will generate the wrong tree
        l = parse_subtype_flags(next(l), child, FLAG_ANDTYPE);
        if(l == NULL){
            // invalid
            destroy_children(e);
            return NULL;
        }
        
    }
}

// ortype = ( "(", {identifier, ":", (subtype | void) , "|" }, ")" )
Lextoken* parse_ortype(Lextoken* p, Token* e){
    if(!match(p, LEFT_PAREN)){
        return NULL;
    }
    p = next(p);
    Token* child = allocate_child_token(e, p->line);
    while(1){
        if(subtoken_count(e) > 1){
            if(!match(p, PIPE)){
                if(match(p, RIGHT_PAREN)){
                    // good
                    e->type = T_ORTYPE;
                    destroy_youngest(e);
                    return next(p);
                } 
                // bad
                destroy_children(e);
                return NULL;
            }
            p = next(p);
        }
        if(!(match(p, IDENTIFIER) && match(next(p), COLON))){
            // bad
            destroy_children(e);
            return NULL;
        }
        char* seg = p->str;
        if(match(next(next(p)), VOID)){
            p = next(next(next(p)));
            // ok
            child->str = malloc(strlen(seg)+1);
            memcpy(child->str, seg, strlen(seg)+1);
        }
        else{
            p = parse_subtype(next(next(p)), child);
            if(p == NULL){
                // "segment" ":" (invalid)
                destroy_children(e);
                return NULL;
            }
            child->str = malloc(strlen(seg)+1);
            memcpy(child->str, seg, strlen(seg)+1);
        }
        child = allocate_child_token(e, p->line);
    }
}


// typeval = type, identifier
Lextoken* parse_typeval(Lextoken* p, Token* e){
    Token* child = allocate_child_token(e, p->line);
    Token* grandchild = allocate_child_token(child, p->line);
    Lextoken* l = parse_type(p, grandchild);
    if(match(l, IDENTIFIER)){
        e->type = T_SEGMENT;
        child->type = T_TYPEVAL;
        child->str = malloc(strlen(l->str)+1);
        memcpy(child->str, l->str, strlen(l->str)+1);
        return next(l); 
    }
    destroy_children(e);
    return NULL;
}
