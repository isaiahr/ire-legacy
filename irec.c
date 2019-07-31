#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<getopt.h>
#include<sys/types.h>
#include<sys/wait.h>
#include"irec.h"
#include"datastructs.h"
#include"lexer.h"
#include"parser.h"
#include"compiler.h"
#include"writer.h"
#include"precompiler.h"
#include"error.h"
#include"commitid.h"

int main(int argc, char **argv)
{
    struct option options[] = {
    {"asm", 0, NULL, 'a'},
    {"output", 0, NULL, 'o'},
    {"verbose", 0, NULL, 'v'},
    {"annotate", 0, NULL, 'n'},
    {"help", 0, NULL , 'h'},
    {"backend", 0, NULL, 'b'},
    };
    State *state = (State*) malloc(sizeof (State));
    state->comp_asm = 0;
    state->outputfile = NULL;
    state->verbose = 0;
    state->annotate = 0;
    state->llvm = 1;
    state->comp_llvm = 0;
    state->lblcount = 0;
    state->writ_return = 0;
    state->errors = NULL;
    char c = 0;
    int ind = 0;
    while((c = getopt_long(argc, argv, "ao:vnb:hl", options, &ind)) != -1){
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
            case 'l':
                state->comp_llvm = 1;
                break;
            case 'b':
                if(strcmp(optarg, "llvm") == 0){
                    state->llvm = 1;
                }
                else if(strcmp(optarg, "asm") != 0){
                    printf("Invalid backend \"%s\". Possible choices are \"asm\" or \"llvm\"", optarg);
                    return 1;
                }
                else{
                    state->llvm = 0;
                }
                break;
            case 'h':
                printf("irec, the ire compiler\n");
                printf("Version %s, build %s\n", VERSION_STRING, COMMIT_ID);
                printf("Usage: %s [options] file\n", argv[0]);
                printf("Options:\n");
                printf("-a, --asm,                Compile, but do not assemble & link\n");
                printf("-o, --output=<outfile>    Compile to file outfile\n");
                printf("-v, --verbose             Print more info on what irec is doing\n");
                printf("-b, --backend=<llvm/asm>  Use the specified backend for compilation (default:llvm)\n");             
                printf("-n, --annotate            Annotate generated output\n");
                printf("-h, --help                Display this information\n");
                printf("-l, --llvm                Output llvm IR and stop\n");
                printf("irec is distributed under the LGPL v3 License.\n");
                printf("See LICENSE for details. If you do not have the source code\n");
                printf("for this application, you may acquire it at github.com/isaiahr/ire\n");
                return 0;
            case '?':
                printf("Try %s --help for usage.\n", argv[0]);
                return 0;
        }
    }
    // check options are ok
    if(state->comp_llvm && state->comp_asm){
        printf("Incompatible options -l, -a\n");
        return 1;
    }
    if(state->comp_llvm && state->llvm == 0){
        printf("Set backend=llvm to output llvm IR\n");
        return 1;
    }
    char* filename = argv[optind];
    if(filename == NULL){
        printf("No file given.\n");
        printf("Try %s --help for usage.\n", argv[0]);
        return 0;
    }
    if(state->outputfile == NULL){
        char* indx = strchr(filename, '.');
        if(indx == NULL)
            indx = strchr(filename, 0);
        char* outputfile = (char*) malloc(indx-filename+4);
        memcpy(outputfile, filename, (indx-filename)+1);
        char* ind01 = strchr(outputfile, '.');
        if(state->comp_asm){
            if(ind01 == NULL){
                ind01 = strchr(outputfile, 0);
                ind01[0] = '.';
            }
            ind01[1] = 's';
            ind01[2] = 0;
        }
        else if(state->comp_llvm){
            if(ind01 == NULL){
                ind01 = strchr(outputfile, 0);
                ind01[0] = '.';
            }
            ind01[1] = 'l';
            ind01[2] = 'l';
            ind01[3] = 0;
        }
        else{
            if(ind01 == NULL){
                ind01 = strchr(outputfile, 0);
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
    if(!((state->comp_asm && !state->llvm) || state->comp_llvm)){
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
    Token* all = NULL;
    while(cur != NULL){
        loadfile(cur);
        printf("Compiling %s, %ld bytes\n", cur->path, cur->sz);
        Token* t = parsefile(state, cur->data);
        all = join(t, all);
        unloadfile(cur);
        cur = cur->next;
    }
    compile(state, all);
    write_footer(state);
    fclose(state->fp);
    if(state->comp_llvm){
        printf("Done compilation.\n");
        return 0;
    }
    if(state->comp_asm && !state->llvm){
        printf("Done compilation.\n");
    }
    if(state->llvm){
        //invoke llc
        printf("Done compilation. Generating asm...\n");
        char* t = NULL;
        if(state->comp_asm){
            t = state->outputfile;
        }
        else{
            t = tempnam(NULL, "irecv");
        }
        int i = fork();
        if(i == 0){
            execl("/usr/bin/llc", "/usr/bin/llc", compto, "-o", t, (char*) NULL);
            return 0;
        }
        int st;
        wait(&st);
        compto = t;
    }
    if(!state->comp_asm){
        printf("Assembling...\n");
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
}
