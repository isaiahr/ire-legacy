#ifndef __PRECOMPILER_H__
#define __PRECOMPILER_H__
typedef struct Compilationfile{
    char* data;
    char* path;
    FILE* fp;
    int loaded;
    long sz;
    struct Compilationfile* next;
}Compilationfile;


extern Compilationfile* precompile(char* path);
void unloadfile(Compilationfile* f);
void loadfile(Compilationfile* f);
#endif
