#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"common/common.h"
#include"error.h"

char* geterrorstr(int type){
    switch(type){
        case LEXERROR:
            return "Unknown Symbol";
        case SYNTAXERROR:
            return "Syntax error";
        case UNDEFTYPE:
            return "Undefined type";
        case UNDEFVAR:
            return "Undefined variable";
        case UNDEFFUNC:
            return "Undefined function";
        case DUPDEFFUNC:
            return "Duplicate definition of function";
        case DUPDEFVAR:
            return "Duplicate definition of variable";
        case DUPDEFTYPE:
            return "Duplicate definition of type";
        case INCOMPATTYPE:
            return "Incompatible types";
        default:
            break;
    }
    fprintf(stderr, "Unknown error occurred.\n");
    fprintf(stderr, "Exiting\n");
    exit(-50);
}

int stage(int type){
    switch(type){
        case LEXERROR:
            return ERRORLEXING;
        case SYNTAXERROR:
            return ERRORPARSING;
        case UNDEFTYPE:
        case UNDEFVAR:
        case UNDEFFUNC:
        case DUPDEFFUNC:
        case DUPDEFVAR:
        case DUPDEFTYPE:
        case INCOMPATTYPE:
            return ERRORSEMANTIC;
    }
    fprintf(stderr, "Unknown error occurred.\n");
    fprintf(stderr, "Exiting\n");
    exit(-50);
}

char* stagename(int stage){
    switch(stage){
        case ERRORLEXING:
            return "reading input file";
        case ERRORPARSING:
            return "parsing input file";
        case ERRORSEMANTIC:
            return "translating program";
    }
    fprintf(stderr, "Unknown error occurred.\n");
    fprintf(stderr, "Exiting\n");
    exit(-50);
}


void add_error(State* state, int code, int line, char* info){
    Error* new = malloc(sizeof(struct Error));
    new->type = code;
    new->next = NULL;
    new->count = 1;
    char* msgp1 = geterrorstr(code);
    char* msgp2 = info;
    char* msgp3 = "%s on line %i: %s\n";
    new->msg = malloc(strlen(msgp1)+strlen(msgp2)+strlen(msgp3)+21);// 20 = len of 2^64
    snprintf(new->msg, strlen(msgp1)+strlen(msgp2)+strlen(msgp3)+20, msgp3, msgp1, line, msgp2);
    if(state->errors == NULL){
        state->errors = new;
        return;
    }
    Error* e = state->errors;
    e->count = e->count + 1;
    while(e->next != NULL){
        e = e->next;
    }
    e->next = new;
}

void mark(State* state){
    if(state->errors == NULL){
        return; // no errors
    }
    Error* head = state->errors;
    fprintf(stderr, "%i errors occurred %s\n", head->count, stagename(stage(head->type)));
    while(head != NULL){
        fprintf(stderr, head->msg);
        head = head->next;
    }
    exit(stage(state->errors->type));
}
