/**
 * ast_types.c - funcs for manipulating the type ast.
 * this is responsible for processing defined types
 * 
 */

#include<stdlib.h>
#include<string.h>
#include"common/common.h"
#include"common/error.h"
#include"ast_manip.h"
#include"ast_types.h"

char* duptypes_helper(TypeStructure* ts, List* l);
int findoffsethelper(TypeStructure* ts, char* ident);
int findoffsettaghelper(TypeStructure* ts, char* ident);
Type* findtypehelper(TypeStructure* ts, char* ident);

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
            numtags += 8;
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

char* duptypes_helper(TypeStructure* ts, List* l){
    if(ts->sub == NULL){
        while(l != NULL){
            if(strcmp(l->data,ts->identifier) == 0){
                return l->data; // dup found.
            }
            if(l->next == NULL){
                l->next = malloc(sizeof (struct List));
                l->next->next = NULL;
                l->next->data = ts->identifier;
                break;
            }
            l = l->next;
        }
        return NULL;
    }
    else{
        TypeStructure* cur = ts->sub;
        while(cur != NULL){
            char* result = duptypes_helper(cur, l);
            if(result != NULL){
                return result;
            }
            cur = cur->next;
        }
        return NULL;
    }
}

// recursizely cleans up list.
void recclean(List* l){
    if(l == NULL)
        return;
    recclean(l->next);
    free(l);
}

// returns true iff t has 2+ types with same ident
char* duptypes(Type* t){
    List* l = malloc(sizeof(struct List));
    l->data = "";
    l->next = NULL;
    char* ret = duptypes_helper(t->ts, l);
    recclean(l);
    return ret;
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
                numtags += 8;
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
                    off += 8;
                    cur = cur->next;
                }
            }
            return ab + offt + off;
        }
        offt += bytes(cur);
        cur = cur->next;
        if(ts->mode != S_MODE_AND)
            off += 8;
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
