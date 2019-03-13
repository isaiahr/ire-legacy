#include<stdio.h>
#include<stdlib.h>
#include"datastructs.h"
#include"error.h"

char* geterrorstr(int type){

    if(type == SYNTAXERROR)
        return "Syntax error";
    if(type == UNDEFVAR)
        return "Undefined variable";
    if(type == UNDEFFUNC)
        return "Undefined function";
    if(type == DUPDEFFUNC)
        return "Duplicate definition of function";
    if(type == DUPDEFVAR)
        return "Duplicate definition of variable";
    printf("Unknown error occurred.\n");
    printf("Exiting\n");
    exit(-50);
}

void errornl(int type, char* msg){
    char* error = geterrorstr(type);
    printf("Compilation Failed.\n");
    printf("%s %s\n", error, msg);
    printf("Exiting\n");
    exit(1);
}


void error(int type, int line, char* token){
    char* error = geterrorstr(type);
    printf("Compilation Failed.\n");
    printf("%s on line %i, token \"%s\"\n", error, line, token);
    printf("Exiting\n");
    exit(1);
}

void warning(int type, int line, char* token){
    //TODO add warnings
}
