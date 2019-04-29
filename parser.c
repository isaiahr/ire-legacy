#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include"datastructs.h"
#include"writer.h"
#include"common.h"
#include"error.h"
#include"parser.h"


// THIS IS THE FOURTH TIME THE PARSER HAS BEEN REWRITTEN. and hopefully the last.

/**
 * EBNF OF SYNTAX
 * 
 * 
 * term = "\n" | ";"
 * whitesp = " ", {" "}
 * optwhitesp = {" "}
 * comment = "//"
 * 
 * identifier = (letter | underscore), { letter | digit | underscore }
 * number = ["-"], digit, {digit}
 * char = "'", ["\"], letter, "'"
 * constant = number | char
 * 
 * variable = identifier, [optwhitesp, "[", optwhitesp, expression, optwhitesp, "]"]
 * type = indentifier, [ optwhitesp, "[", optwhitesp, "]"]
 * 
 * funccall = identifier, optwhitesp, "(", optwhitesp, expression, {optwhitesp, ",", optwhitesp, expression}, optwhitesp, ")"
 * expression = (constant | string | variable | funccall)
 * assignment = identifier, optwhitesp, "=", optwhitesp, expression
 * varinit = type, optwhitesp, identifier
 * statement = varinit | expression | assignment
 * 
 * body = {optwhitesp, [statement] , [comment] optwhitesp, term}
 * funcdef = type, whitesp, identifier, optwhitesp,
 *  "(", optwhitesp, {type, whitesp, identifier, optwhitesp, ",", optwhitesp},
 *  ")", anyws,  "{"
 * 
 * 
 * function = funcdef, body, "}"
 * 
 * program = {{optwhitesp | term} function {optwhitesp | term}}
 * 
 */

/***
 * 
 * Parsing is done by recursive descent using the syntax tree shown above.
 * 
 * 
 */

ParseResult* parsestr(char* str, char* term){
    ParseResult* p = newPR();
    for(int i = 0; i<strlen(term); i++){
        char val = str[i];
        if(str[i] != 0 || val != term[i]){
            p->success = 0;
            return p;
        }
    }
    p->success = 1;
    p->str = term;
    p->read = i;
    return p;
}

// term = "\n" | ";"
ParseResult* parseterm(char* str){
    ParseResult*  p = parsestr(str, "\n");
    if(!p->success){
        p = parsestr(str, ";");
    }
    return p;
}

// whitesp = " ", {" "}
ParseResult* parsewhitesp(char* str){
    ParseResult* p = parsestr(str, " ");
    if(!p->success){
        return p;
    }
    while(1){
        ParseResult* new = parsestr(&str[p->read], " ");
        if(!new->success){
            return p;
        }
        ParseResult* n = combine(new, p);
        gc(p);
        gc(new);
        p = n;
    }
}

// optwhitesp = {" "}
ParseResult* parseoptwhitesp(char* str){
    ParseResult p = newPR();
    p->success = 1;
    while(1){
        ParseResult* new = parsestr(&str[p->read], " ");
        if(!new->success){
            return p;
        }
        ParseResult* n = combine(new, p);
        gc(p);
        gc(new);
        p = n;
    }
}

// comment = "//"
ParseResult* parsecomment(char* str){
    return parsestr(str, "//");
}

ParseResult* parseletter(char* str){
    ParseResult* p = newPR();
    if(ISALPHA(str[0])){
        p->success = 1;
        p->read = 1;
        p->str = malloc(2);
        p->str[0] = str[0];
        p->str[1] = 0;
    }
    return p;
}

ParseResult* parsedigit(char* str){
    ParseResult* p = newPR();
    if(ISDIGIT(str[0])){
        p->success = 1;
        p->read = 1;
        p->str = malloc(2);
        p->str[0] = str[0];
        p->str[1] = 0;
    }
    return p;
}

// This is the part where having gc would help readability a lot more, could chain combine func etc.

// identifier = (letter | underscore), { letter | digit | underscore }
ParseResult* parseidentifier(char* str){
    ParseResult* p = parseletter(str);
    if(!p->success){
        p = parsestr(str, "_");
        if(!p->success){
            return p;
        }
    }
    while(1){
        ParseResult* new = parseletter(&str[p->read]);
        if(!new->success){
            gc(new);
            new = parsedigit(&str[p->read]);
        }
        if(!new->success){
            gc(new);
            new = parsestr(&str[p->read], "_");
        }
        if(!new->success){
            gc(new);
            return p;
        }
        n = combine(p, new);
        gc(p);
        gc(new);
        p = n;
    }
}

