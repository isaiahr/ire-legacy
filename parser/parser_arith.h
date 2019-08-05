#ifndef __PARSER_ARITH_H__
#define __PARSER_ARITH_H__

Lextoken* parse_arith(Lextoken* p, Token* e);
Lextoken* parse_arith_flags(Lextoken* p, Token* e, int flags);

#endif
