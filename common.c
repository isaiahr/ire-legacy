#include<stdarg.h>
#include<stdio.h>
#include"datastructs.h"
#include"common.h"

// a wrapper for prints
void debug(State* st, char* format, ...){
    if(!st->verbose)
        return;
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