// number = ["-"], digit, {digit}
ParseResult* parsenumber(char* str){
    ParseResult* p = parsestr(str, "-");
    if(!p->success){
        p->success = 1;
        p->read = 0;
    }
    ParseResult* p2 = parsedigit(&str[p->read]);
    if(!p2->success){
        free(p);
        return p2;
    }
    
    ParseResult* n = combine(p2, p);
    gc(p2);
    gc(p);
    p = n;
    
    while(1){
        ParseResult new = parsedigit(&str[p->read]);
        if(!new->success){
            free(new);
            Token* t = malloc(sizeof(struct Token));
            t->type = INT;
            t->nt = strtol(p->str, NULL, 10);
            p->token = t;
            return p;
        }
        n = combine(p, new);
        gc(p);
        gc(new);
        p = n;
    }
}

 // char = "'", ["\"], letter, "'"
ParseResult* parsechar(char* str){
    ParseResult* p = parsestr(str, "'");
    ParseResult* p2 = parsestr(str, "\\");
    int cond = 0;
    if(p2->success){
        ParseResult* e = combine(p, p2);
        gc(p);
        gc(p2);
        p = e;
        cond = 1;
    }
    else{
        gc(p2);
    }
    p->read += 1;
    ParseResult* p3 = parsestr(&str[p->read], "'");
    if(!p3->success){
        gc(p);
        return p3;
    }
    Token* t = malloc(sizeof (struct Token));
    t->type = CHAR;
    if(cond){
        if(str[p->read-1] == 'n'){
            t->chr = '\n';
        }
    }
    else {
        t->chr = str[p->read-1];
    }
    ParseResult* r = combine(p, p3);
    gc(p);
    gc(p3);
    r->token = t;
    return r;
}

// constant = number | char
ParseResult* parseconstant(char* str){
    ParseResult* r1 = parsenumber(str);
    if(r1->success){
        return r1;
    }
    gc(r1);
    return parsechar(str);
}

// variable = identifier, [optwhitesp, "[", optwhitesp, expression, optwhitesp, "]"]
ParseResult* parsevariable(char* str){
    ParseResult* ident = parseidentifier(str);
    if(!ident->success){
        return ident;
    }
    Token* to = malloc(sizeof(struct Token));
    to->type = VARIABLE_REF
    to->str = ident->str;
    ParseResult* t = parseoptwhitesp(&str[ident->read]);
    ParseResult* tc = combine(ident, t);
    gc(t);
    ParseResult* b1 = parsestr(&str[tc->read], "[");
    if(!b1->success){
        gc(tc);
        gc(b1);
        ident->token = to;
        return ident;
    }
    ParseResult* tc1 = combine(tc, b1);
    gc(tc);
    gc(b1);
    ParseResult* ws = parseoptwhitesp(&str[tc1->read]);
    ParseResult* tc2 = combine(tc1, ws);
    gc(tc1);
    gc(ws);
    ParseResult* co = parseexpression(&str[tc2->read]);
    if(!co->success){
        gc(co);
        ident->token = to;
        return ident;
    }
    
    ParseResult* tc3 = combine(tc2, co);
    gc(tc2);
    // use free instead of gc because of token.
    free(co->str);
    free(co);
    ParseResult* ws2 = parseoptwhitesp(&str[tc3->read]);
    ParseResult* tc4 = combine(tc3, ws2);
    gc(tc3);
    gc(ws2);
    ParseResult* b2 = parsestr(&str[tc4->read], "]");
    ParseResult* final = combine(tc4, b2);
    if(!final->success){
        gc(tc4);
        gc(b2);
        ident->token = to;
        return ident;
    }
    final->token = to;
    final->type = ARRAY_INDEX;
    to->t1 = co->token;
    return final;
}

/**
 * Type = identifier, [optwhitesp, "[", optwhitesp, "]"]
 */
