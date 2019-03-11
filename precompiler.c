#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<error.h>
#include"precompiler.h"

// The precompiler.
// all it does is handle file loading and import statements.
// does not change the actual structure of the file.


/**
loads files in dfs order
*/
Compilationfile* precompile(char* path){
    Compilationfile *head = malloc(sizeof(Compilationfile));
    head->path = path;
    loadfile(head);
    int i = 0;
    int cond = 0;
    char* data = head->data;
    while(i < head->sz){
        if(cond == 2){// already at "import"
            int j = i;
            while(data[j] != 0 && data[j] != '\n') j++;
            char* filename = malloc(j-i+1);
            memcpy(filename, &data[i], j-i);
            filename[j-i] = 0;
            Compilationfile* next = precompile(filename);
            Compilationfile* cur = head;
            //TODO: detect files loading each other
            while(cur->next != NULL){
                cur = cur->next;
            }
            cur->next = next;
            cond = 1; // wait for newline again
            i++;
            continue;
        }
        else if((cond==0) && i+6 < head->sz){
            if(data[i] == 'i' && data[i+1] == 'm' && data[i+2] == 'p' && data[i+3] == 'o'\
                && data[i+4] == 'r' && data[i+5] == 't' && data[i+6] == ' '){
                i = i+6;
                cond = 2;
            }
        }
        if(data[i] == '\n')//new line. possible import
            cond = 0;
        if(cond != 2 && data[i] != ' ')// processing non import
            cond = 1;
        i++;
    }
    unloadfile(head);
    return head;
    
}


void unloadfile(Compilationfile* f){
    fclose(f->fp);
    free(f->data);
    f->loaded = 0;
}
/**
populates fields of this file, and loads it into memory.
*/
void loadfile(Compilationfile* f){
    f->fp = fopen(f->path, "r");
    if(f->fp == NULL){
        printf("Error: could not open file \"%s\"", f->path);
    }
    fseek(f->fp, 0L, SEEK_END);
    f->sz = ftell(f->fp);
    rewind(f->fp);
    f->data = (char*) malloc(f->sz);
    if(f->data == NULL){
        printf("Error: could not allocate %ld bytes of memory\n", f->sz);
        exit(1);
    }
    fread(f->data, 1, f->sz, f->fp);
    f->data[f->sz] = 0;
    f->loaded = 1;
}
