/**
 *
 * invoker.c - this is responsible for handling subprocesses - llvm opt/llc, assembler, linker
 * 
 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include"invoker.h"


void error(int code){
    if(code == 0){
        return;
    }
    printf("Error: subprocess returned nonzero exit code\n");
    printf("Exiting.\n");
    exit(-1);
}

void invoke_assembler(char* source, char* dest){
    printf("Assembling\n");
    int i = fork();
    if(i == 0){
        execl(ASSEMBLER_PATH, ASSEMBLER_PATH, source, "-o", dest, (char*) NULL);
        return;
    }
    int s;
    wait(&s);
    remove(source);
    error(s);
}

void invoke_linker(char* source, char* dest){
    printf("Linking\n");
    int i = fork();
    if(i == 0){
        execl(LINKER_PATH, LINKER_PATH, source, "-o", dest, (char*) NULL);
        return;
    }
    int s;
    wait(&s);
    remove(source);
    error(s);
}

void invoke_llc(char* source, char* dest){
    printf("Running llvm compiler\n");
    int i = fork();
    if(i == 0){
        execl(LLC_PATH, LLC_PATH, source, "-o", dest, (char*) NULL);
        return;
    }
    int s;
    wait(&s);
    remove(source);
    error(s);
}

void invoke_opt(char* source, char* dest, int optcode){
    printf("Running llvm optimizer\n");
    char* o;
    switch(optcode){
        case 0:
            o = "-O0";
            break;
        case 1:
            o = "-O1";
            break;
        case 2:
            o = "-O2";
            break;
        case 3:
            o = "-O3";
            break;
        default:
            printf("Bug Detected");
            exit(47);
    }
    int i = fork();
    if(i == 0){
        execl(OPT_PATH, OPT_PATH, source, o, "-S", "-o", dest, (char*) NULL);
        return;
    }
    int s;
    wait(&s);
    remove(source);
    error(s);
}

/**
 * gets tempfilename. may exist or not exist, but it will be unique enough to warrent overwriting existing files.
 * note subprocesses == -1 should hit default
 * 
 */
char* get_tempfile(int subprocesses){
    char* sub_code;
    switch(subprocesses){
        case ASM:
            sub_code = "a";
            break;
        case LLC:
            sub_code = "c";
            break;
        case OPT:
            sub_code = "o";
            break;
        default:
            sub_code = "";
            break;
    }
    int r = rand();
    // generous upper bound. actual = 11 + 1 + ceil(log2(32)) + 1
    char* str = malloc(11 + 1 + 32 + 1);
    sprintf(str, "/tmp/irec%s%d", sub_code, r);
    return str;
}