ParseResult* parsetype(char* str){
    ParseResult* p1 = parseidentifier(str);
    if(!p1->success){
        return p1;
    }
    ParseResult* p2 = parseoptwhitesp(&str[p1->read]);
    ParseResult* p3 = combine(p1, p2);
    ParseResult* p4 = parsestr(&str[p3->read], "[");
    gc(p2);
    gc(p3);
    if(!p4->success){
        gc(p4);
        return p1;
    }
    ParseResult* p5 = combine(p3, p4);
    ParseResult* p6 = parseoptwhitesp(&str[p5->read]);
    ParseResult* p7 = combine(p5, p6);
    ParseResult* p8 = parsestr(&str[p7->read], "]");
    ParseResult* final = combine(p7, p8);
    gc(p4)
    gc(p5);
    gc(p6);
    gc(p7);
    gc(p8);
    if(!final->success){
        gc(final);
        return p1;
    }
    final->str = malloc(strlen(p1->str)+3);
    memcpy(final->str, p1->str, strlen(p1->str));
    final->str[strlen(p1->str)] = '[';
    final->str[strlen(p1->str)+1] = ']';
    final->str[strlen(p1->str)+2] = 0;
    gc(p1);
    return final;
}

// funccall = identifier, optwhitesp, "(", optwhitesp, expression, {optwhitesp, ",", optwhitesp, expression}, optwhitesp, ")"
ParseResult* parsefuncall(char* str){
    //TODO ADD TOKENS TO THIS FUNC
    ParseResult* p1 = parseidentifier(str);
    if(!p1->success){
        return p1;
    }
    ParseResult* p2 = parseoptwhitesp(&str[p1->read]);
    ParseResult* c = combine(p1, p2);
    gc(p1);
    gc(p2);
    ParseResult* p3 = parsestr(&str[c->read], "(");
    ParseResult* c2 = combine(c, p3);
    gc(c);
    if(!p3->success){
        gc(c2);
        return p3;
    }
    gc(p3);
    ParseResult* p4 = parseoptwhitesp(&str[c2->read]);
    ParseResult* c3 = combine(c2, p4);
    gc(c2);
    gc(p4);
    ParseResult* exp = parseexpression(&str[c3->read]);
    ParseResult* c4 = combine(c3, exp);
    gc(c3);
    if(!c4->success){
        gc(exp);
        return c4;
    }
    while(1){
        ParseResult* ws = parseoptwhitesp(&str[c4->read]);
        ParseResult* wcs = combine(c4, ws);
        ParseResult* comma = parsestr(&str[wcs->read], ",");
        ParseResult* t = combine(wcs, comma);
        if(!t->success){
            // no comma, end of param. ok
            gc(ws);
            gc(comma);
            gc(wcs);
            gc(exp);
            gc(t);
            break;
        }
        ParseResult* ws2 = parseoptwhitesp(&str[t->read]);
        ParseResult* wcs2 = combine(t, ws2);
        ParseResult* ne = parseexpression(&str[wcs2->read]);
        ParseResult* t2 = combine(wcs2, ne);
        gc(ws);
        gc(wcs);
        gc(comma);
        gc(t);
        gc(ws2);
        gc(wcs2);
        gc(ne);
        if(!t2->success){
            // malformed input. return fail
            gc(c4);
            return t2;
        }
        // good. continue loop.
        gc(c4);
        gc(ws);
        c4 = t2;
    }
    // done parsing params. parse end bracket
    ParseResult* wsf = parseoptwhitesp(&str[c4->read]);
    ParseResult* wsfc = combine(c4, wsf);
    gc(c4);
    gc(wsf);
    ParseResult* end = parsestr(&str[wsfc->read], ")");
    ParseResult* final = combine(wsfc, end);
    gc(wsfc);
    gc(end);
    return final;
}

// expression = (constant | string | variable | funccall)
ParseResult* parseexpression(char* str){
    ParseResult* b = parsevariable(str);
    if(!b->success){
        gc(b);
        b = parsefuncall(str);
        if(!b->success){
            gc(b);
            b = parseconstant(str);
            if(!b->success){
                gc(b);
                b = parsestring(str);
            }
        }
    }
    return b;
}

