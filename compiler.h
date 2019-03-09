#ifndef __COMPILER_H__
#define __COMPILER_H__
#define ISALPHA(x) ((x > 64 && x < 91) || (x > 96 && x < 123))
#define ISNUMERIC(x) (x > 47 && x < 58) 

//token types
#define ASSIGNMENT 2
#define FUNCTION_CALL 3
#define FUNCTION_DEFN 4
#define FUNCTION_RETURN 8
#define FUNCTION_END 7
#define CONDITIONAL 5
#define ASM 6
#define INVALID -1
// Data structures


extern void compile(State* state, char* data, long sz);
extern int matches(char* big, char* small);
extern char* proc_str(char* data, long max, int* index);
extern void process_token(char* token, State* state);
extern int get_token_type(char* token);
extern int nextwhite(char* str);
extern int nextnonwhite(char* str);

#endif
