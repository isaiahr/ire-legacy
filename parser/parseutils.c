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
    t->subtoken_count = 0;
    t->lnt = 0;
    t->line = line;
    t->subtokens = NULL;
    t->chr = 0;
    return t;
}

Token* realloc_token(Token* ptr, int len){
    Token* pt = realloc(ptr, len*sizeof(struct Token));
    pt[len-1].str = NULL;
    pt[len-1].subtoken_count = 0;
    pt[len-1].lnt = 0;
    pt[len-1].line = ptr->line;
    pt[len-1].subtokens = NULL;
    pt[len-1].chr = 0;
    return pt;
}

void destroy_token(Token* ptr){
    return; // TODO make this work in the future.
    // note: dont recurse, functions already should do that
    if(ptr == NULL){
        return;
    }
    for(int i = 0; (i*sizeof(struct Token)) < sizeof(ptr); i++){
        if(ptr[i].str != NULL){
            free(ptr[i].str);
            ptr[i].str = NULL;
        }
        if(ptr[i].subtokens != NULL){
            destroy_token(ptr[i].subtokens);
            ptr[i].subtokens = NULL;
        }
    }
    free(ptr);
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
    else if(p->str == NULL){
        printf("%s%s\n", lvlstr, type(p));
    } else {
        printf("%s%s [%s]\n", lvlstr, type(p), p->str);
    }
    free(lvlstr);
    if(p->subtokens != NULL){
        for(int i = 0; i < p->subtoken_count; i++){
            print_tree(&p->subtokens[i], lvl+4);
        }
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
        case T_XORTYPE: return "XORTYPE";
        case T_TYPEVAL: return "TYPEVAL";
        case T_SEGMENT: return "SEGMENT";
        case T_CONSTRUCTOR: return "CONSTRUCTOR";
        case T_SEGCONSTRUCT: return "SEGCONSTRUCT";
        case T_ACCESSOR: return "ACCESSOR";
        case T_SETMEMBER: return "SETMEMBER";
        case T_CONSTRUCTASSIGN: return "CONSTRUCTASSIGN";
        case T_MEMBERIDENT: return "MEMBERIDENT";
        case T_IF: return "IF";
        default: return "UNKNOWN";
    }
    
}