// assignment = variable, optwhitesp, "=", optwhitesp, expression
ParseResult* parseassignment(char* str){
    ParseResult* v = parsevariable(str);
    if(!v->success){
        return v;
    }
    ParseResult* p = parseoptwhitesp(&str[v->read]);
    ParseResult* c = combine(v, p);
    ParseResult* e = parsestring(&str[c->read], "=");
    ParseResult* c2 = combine(c, e);
    if(!c2->success){
        gc(p);
        gc(c);
        gc(e);
        gc(v);
        return c2;
    }
    ParseResult* o = parseoptwhitesp(&str[c2->read]);
    ParseResult* c3 = combine(c2, o);
    ParseResult* xp = parseexpression(&str[c3->read]);
    ParseResult* fi = combine(c3, xp);
    gc(v);
    gc(p);
    gc(c);
    gc(e)
    gc(c2);
    gc(o);
    gc(c3);
    gc(xp);
    return fi;
}

// varinit = type, whitesp, identifier
ParseResult* parsevarinit(char* str){
    ParseResult* p = parsetype(str);
    if(!p->success){
        return p;
    }
    ParseResult* p1 = parsewhitesp(&str[p->read]);
    ParseResult* p2 = combine(p, p1);
    if(!p2->success){
        gc(p);
        gc(p1);
        return p2;
    }
    ParseResult* p3 = parseidentifier(&str[p2->read]);
    ParseResult* f = combine(p2, p3);
    gc(p);
    gc(p1);
    gc(p2);
    gc(p3);
    return f;
}

// statement = varinit | expression | assignment
ParseResult* parsestatement(char* str){
    ParseResult* p = parseassignement(str);
    if(!p->success){
        gc(p);
        p = parsevarinit(str);
        if(!p->success){
            gc(p);
            p = parseexpression(str);
        }
    }
    return p;
}

// body = {optwhitesp, [statement] , [comment] optwhitesp, term}
// need to allow body to finish without linebreak.
ParseResult* parsebody(char* str){
    ParseResult* p = newPR();
    p->success = 1;
    while(1){
        ParseResult* w = parsewhitesp(&str[p->read]);
        ParseResult* wc = combine(p, w);
        gc(w);
        ParseResult* stmt = parsestatement(&str[wc->read]);
        ParseResult* sc = wc;
        if(stmt->success){
            sc = combine(wc, stmt);
            gc(wc);
        }
        gc(stmt);
        ParseResult* comment = parsecomment(&str[sc->read]);
        ParseResult* cc = sc;
        if(comment->success){
            cc = combine(sc, comment);
            gc(sc);
        }
        gc(comment);
        ParseResult* ows = parseoptwhitesp(&str[cc->read]);
        ParseResult* owsc = combine(cc, ows);
        ParseResult* t = parseterm(&str[owsc->read]);
        ParseResult* f = combine(owsc, t);
        gc(cc);
        gc(ows);
        gc(owsc);
        gc(t);
        if(!f->success){
            gc(f);
            return p;
        }
        gc(p);
        p = f;
    }
}
/**
 * funcdef = type, whitesp, identifier, optwhitesp,
 *  "(", optwhitesp, {type, whitesp, identifier, optwhitesp, ",", optwhitesp},
 *  ")", anyws,  "{"
 */
