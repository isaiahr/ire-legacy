#ifndef __PARSER_EXPR_H__
#define __PARSER_EXPR_H__

#define FLAG_ARRIND 1
#define FLAG_ARITH 2
#define FLAG_ACCESSOR 4

Lextoken* parse_constant(Lextoken* p, Token* t);
Lextoken* parse_variable(Lextoken* p, Token* t);
Lextoken* parse_expression(Lextoken* p, Token* t);
Lextoken* parse_expression_flags(Lextoken* p, Token* t, int flags);
Lextoken* parse_funcall(Lextoken* p, Token* t);
Lextoken* parse_arrind(Lextoken* p, Token* e);
Lextoken* parse_arrind_flags(Lextoken* p, Token* e, int flags);
Lextoken* parse_card(Lextoken* p, Token* e);
Lextoken* parse_newarr(Lextoken* p, Token* e);
Lextoken* parse_brexpr(Lextoken* p, Token* e);
Lextoken* parse_constructor(Lextoken* p, Token* e);
Lextoken* parse_segconstruct(Lextoken* p, Token* e);
Lextoken* parse_accessor(Lextoken* p, Token* e);
Lextoken* parse_accessor_flags(Lextoken* p, Token* e, int FLAGS);

#endif
