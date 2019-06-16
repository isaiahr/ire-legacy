#include<stdarg.h>
#include<stdlib.h>
#include<stdio.h>
#include"datastructs.h"
#include"common.h"
#include"operators.h"

// a wrapper for prints
void debug(State* st, char* format, ...){
    if(!st->verbose)
        return;
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

char sym(int id){
    switch(id){
        case PLUS: return '+';
        case DOUBLEEQUALS: return '=';
        case LESS: return '<';
        case GREATER: return '>';
        case SUBTRACT: return '-';
        case MULT: return '*';
        default: exit(67); // shouldn't happen, but easy to track down if it does
    }
}
