#include<stdio.h>
#include<stdlib.h>
#include"datastructs.h"
#include"error.h"

char* geterrorstr(int type){

    if(type == SYNTAXERROR)
        return "Syntax error";
    if(type == UNDEFTYPE)
        return "Undefined type";
    if(type == UNDEFVAR)
        return "Undefined variable";
    if(type == UNDEFFUNC)
        return "Undefined function";
    if(type == DUPDEFFUNC)
        return "Duplicate definition of function";
    if(type == DUPDEFVAR)
        return "Duplicate definition of variable";
    if(type == DUPDEFTYPE)
        return "Duplicate definition of type";
    fprintf(stderr, "Unknown error occurred.\n");
    fprintf(stderr, "Exiting\n");
    exit(-50);
}

void errornl(int type, char* msg){
    char* error = geterrorstr(type);
    fprintf(stderr, "Compilation Failed.\n");
    fprintf(stderr, "%s %s\n", error, msg);
    fprintf(stderr, "Exiting\n");
    exit(1);
}


void error(int type, int line, char* token){
    char* error = geterrorstr(type);
    fprintf(stderr, "Compilation Failed.\n");
    fprintf(stderr, "%s on line %i, token \"%s\"\n", error, line, token);
    fprintf(stderr, "Exiting\n");
    exit(1);
}

void warning(int type, int line, char* token){
    //TODO add warnings
}
