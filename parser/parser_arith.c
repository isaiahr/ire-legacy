#include<string.h>
#include<stdlib.h>
#include"parser.h"
#include"parseutils.h"
#include"parser_arith.h"
#include"parser_expr.h"

// inclusive
#define LAST_ORDER 4


// arith = exprnoarith ("<" | ">" | "=" | "+" | "-") exprnoarith ...
/**
* Algorithmn details: O(nk) n -> num tokens, k -> numsymbols (constant in this case)
*/


typedef struct ArithExpr{
    Token* p;
    struct OpExpr* next;
    struct OpExpr* prev;
} ArithExpr;

typedef struct OpExpr {
    int type;
    struct ArithExpr* next;
    struct ArithExpr* prev;
} OpExpr;


OpExpr* opexprhelper(Lextoken* p);
int ordermatches(int order, int type);


OpExpr* opexprhelper(Lextoken* p){
    switch(p->type){
        case PLUS:
        case DOUBLEEQUALS:
        case LESS:
        case GREATER:
        case SUBTRACT:
        case MULT:
        case PERCENT:
        case FSLASH:
        case PIPE:
        case AMPERSAND:
            ; // nessecary
            OpExpr* o = malloc(sizeof(struct OpExpr));
            o->type = p->type;
            o->next = NULL;
            o->prev = NULL;
            return o;
        default: return NULL;
    }
}



/**
 *
 * Note: lower order = strong associativity. (or higher order)
 * higher orders "wait" for lower orders to compute before computing.
 * a expr like a == b * c + d
 * will be (a == ((b*c)+d))
 * 
 */

int ordermatches (int order, int type){
    switch(type){
        case MULT:
        case PERCENT:
        case FSLASH:
            return order == 0;
        case PLUS:
        case SUBTRACT:
            return order == 1;
        case LESS:
        case GREATER:
            return order == 2;
        case DOUBLEEQUALS:
            return order == 3;
        case AMPERSAND:
        case PIPE:
            return order == 4;
        default:
            return 0;
    }
    return 0; // shouldnt happen
}

Lextoken* parse_arith(Lextoken* p, Token* e){
    return parse_arith_flags(p, e, 0);
}

Lextoken* parse_arith_flags(Lextoken* p, Token* e, int flags){
    // 1. build the initial list.
    ArithExpr* head = malloc(sizeof(struct ArithExpr));
    Token* o = init_token(p->line);
    Lextoken* l = parse_expression_flags(p, o, FLAG_ARITH | flags);
    if(l == NULL){
        destroy_token(o);
        free(head);
        return NULL;
    }
    head->p = o;
    head->next = opexprhelper(l);
    head->prev = NULL;
    if(head->next == NULL){
        destroy_token(o);
        free(head);
        return NULL;
    }
    head->next->next = malloc(sizeof(struct ArithExpr));
    o = init_token(p->line);
    l = parse_expression_flags(next(l), o, FLAG_ARITH);
    if(l == NULL){
        destroy_token(head->p);
        destroy_token(o);
        free(head);
        free(head->next);
        free(head->next->next);
        return NULL;
    }
    head->next->next->p = o;
    head->next->next->next = NULL;
    head->next->next->prev = head->next;
    head->next->prev = head;
    // valid.
    ArithExpr* cur = head->next->next;
    while(1){
        OpExpr* op = opexprhelper(l);
        if(op == NULL){
            //ok. proceed.
            break;
        }
        op->next = NULL;
        cur->next = op;
        cur->next->prev = cur;
        Token* newt = init_token(next(l)->line);
        l = parse_expression_flags(next(l), newt, FLAG_ARITH);
        if(l == NULL){
            // failed, no expr on end (such as "3*2*")
            // gc all held items
            int y = 1;
            void* hed = (void*) head;
            while(head != NULL){
                if(y){
                    ArithExpr* ae = (ArithExpr*) hed;
                    destroy_token(ae->p);
                    free(ae);
                    hed = (void*) ae->next;
                }
                else{
                    OpExpr* oe = (OpExpr*) hed;
                    free(oe);
                    hed = (void*) oe->next;
                }
                y = !y;
            }
            return NULL;
        }
        cur->next->next = malloc(sizeof(struct ArithExpr*));
        cur->next->next->prev = cur->next;
        cur->next->next->next = NULL;
        cur->next->next->p = newt;
        cur = cur->next->next;
        // ok. proceed.
    }
    // 2. parse flat tree to account for order of operations.
    // lower order = first.
    for(int order = 0; order <= LAST_ORDER; order++){
        ArithExpr* cur = head;
        while(cur->next != NULL){
            // note cur->next nonull implies cur->next->next exists.
            if(ordermatches(order, cur->next->type)){
                // replace this one.
                Token* repl = init_token(cur->p->line);
                repl->subtoken_count = 2;
                repl->subtokens = init_token(cur->p->line);
                repl->subtokens = realloc_token(repl->subtokens, 2);
                repl->lnt = cur->next->type; // set operation
                memcpy(repl->subtokens, cur->p, sizeof(struct Token));
                memcpy(&repl->subtokens[1], cur->next->next->p, sizeof(struct Token));
                repl->type = T_ARITH;
                destroy_token(cur->p);
                destroy_token(cur->next->next->p);
                cur->p = repl;
                OpExpr* destroy = cur->next;
                cur->next = cur->next->next->next;
                if(cur->next != NULL){
                    cur->next->prev = cur;
                    // possible case. ok.
                }
                free(destroy->next);
                free(destroy);
                // ok.
            }
            else{
                cur = cur->next->next;
                // not precedence we are looking for.
            }
        }
    }
    // should be binary tree now.
    memcpy(e, head->p, sizeof(struct Token));
    destroy_token(head->p);
    return l;
}