ParseResult* parsefuncdef(char* str){
    ParseResult* p = parsetype(str);
    if(!p->success){
        return p;
    }
    ParseResult* p1 = parsewhitesp(&str[p->read]);
    if(!p1->success){
        gc(p);
        return p1;
    }
    ParseResult* p2 = combine(p, p1);
    ParseResult* p3 = parseidentifier(&str[p2->read]);
    if(!p3->success){
        gc(p);
        gc(p1);
        gc(p2);
        return p3;
    }
    ParseResult* p4 = combine(p2, p3);
    ParseResult* p5 = parseoptwhitesp(p3, p4);
    ParseResult* p6 = combine(
}

// function = funcdef, body, "}"
ParseResult* parsefunction(char* str){
    ParseResult* p = parsefuncdef(str);
    if(!p->success){
        return p;
    }
    ParseResult* p2 = parsebody(&str[p->read]);
    if(!p2->success){
        gc(p);
        return p2;
    }
    ParseResult* p3 = combine(p, p2);
    gc(p);
    gc(p2);
    ParseResult* ret = parsestr(&str[p3->read], "}");
    gc(p3);
    return ret;
}

// anyws = {whitesp | term}
ParseResult* parseanyws(char* str){
    ParseResult* p = newPR();
    while(1){
        ParseResult* n = parsewhitesp(&str[p->read]);
        if(!n->success){
            gc(n);
            n = parseterm(&str[p->read]);
            if(!n->success){
                gc(n);
                return p;
            }
        }
        ParseResult* x = combine(p, n);
        gc(p);
        gc(n);
        p = x;
    }
}

// program = {anyws function}
ParseResult* parseprogram(char* str){
    ParseResult* p = newPR();
    while(1){
        ParseResult* m = parseanyws(str);
        ParseResult* i = combine(p, m);
        ParseResult* j = parsefunction(&str[i]);
        ParseResult* k = combine(i, j);
        if(!k->success){
            gc(m);
            gc(i);
            gc(j);
            gc(k);
            return p;
        }
        gc(m);
        gc(i);
        gc(j);
        gc(p);
        p = k;
    }
}
/**
 * EBNF OF SYNTAX
 * 
 * 
 * term = "\n" | ";"
 * whitesp = " ", {" "}
 * optwhitesp = {" "}
 * comment = "//"
 * 
 * identifier = (letter | underscore), { letter | digit | underscore }
 * number = ["-"], digit, {digit}
 * char = "'", ["\"], letter, "'"
 * constant = number | char
 * 
 * variable = identifier, [optwhitesp, "[", optwhitesp, expression, optwhitesp, "]"]
 * type = indentifier, [ optwhitesp, "[", optwhitesp, "]"]
 * 
 * funccall = identifier, optwhitesp, "(", optwhitesp, expression, {optwhitesp, ",", optwhitesp, expression}, optwhitesp, ")"
 * expression = (constant | string | variable | funccall)
 * assignment = variable, optwhitesp, "=", optwhitesp, expression
 * varinit = type, whitesp, identifier
 * statement = varinit | expression | assignment
 * 
 * 
 * body = {optwhitesp, [statement] , [comment] optwhitesp, term}
 * funcdef = identifier, whitesp, identifier, optwhitesp,
 *  "(", optwhitesp, {type, whitesp, identifier, optwhitesp, ",", optwhitesp},
 *  ")", {optwhitesp | term},  "{"
 * 
 * 
 * function = funcdef, body, "}"
 * 
 * program = {{optwhitesp | term} function {optwhitesp | term}}
 * 
 */


//
// ex = seq(seq(parseidentifier, parsewhitesp), parseidentifier) (str)
//

ParseResult* seqh(ParseResult* (first) (char*), ParseResult* (second) (char*), char* con){
    ParseResult* f = first(con);
    ParseResult* s = second(con);
    return combine(f, s);
}

typedef ParseResult* (*parser) (char*);

(ParseResult* (result) char*) seq(ParseResult* (first) (char*), ParseResult* (second) (char*)){
    return seqh(first, second, x);
}

ParseResult* combine(ParseResult* p1, ParseResult* p2){
    ParseResult* new = newPR();
    if(!(p1->success && p2->success)){
        // no successes.
        new->success = 0;
        return new;
    }
    new->read = p1->read + p2->read;
    if(p1->str != NULL && p2->str != NULL){
        new->str = malloc(strlen(p1->str) + strlen(p2->str) + 1);
        memcpy(new->str, p1->str, strlen(p1->str));
        memcpy(&new->str[strlen(p1->str)], p2->str, strlen(p2->str));
        new->str[strlen(p1->str)+strlen(p2->str)] = 0;
    }
    return new;
}

void gc(ParseResult* pr){
    if(p->str != NULL){
        free(p->str);
    }
    if(p->token != NULL){
        // TODO: better gc for token
        free(p->token);
    }
    free(p);
}

ParseResult* newPR(){
    ParseResult* p = malloc(sizeof(struct ParseResult));
    p->success = 0;
    p->token = NULL;
    p->str = NULL;
    p->read = 0;
}




// turns a token candidate (typically a line) into a token.
Token* tokenize(char* str, int line, State* state){
    if(str[0] == ' '){
        return tokenize(&str[1], line, state);
    }
    Token* t = malloc(sizeof (Token));
    if(beginswith("import ", str)){
        t->type = IMPORT;
        return t;
    }
    if(beginswith("//", str)){
        t->type = COMMENT;
        return t;
    }
    if(beginswith("def ", str)){
        t->type = FUNCTION_DEFN;
        t->str = copy(str+4, " ", " {");
        if(t->str == NULL){
            error(SYNTAXERROR, line, str);
        }
        validname(t->str, line);
        return t;
    }
    if(beginswith("return ", str)){
        t->type = FUNCTION_RETURN;
        char* ret = copy(str+7, " ", " ");
        t->t1 = tokenize(ret, line, state);
        return t;
    }
    if(str[0] == '}'){
        t->type = FUNCTION_END;
        return t;
    }
    if(str[0] == '`'){
        t->type = ASM;
        t->str = oldcopy(str, 1, strlen(str));
        return t;
    }
    if(str[0] == '\''){
        t->type = CHAR;
        if(str[1] == '\\'){
            // escape
            if(str[2] == 'n'){
                t->chr = '\n';
            }
            if(str[2] == '\\'){
                t->chr = '\\';
            }
        }
        else if(str[2] == '\''){
            t->chr = str[1];
        }
        else{
            error(SYNTAXERROR, line, str);
        }
        return t;
    }
    if(str[0] == '"'){
        t->type = STRING;
	int len; 
    	t->str = proc_str(str+1, &len);
    	t->nt = len;
    	return t;
    }
    if(ISNUMERIC(str[0])){
        t->type = INT;
        char* b = copy(str, "", " ");
        t->nt = strtol(b, NULL, 10);
        return t;
    }
    char* arr = copy(str, "", "[");
    Variable* v0 = NULL;
    if(arr != NULL && str[strlen(arr)] == '['){
        v0 = ref_var(state->currentfunc, arr, state);
    }
    if(v0 != NULL){
        //array operation
        int indxe = indexchr(str, '=');
        int indxb = indexchr(str, ']');
        if((indxb != -1 && indxe != -1) && (indxe > indxb)){
            t->type = ARRAY_SET;
            char* inner = match_sqparen(str);
            if(inner == NULL){
                error(SYNTAXERROR, line, str);
            }
            t->t1 = tokenize(inner, line, state);
            t->t2 = tokenize(copy(&str[indxe+1], " ", " "), line, state);
            t->var1 = v0;
            return t;
        }
        if(indxb == -1){
            error(SYNTAXERROR, line, str);
        }
        t->type = ARRAY_INDEX;
        t->var1 = v0;
        char* inner = match_sqparen(str);
        if(inner == NULL){
            error(SYNTAXERROR, line, str);
        }
        t->t1 = tokenize(inner, line, state);
        return t;
    }
    char* begin = copy(str, "", " ");
    if(begin != NULL){
        Type* ty = ref_type(begin, state);
        if(ty != NULL){
            t->type = VARIABLE_DEFN;
            t->t = ty;
            char* varname = copy(str+strlen(begin), " ", " ");
            validname(varname, line);
            t->str = varname;
            return t;
        }
        int indxb = indexchr(str, '(');
        int indxe = indexchr(str, '=');
        if(indxb != -1 && indxe == -1){
            t->type = FUNCTION_CALL;
            char* fname = copy(str, "", " (");
            validname(fname, line);
            Function* fun = ref_func(fname, state);
            t->str = fname;
            t->func = fun;// might be null, that is OK.
            char* inner = match_paren(str);
            if(inner == NULL){
                error(SYNTAXERROR, line, str);
            }
            t->t1 = tokenize(inner, line, state);
            return t;
        }
        if(indxe != -1){
            if(indexstr(str, "+=") == indxe-1){
                t->type = ARRAY_ADD;
                char* v = copy(str, "", " +");
                Variable* var = ref_var(state->currentfunc, v, state);
                char* end = copy(str+strlen(v), " +=", "");
                t->var1 = var;
                t->t1 = tokenize(end, line, state);
                return t;
            }
            t->type = ASSIGNMENT;
            Variable* var = ref_var(state->currentfunc, begin, state);
            if(var == NULL){
                error(UNDEFVAR, line, str);
            }
            t->var1 = var;
            char* assigner = copy(&str[indxe+1], " ", "");
            t->t1 = tokenize(assigner, line, state);
            return t;
        }
        Variable* v = ref_var(state->currentfunc, begin, state);
        if(v != NULL){
            t->type = VARIABLE_REF;
            t->var1 = v;
            return t;
        }
    }
    t->type = INVALID;
    return t;
    

}
int indexchr(char* str, char chr){
    int i = 0;
    while(str[i] != 0){
        if(str[i] == chr)
            return i;
        i++;
    }
    return -1;
}

void validname(char* name, int line){
    if(strlen(name) < 1)
        error(SYNTAXERROR, line, name);
    if(!ISALPHA(name[0]))
        error(SYNTAXERROR, line, name);
    int i = 1;
    while(name[i] != 0){
        if(!(ISALPHA(name[i]) || ISNUMERIC(name[i]))){
            error(SYNTAXERROR, line, name);
        }
        i+= 1;
    }   
}

// example input "((()))"
char* match_paren(char* input){
    int numpar = 1;
    int first = -1;
    int i = 0;
    while(input[i] != 0){
        if(input[i] == '(' && first == -1){
            first = i;
        }
        else if (input[i] == '('){
            numpar += 1;
        }
        else if (input[i] == ')'){
            numpar -= 1;
        }
        if(numpar == 0){
            char* x = oldcopy(input, first+1, i);
            return x;
        }
        i++;
    }
    return NULL;
}

char* match_sqparen(char* input){
    int numpar = 1;
    int first = -1;
    int i = 0;
    while(input[i] != 0){
        if(input[i] == '[' && first == -1){
            first = i;
        }
        else if (input[i] == '['){
            numpar += 1;
        }
        else if (input[i] == ']'){
            numpar -= 1;
        }
        if(numpar == 0){
            char* x = oldcopy(input, first+1, i);
            return x;
        }
        i++;
    }
    return NULL;
}

// returns index of first occurance of st2 in st1.
// so indexstr (a += b) (+=) -> 2
int indexstr(char* str1, char* str2){
    // index
    int i = 0;
    // match index
    int mi = 0;
    while(str1[i+mi] != 0){
        if(str2[mi] == 0){
            return i;
        }
        if(str1[i+mi] == str2[mi]){
            mi += 1;
        }
        else{
            mi = 0;
            i++;
        }
    }
    return -1;

}

char* copy(char* token, char* pass, char* end){
    int i = 0;
    int ind = -1;
    while(token[i] != 0){
        if(ind == -1 && strchr(pass, token[i]) == NULL){
            // first char we dont pass on. set start to here.
            if(ind == -1){
                ind = i;
            }
        }
        else if(strchr(end, token[i]) != NULL && ind != -1){
            // start index is set, and matches end.
            return oldcopy(token, ind, i);
        }
        i++;
    }
    if(ind == -1){
        return NULL;
    }
    return oldcopy(token, ind, i);
}

//returns copy including ind0, but not including ind1
char* oldcopy(char* token, int ind0, int ind1){
    char* result = malloc(ind1-ind0+1);
    memcpy(result, &token[ind0], ind1-ind0);
    result[ind1-ind0] = 0;
    return result;
}


int beginswith(char* begin, char* token){

    int matches = 1;
    int j = 0;
    while(matches && token[j] != 0){
        matches = matches && (begin[j] == token[j]);
        j += 1;
        if(begin[j] == 0)
            return matches;
    }
    return 0;
}

char* proc_str(char* data, int* len){
    int esc_next = 0;
    int stringalloc = 32;
    int stringind = 0;
    char* string = (char*) malloc(stringalloc);
    for(long i=0; data[i] != 0; i++){ 
        char c = data[i];
        if(c == '"' && (esc_next == 0)){
            string[stringind] = '\0';
            esc_next = 0;
            (*len) = i-1;
            return string;
        }
        else if(c == '"' && (esc_next == 1)){
            string[stringind] = c;
            stringind = stringind + 1;
            if(stringind == stringalloc){//allocate more space
                stringalloc = stringalloc * 2;
                string = (char*) realloc(string, stringalloc);
            }
            esc_next = 0;
        }
        else {
            if(c == '\\' && esc_next == 0){
                esc_next = 1;
            }
            else {
                if(esc_next == 1){
                    if(c == 'n')
                        string[stringind] = '\n';
                }
                else{
                    string[stringind] = c;
                }
                stringind=stringind+1;
                if(stringind == stringalloc){
                    stringalloc = stringalloc * 2;
                    string = (char*) realloc(string, stringalloc);
                }
                esc_next = 0;
            }
        }
    }
    return NULL;
}
