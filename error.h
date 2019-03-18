#ifndef __ERROR_H__
#define __ERROR_H__

#define SYNTAXERROR 1
#define UNDEFVAR 2
#define UNDEFTYPE 6
#define UNDEFFUNC 3
#define DUPDEFFUNC 4
#define DUPDEFVAR 5
#define DUPDEFTYPE 7

extern char* geterrorstr(int type);
extern void error(int type, int line, char* token);
extern void errornl(int type, char* msg);
extern void warning(int type, int line, char* token);
#endif
