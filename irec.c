#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<getopt.h>
#include<sys/types.h>
#include<sys/wait.h>
#include"irec.h"
#include"datastructs.h"
#include"parser.h"
#include"compiler.h"
#include"writer.h"
#include"precompiler.h"
#include"error.h"

int main(int argc, char **argv)
{
    struct option options[] = {
    {"asm", 0, NULL, 'a'},
    {"output", 0, NULL, 'o'},
    {"verbose", 0, NULL, 'v'},
    {"annotate", 0, NULL, 'n'},
    {"help", 0, NULL , 'h'},
    };
    List* types = (List*) malloc(sizeof (List));
    Type* type_int = (Type*) malloc(sizeof (Type));
    type_int->name = "int";
    type_int->composite = NULL;
    type_int->functions = NULL;
    type_int->operators = NULL;
    type_int->id = VARTYPE_INTEGER;
    Type* type_byte = (Type*) malloc(sizeof (Type));
    type_byte->name = "byte";
    type_byte->composite = NULL;
    type_byte->functions = NULL;
    type_byte->operators = NULL;
    type_byte->id = VARTYPE_BYTE;
    types->data = type_int;
    types->next = (List*) malloc(sizeof (List));
    types->next->data = type_byte;
    types->next->next = NULL;
    State *state = (State*) malloc(sizeof (State));
    state->comp_asm = 0;
    state->outputfile = NULL;
    state->functions = NULL;
    state->variables = NULL;
    state->types = types;
    state->verbose = 0;
    state->annotate = 0;
    state->currentfunc = NULL;
    state->writ_return = 0;
    char c = 0;
    int ind = 0;
    while((c = getopt_long(argc, argv, "ao:vn", options, &ind)) != -1){
        switch(c){
            case 'a':
                state->comp_asm = 1;
                break;
            case 'o':
                state->outputfile = optarg;
                break;
            case 'v':
                state->verbose = 1;
                break;
            case 'n':
                state->annotate = 1;
                break;
            case 'h':
                printf("irec, the ire compiler\n");
                printf("Usage: %s [options] file\n", argv[0]);
                printf("Options:\n");
                printf("-a, --asm,                Compile, but do not assemble & link\n");
                printf("-o, --output=<outfile>    Compile to file outfile\n");
                printf("-v, --verbose             Print more info on what irec is doing\n");
                printf("-n, --annotate            Annotate assembly output\n");
                printf("-h, --help                Display this information\n");
                printf("irec is distributed under the LGPL v3 License.\n");
                printf("See LICENSE for details. If you do not have the source code\n");
                printf("for this application, you may acquire it at github.com/isaiahr/irec\n");
                return 0;
            case '?':
                printf("Try %s --help for usage.\n", argv[0]);
                return 0;
        }
    }
    char* filename = argv[optind];
    if(state->outputfile == NULL){
        char* indx = strchr(filename, '.');
        if(indx == NULL) indx = strchr(filename, 0);
        char* outputfile = (char*) malloc(indx-filename+4);
        memcpy(outputfile, filename, (indx-filename)+1);
        char* ind01 = strchr(outputfile, '.');
        if(state->comp_asm){
            if(ind01 == NULL){
                ind01 = strchr(outputfile, 0);
                ind01[0] = '.';
            }
            ind01[1] = 'a';
            ind01[2] = 's';
            ind01[3] = 0;
        }
        else{
            if(ind01 == NULL){
                ind01[0] = '0';
                ind01[1] = '0';
                ind01[2] = 0;
            }
            else{
                ind01[0] = 0;
            }
        }
        state->outputfile = outputfile;
    }
    FILE* fpo;
    char* compto;
    if(!state->comp_asm){
        compto = tempnam(NULL, "irecc");
        if((fpo = fopen(compto, "w")) == NULL){
            fprintf(stderr, "error writing to temp");
            return -1;
        }
        
    }
    else if((fpo = fopen(state->outputfile, "w")) == NULL)
    {
        fprintf(stderr, "error writing output");
        return -1;
    }
    state->fp = fpo;
    Compilationfile* cur = precompile(filename);
    write_header(state);
    while(cur != NULL){
        loadfile(cur);
        printf("Compiling %s, %ld bytes\n", cur->path, cur->sz);
        compile(state, cur->data, cur->sz);
        unloadfile(cur);
        cur = cur->next;
    }
    // check if state is ok.
    // make sure all functions are defined
    List* head = state->functions;
    while(head != NULL){
        Function* f = (Function*) head->data;
        if(f->defined == 0){
            errornl(UNDEFFUNC, f->name);
        }
        head = head->next;
    }
    write_footer(state);
    fclose(state->fp);
    if(!state->comp_asm){
        printf("Done compilation. Assembling...\n");
        char* t = tempnam(NULL, "ireca");
        int i = fork();
        if(i == 0){
            execl("/usr/bin/as", "/usr/bin/as", compto, "-o", t, (char*) NULL);
            return 0;
        }
        int s0;
        wait(&s0);
        printf("Linking...\n");
        i = fork();
        if(i == 0){
            execl("/usr/bin/ld", "/usr/bin/ld", t, "-o", state->outputfile, (char*) NULL);
            return 0;
        }
        int s1;
        wait(&s1);
        printf("Done. %s\n", state->outputfile);
    }
    return 0;
    /**
    printf("\n%s\n", data);
    printf("%02X%02X %02x%02x", 'x', 'd', 'f', 'p');
    return compile(sz, data);
    */
}
