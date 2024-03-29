#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<getopt.h>
#include<sys/types.h>
#include<time.h>
#include"irec.h"
#include"core/common.h"
#include"parser/lexer.h"
#include"parser/parser.h"
#include"core/compiler.h"
#include"codegen/writer.h"
#include"precompiler/precompiler.h"
#include"core/error.h"
#include"build/commitid.h"
#include"core/invoker.h"

int main(int argc, char **argv)
{
    struct option options[] = {
    {"asm", 0, NULL, 'a'},
    {"output", 0, NULL, 'o'},
    {"version", 0, NULL, 'v'},
    {"dumptrees", 0, NULL, 'd'},
    {"annotate", 0, NULL, 'n'},
    {"Optimize", 0, NULL, 'O'},
    {"help", 0, NULL , 'h'},
    {"backend", 0, NULL, 'b'},
    };
    State *state = (State*) malloc(sizeof (State));
    state->comp_asm = 0;
    state->outputfile = NULL;
    state->verbose = 0;
    state->annotate = 0;
    state->llvm = 1;
    state->optimization = 0;
    state->comp_llvm = 0;
    state->lblcount = 0;
    state->writ_return = 0;
    state->errors = NULL;
    char c = 0;
    int ind = 0;
    while((c = getopt_long(argc, argv, "ao:vdnb:O:hl", options, &ind)) != -1){
        switch(c){
            case 'a':
                state->comp_asm = 1;
                break;
            case 'o':
                state->outputfile = optarg;
                break;
            case 'v':
                printf("irec version %s, build %s\n", VERSION_STRING, COMMIT_ID);
                return 0;
            case 'd':
                state->verbose = 1;
                break;
            case 'n':
                state->annotate = 1;
                break;
            case 'O':
                if(strcmp(optarg, "0") == 0){
                    state->optimization = 0;
                }
                else if(strcmp(optarg, "1") == 0){
                    state->optimization = 1;
                }
                else if(strcmp(optarg, "2") == 0){
                    state->optimization = 2;
                }
                else if(strcmp(optarg, "3") == 0){
                    state->optimization = 3;
                }
                else{
                    printf("Invalid optimization level \"%s\". Possible choices are 0, 1, 2, 3\n", optarg);
                    return 1;
                }
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
                printf("-d, --dumptrees           Print lexer token stream, parse tree, and abstract syntax tree\n");
                printf("-b, --backend=<llvm/asm>  Use the specified backend for compilation (default:llvm)\n");             
                printf("-n, --annotate            Annotate generated output\n");
                printf("-h, --help                Display this information\n");
                printf("-v, --version             Display compiler version\n");
                printf("-l, --llvm                Output llvm IR and stop\n");
                printf("-O, --Optimize=<0,1,2,3>  Use optimization level (default:0)\n");
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
    if(state->optimization != 0 && state->llvm == 0){
        printf("Optimization only supported when using llvm backend.\n");
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
    Compilationfile* cur = precompile(filename);
    Token* all = NULL;
    while(cur != NULL){
        loadfile(cur);
        printf("Compiling %s, %ld bytes\n", cur->path, cur->sz);
        Token* t = parsefile(state, cur->data);
        all = join(t, all);
        unloadfile(cur);
        cur = cur->next;
    }
    char* source;
    if((state->comp_asm && !state->llvm) || (state->comp_llvm && state->optimization == 0)){
        // case where output writes directly to -o file.
        if((state->fp = fopen(state->outputfile, "w")) == NULL){
            fprintf(stderr, "error writing output\n");
            return -1;
        }
    }
    else{
        source = get_tempfile(-1);
        if((state->fp = fopen(source, "w")) == NULL){
            fprintf(stderr, "error writing output\n");
            return -1;
        }
    }
    write_header(state);
    compile(state, all);
    write_footer(state);
    fclose(state->fp);
    srand(time(NULL));
    if((state->comp_asm && !state->llvm) || (state->comp_llvm && state->optimization == 0)){
        // exit.
    }
    else if(state->llvm){
        // llvm: irec -> [opt] -> llc -> as -> ld
        if(state->comp_llvm){
            invoke_opt(source, state->outputfile, state->optimization);
        }
        else if(state->comp_asm){
            if(state->optimization == 0){
                invoke_llc(source, state->outputfile);
            }
            else{
                char* t1 = get_tempfile(OPT);
                invoke_opt(source, t1, state->optimization);
                invoke_llc(t1, state->outputfile);
            }
        }
        else{
            if(state->optimization == 0){
                char* t2 = get_tempfile(LLC);
                invoke_llc(source, t2);
                char* t3 = get_tempfile(ASM);
                invoke_assembler(t2, t3);
                invoke_linker(t3, state->outputfile);
            }
            else{
                char* t1 = get_tempfile(OPT);
                invoke_opt(source, t1, state->optimization);
                char* t2 = get_tempfile(LLC);
                invoke_llc(t1, t2);
                char* t3 = get_tempfile(ASM);
                invoke_assembler(t2, t3);
                invoke_linker(t3, state->outputfile);
            }
        }
        
    }
    else{
        // asm: irec -> as -> ld
        char* t1 = get_tempfile(ASM);
        invoke_assembler(source, t1);
        invoke_linker(t1, state->outputfile);
    }
    printf("Done. => %s\n", state->outputfile);
    return 0;
}
