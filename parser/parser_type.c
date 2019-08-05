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
    e->subtoken_count = 0;
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
    e->subtokens = init_token(p->line);
    Lextoken* a = parse_subtype(next(next(next(p))), e->subtokens);
    if(a == NULL || !match(a, RIGHT_CRPAREN)){
        destroy_token(e);
        e->subtokens = NULL;
        return NULL;
    }
    e->subtoken_count = 1;
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
    l = parse_xortype(p, e);
    if(l != NULL){
        return l;
    }
    return parse_typeval(p, e);
}

// andtype = subtype, "&", subtype, { "&", subtype} 
Lextoken* parse_andtype_flags(Lextoken* p, Token* e, int FLAGS){
    e->subtokens = init_token(p->line);
    Lextoken* l = parse_subtype_flags(p, e->subtokens, FLAG_ANDTYPE);
    if(l == NULL){
        destroy_token(e->subtokens);
        e->subtokens = NULL;
        return NULL;
    }
    e->subtoken_count = 1;
    while(1){
        if(!match(l, AMPERSAND)){
            // possibly ok.
            if(e->subtoken_count == 1){
                // bad
                e->subtokens = NULL;
                e->subtoken_count = 0;
                return NULL;
            }
            // reached end.
            e->type = T_ANDTYPE;
            return l;
        }
        // continue
        e->subtokens = realloc_token(e->subtokens, e->subtoken_count+1);
        e->subtoken_count += 1;
        // not for left recursion, but will generate the wrong tree
        l = parse_subtype_flags(next(l), &e->subtokens[e->subtoken_count-1], FLAG_ANDTYPE);
        if(l == NULL){
            // invalid
            destroy_token(e->subtokens);
            e->subtokens = NULL;
            e->subtoken_count = 0;
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
    e->subtokens = init_token(p->line);
    e->subtoken_count = 1;
    while(1){
        if(e->subtoken_count > 1){
            if(!match(p, PIPE)){
                if(match(p, RIGHT_PAREN)){
                    // good
                    e->type = T_ORTYPE;
                    e->subtoken_count = e->subtoken_count-1;
                    return next(p);
                } 
                // bad
                e->subtoken_count = 0;
                destroy_token(e->subtokens);
                e->subtokens = NULL;
                return NULL;
            }
            p = next(p);
        }
        if(!(match(p, IDENTIFIER) && match(next(p), COLON))){
            // bad
            e->subtoken_count = 0;
            destroy_token(e->subtokens);
            e->subtokens = NULL;
            return NULL;
        }
        char* seg = p->str;
        if(match(next(next(p)), VOID)){
            p = next(next(next(p)));
            // ok
            e->subtokens[e->subtoken_count-1].str = malloc(strlen(seg)+1);
            memcpy(e->subtokens[e->subtoken_count-1].str, seg, strlen(seg)+1);
            e->subtoken_count += 1;
            e->subtokens = realloc_token(e->subtokens, e->subtoken_count);
        }
        else{
            p = parse_subtype(next(next(p)), &e->subtokens[e->subtoken_count-1]);
            if(p == NULL){
                // "segment" ":" (invalid)
                e->subtoken_count = 0;
                destroy_token(e->subtokens);
                e->subtokens = NULL;
                return NULL;
            }
            e->subtokens[e->subtoken_count-1].str = malloc(strlen(seg)+1);
            memcpy(e->subtokens[e->subtoken_count-1].str, seg, strlen(seg)+1);
            e->subtoken_count += 1;
            e->subtokens = realloc_token(e->subtokens, e->subtoken_count);
        }
    }
}

// same as ortype.
// TODO combine these 2 funcs ?
// xortype = ( "(", {identifier, ":", (subtype | void) , "^" }, ")" )
Lextoken* parse_xortype(Lextoken* p, Token* e){
    if(!match(p, LEFT_PAREN)){
        return NULL;
    }
    p = next(p);
    e->subtokens = init_token(p->line);
    e->subtoken_count = 1;
    while(1){
        // subtoken_count actually less
        if(e->subtoken_count > 1){
            if(!match(p, CARET)){
                if(match(p, RIGHT_PAREN)){
                    // good
                    e->type = T_XORTYPE;
                    e->subtoken_count = e->subtoken_count-1;
                    return next(p);
                } 
                // bad
                e->subtoken_count = 0;
                destroy_token(e->subtokens);
                e->subtokens = NULL;
                return NULL;
            }
            p = next(p);
        }
        if(!(match(p, IDENTIFIER) && match(next(p), COLON))){
            // bad
            e->subtoken_count = 0;
            destroy_token(e->subtokens);
            e->subtokens = NULL;
            return NULL;
        }
        char* seg = p->str;
        if(match(next(next(p)), VOID)){
            p = next(next(next(p)));
            // ok
            e->subtokens[e->subtoken_count-1].str = malloc(strlen(seg)+1);
            memcpy(e->subtokens[e->subtoken_count-1].str, seg, strlen(seg)+1);
            e->subtoken_count += 1;
            e->subtokens = realloc_token(e->subtokens, e->subtoken_count);
        }
        else{
            p = parse_subtype(next(next(p)), &e->subtokens[e->subtoken_count-1]);
            if(p == NULL){
                // "segment" ":" (invalid)
                e->subtoken_count = 0;
                destroy_token(e->subtokens);
                e->subtokens = NULL;
                return NULL;
            }
            e->subtokens[e->subtoken_count-1].str = malloc(strlen(seg)+1);
            memcpy(e->subtokens[e->subtoken_count-1].str, seg, strlen(seg)+1);
            e->subtoken_count += 1;
            e->subtokens = realloc_token(e->subtokens, e->subtoken_count);
        }
    }
}

// typeval = type, identifier
Lextoken* parse_typeval(Lextoken* p, Token* e){
    e->subtokens = init_token(p->line);
    e->subtokens->subtokens = init_token(p->line);
    Lextoken* l = parse_type(p, &e->subtokens->subtokens[0]);
    if(match(l, IDENTIFIER)){
        e->type = T_SEGMENT;
        e->subtokens->type = T_TYPEVAL;
        e->subtokens->str = malloc(strlen(l->str)+1);
        e->subtokens->subtoken_count = 1;
        e->subtoken_count = 1;
        memcpy(e->subtokens->str, l->str, strlen(l->str)+1);
        return next(l); 
    }
    e->subtokens->subtoken_count = 0;
    e->subtokens->subtokens = NULL;
    destroy_token(e->subtokens);
    destroy_token(e);
    e->subtoken_count = 0;
    e->subtokens = NULL;
    return NULL;
}
