#ifndef __PARSER_TYPE_H__
#define __PARSER_TYPE_H__

Lextoken* parse_subtype(Lextoken* p, Token* e);
Lextoken* parse_subtype_flags(Lextoken* p, Token* e, int FLAGS);
Lextoken* parse_andtype_flags(Lextoken* p, Token* e, int FLAGS);
Lextoken* parse_xortype(Lextoken* p, Token* e);
Lextoken* parse_ortype(Lextoken* p, Token* e);
Lextoken* parse_typeval(Lextoken* p, Token* e);
Lextoken* parse_typedef(Lextoken* p, Token* e, State* state);

#endif
