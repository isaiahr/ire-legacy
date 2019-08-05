#ifndef __PARSER_STMT_H__
#define __PARSER_STMT_H__

Lextoken* parse_type(Lextoken* p, Token* t);
Lextoken* parse_varinit(Lextoken* p, Token* t);
Lextoken* parse_arrset(Lextoken* p, Token* e);
Lextoken* parse_addeq(Lextoken* p, Token* e);
Lextoken* parse_assignment(Lextoken* p, Token* t);
Lextoken* parse_statement(Lextoken* p, Token* t, State* state);
Lextoken* parse_setmember(Lextoken* p, Token* e);
Lextoken* parse_if(Lextoken* p, Token* e, State* state);

#endif
