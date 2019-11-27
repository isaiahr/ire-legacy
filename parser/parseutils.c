#include<stdlib.h>
#include<stdio.h>
#include"parser.h"
#include"parseutils.h"

int match(Lextoken* p, int m){
    if(p == NULL){
        return 0;
    }
    return p->type == m;
}

Lextoken* next(Lextoken* p){
    if(p == NULL){
        return NULL;
    }
    return p->next;
}

Token* init_token(int line){
    Token* t = malloc(sizeof(struct Token));
    t->str = NULL;
    t->lnt = 0;
    t->line = line;
    t->subtokens = NULL;
    t->chr = 0;
    return t;
}

Token* allocate_child_token(Token* t, int line){
    Token* child = init_token(line);
    TokenList* node = malloc(sizeof(struct TokenList));
    node->next = NULL;
    node->token = child;
    if(t->subtokens == NULL){
        t->subtokens = node;
        return child;
    }
    TokenList* c = t->subtokens;
    while(c->next != NULL){
        c = c->next;
    }
    c->next = node;
    return child;
}

/**
 * this function makes the adoptedchild param a child of gaurdian.
 * since child is already existing and not "birthed" from allocate_child_token, 
 * we are calling this "adopt" instead. 
 * 
 */
void adopt_child_token(Token* guardian, Token* adoptedchild){
    TokenList* new = malloc(sizeof(struct TokenList));
    new->token = adoptedchild;
    new->next = NULL;
    if(guardian->subtokens == NULL){
        guardian->subtokens = new;
        return;
    }
    TokenList* u = guardian->subtokens;
    while(u->next != NULL){
        u = u->next;
    }
    u->next = new;
}

int subtoken_count(Token* t){
    TokenList* c = t->subtokens;
    int count = 0;
    while(c != NULL){
        count = count + 1;
        c = c->next;
    }
    return count;
}

void destroy_token(Token* ptr){
    if(ptr->subtokens != NULL){
        destroy_children(ptr);
    }
    free(ptr);
}

void destroy_children(Token* parent){
    TokenList* c = parent->subtokens;
    while(c != NULL){
        destroy_token(c->token);
        TokenList* n = c->next;
        free(c);
        c = n;
    }
    parent->subtokens = NULL;
}

/**
 * destroys the newest child node. this is done because we eagerly allocate a new token
 * when a parser can return n tokens.
 */
void destroy_youngest(Token* parent){
    TokenList* c = parent->subtokens;
    if(c == NULL){
        return;
    }
    if(c->next == NULL){
        destroy_token(c->token);
        free(c);
        parent->subtokens = NULL;
        return;
    }
    while(c->next->next != NULL){
        c = c->next;
    }
    destroy_token(c->next->token);
    free(c->next);
    c->next = NULL;
}

Token* subtoken(Token* parent, int index){
    TokenList* c = parent->subtokens;
    if(c == NULL){
        return NULL;
    }
    while(index != 0){
        if(c->next == NULL){
            return NULL;
        }
        c = c->next;
        index = index - 1;
    }
    return c->token;
}

void print_tree(Token* p, int lvl){
    char* lvlstr = malloc(lvl+1);
    for(int i = 0; i < lvl; i++){
        lvlstr[i] = ' ';
    }
    lvlstr[lvl] = 0;
    if(p->type == T_ARITH){
        printf("%s%s <%c>\n", lvlstr, type(p), sym(p->lnt));
    }
    else if(p->type == T_INT){
        printf("%s%s %ld\n", lvlstr, type(p), p->lnt);
    }
    else if(p->str == NULL){
        printf("%s%s\n", lvlstr, type(p));
    } else {
        printf("%s%s [%s]\n", lvlstr, type(p), p->str);
    }
    free(lvlstr);
    TokenList* t = p->subtokens;
    while(t != NULL){
        print_tree(t->token, lvl+4);
        t = t->next;
    }
}

char* type(Token* p){
    switch(p->type){
        case T_PROGRAM: return "PROGRAM";
        case T_FUNCTION: return "FUNCTION";
        case T_ASSIGNMENT: return "ASSIGNMENT";
        case T_TYPE: return "TYPE";
        case T_VARIABLE: return "VARIABLE";
        case T_CHAR: return "CHAR";
        case T_INT: return "INT";
        case T_STRING: return "STRING";
        case T_VARINIT: return "VARINIT";
        case T_FUNCDEF: return "FUNCDEF";
        case T_VARPARAM: return "VARPARAM";
        case T_BODY: return "BODY";
        case T_RETURN: return "RETURN";
        case T_FUNCALL: return "FUNCALL";
        case T_INDGET: return "INDGET";
        case T_INDSET: return "INDSET";
        case T_ADDEQ: return "ADDEQ";
        case T_CARDINALITY: return "CARD";
        case T_ARITH: return "ARITH";
        case T_TYPEDEF: return "TYPEDEF";
        case T_ANDTYPE: return "ANDTYPE";
        case T_ORTYPE: return "ORTYPE";
        case T_TYPEVAL: return "TYPEVAL";
        case T_SEGMENT: return "SEGMENT";
        case T_CONSTRUCTOR: return "CONSTRUCTOR";
        case T_SEGCONSTRUCT: return "SEGCONSTRUCT";
        case T_ACCESSOR: return "ACCESSOR";
        case T_SETMEMBER: return "SETMEMBER";
        case T_CONSTRUCTASSIGN: return "CONSTRUCTASSIGN";
        case T_MEMBERIDENT: return "MEMBERIDENT";
        case T_IF: return "IF";
        case T_ELSEIF: return "ELSEIF";
        case T_ELSE: return "ELSE";
        case T_IFBLK: return "IFBLK";
        case T_BOOLEAN: return "BOOLEAN";
        case T_INVERT: return "INVERT";
        case T_GETTAG: return "GETTAG";
        default: return "UNKNOWN";
    }
    
}
