#include<stdarg.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
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

/**
 * printf to string, except only strings are allowed (handled properly (for now))
 */
char* format(char* format, ...){
    int count = 0;
    for(int i =0; i < strlen(format); i++){
        if(format[i] == '%' && format[i+1] == '%'){
            i += 1;
            continue;
        }
        if(format[i] == '%'){
            count += 1;
        }
    }
    int sz = strlen(format);
    va_list args;
    va_list argz;
    va_start(args, format);
    va_copy(argz, args);
    for(int i =0; i < count; i++){
        char* x = va_arg(args, char*);
        sz += strlen(x);
    }
    char* result = malloc(sz + 1);
    result[sz] = 0;
    va_end(args);
    vsnprintf(result, sz, format, argz);
    va_end(argz);
    return result;
    
}
