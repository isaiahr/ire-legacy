#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<getopt.h>
#include<sys/types.h>
#include<sys/wait.h>
#include"irec.h"
#include"datastructs.h"
#include"compiler.h"
#include"writer.h"

int main(int argc, char **argv)
{
    struct option options[] = {
    {"asm", 0, NULL, 'a'},
    {"output", 0, NULL, 'o'},
    {"verbose", 0, NULL, 'v'},
    {"annotate", 0, NULL, 'n'},
    };
    State *state = (State*) malloc(sizeof (State));
    state->comp_asm = 0;
    state->outputfile = NULL;
    state->functions = NULL;
    state->variables = NULL;
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
            case '?':
                printf("Try %s --help for usage.", argv[0]);
                return 1;
        }
    }
    char* filename = argv[optind];
    printf("compiling %s, ", filename);
    FILE* fp;
    if((fp = fopen(filename, "r")) == NULL){
       return -ENOENT;
    }
    fseek(fp, 0L, SEEK_END);
    long sz = ftell(fp);
    rewind(fp);
    printf("%ld bytes\n", sz);
    char* data;
    if((data = (char*) malloc(sz)) == NULL){
        printf("cannot allocate memory to read file");
        return -ENOMEM;
    }
    fread(data, 1, sz, fp);
    data[sz] = '\0';
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
            printf("error writing to temp");
            return 0;
        }
        
    }
    else if((fpo = fopen(state->outputfile, "w")) == NULL)
    {
        printf("error writing output");
        return ENOENT;
    }
    state->fp = fpo;
    compile(state, data, sz);
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
/*
void compile(State* state, char* data, long sz){
    write_header(state);
    int index0 = -1;
    int index1 = -1;
    int multiline = 0;
    for(int i = 0; i < sz; i++){
        if(data[i] == '`'){
            if(index1 == -1){
                index1 = i;
                multiline = 1;
            }
            else{
                char* token = malloc(i-index1);
                memcpy(token, &data[index1], i-index1-1);
                token[i-index1-1] = 0;
                process_token(token, state);
                multiline = 0;
                index1 = -1;
            }
            index0 = -1;
        }
        if(data[i] == '\n' && !multiline){
            if(index0 == -1){
                index0 = i+1;
            }
            else{
                char* token = malloc(i-index0+1);
                memcpy(token, &data[index0], i-index0);
                token[i-index0] = 0;
                process_token(token, state);
                index0 = i+1;
            }    
        }
    }
}

// example input "((()))"
int match_paren(char* input){
    int numpar = 1;
    int i = 1;
    while(numpar != 0){
        if(input[i] == '(') numpar++;
        if(input[i] == ')') numpar--;
        i++;
    }
    return i;
}


void process_token(char* token, State* state){
    if(token[0] == ' '){
        return process_token(token + sizeof(char), state);
    }
    int type = get_token_type(token);
    printf("token %i, %c \n", type, token[0]);
    if(type == FUNCTION_CALL){
        int i = 0;
        while(token[i] != '(') i++;
        int j = match_paren(&token[i]);
        char* new_token = (char*) malloc(j-1);
        memcpy(new_token, &token[i+1], j-2);
        new_token[j-1] = 0;
        printf("inner: %s\n", new_token);
        if(get_token_type(new_token) != INVALID){
            process_token(new_token, state);
        }
        else{
            write_varref(new_token, state);
        }
        write_funcall(token, state);
    }
    if(type == ASM){
        write_asm(token + sizeof(char), state);
    }
    if(type == FUNCTION_DEFN){
        // example: def func { } 
        int i = nextnonwhite(token);
        int j = i+1;
        while(token[j] != ' ' && token[j] != '{') j++;
        char* function = (char*) malloc(j-i+1);
        memcpy(function, &token[i], j-i);
        function[j-i] = 0;
        write_funcdef(function, state);
    }
    if(type == FUNCTION_RETURN){
        int i = nextnonwhite(token);
        write_varref(&token[i], state); 
        write_funcreturn(state);
    }
    if(type == FUNCTION_END){} // TODO 
}
// returns index of first whitespace.
int nextwhite(char* str){
    int i = 0;
    while(str[i] != ' ')i++;
    return i;
}
//returns index following next whitespace gap.
int nextnonwhite(char* str){
    int i = nextwhite(str);
    while(str[i] == ' ')i++;
    return i;
}

void write_header(State* state){
    fprintf(state->fp, ".global _start\n.text\n"); 
}

void write_varref(char* ref, State* state){
    if(ISNUMERIC(ref[0])){
        //move immediate number to eax.
        fprintf(state->fp, "movq $%s, %%rax\n", ref);
    }
    else{} //todo support referencing vars.
}

void write_funcreturn(State* state){
    fprintf(state->fp, "%s\n", "ret");
}

void write_funcdef(char* func, State* state){
    fprintf(state->fp, "%s:\n", func);
}
void write_asm(char* str, State* state){
    fprintf(state->fp, "%s\n", str);
}

void write_funcall(char* token, State* state){
    int i = 0;
    while(token[i] != '(') i++;
    char* func = malloc(i+1);
    memcpy(func, token, i);
    func[i] = 0;
    fprintf(state->fp, "call %s\n", func);
}


int get_token_type(char* token){// TODO ADD TYPE=CONDITIONAL
    int i = 0;
    if(token[0] == '`'){
        return ASM;
    }
    if(token[0] == '}'){
        return FUNCTION_END;
    }
    if(!ISALPHA(token[0])){
        return INVALID; // invalid char
    }
    if(token[0] == 'r' && token[1] == 'e' && token[2] == 't' && token[3] == 'u'
    && token[4] == 'r' && token[5] == 'n') return FUNCTION_RETURN;
    if(token[0] == 'd' && token[1] == 'e' && token[2] == 'f' && token[3] == ' '){
        return FUNCTION_DEFN;
    }
    while(token[i] != '\0'){
        if(token[i] == '='){
            return ASSIGNMENT;
        }
        if(token[i] == '('){
            return FUNCTION_CALL;
        }
        i += 1;
    }
    return INVALID;
}



char* proc_str(char* data, long max, int* indexed){

   int esc_next = 0;
   int stringalloc = 32;
   int stringind = 0;
   char* string = (char*) malloc(stringalloc);
   for(long i=0; i < max; i++){
      
      char c = data[i];
      if(c == '"' && (esc_next == 0)){
        string[stringind] = '\0';
        esc_next = 0;
        (*indexed) = i;
        return string;
      }
      else if(c == '"' && (esc_next == 1)){
        string[stringind] = c;
        stringind = stringind + 1;
        if(stringind == stringalloc){//allocate more space
             stringalloc = stringalloc * 2;
             string = (char*) realloc(string, stringalloc);
        }
        esc_next = 0;
      }
      else{
         if(c == '\\' && esc_next == 0){
             esc_next = 1;
         }
         else {
            string[stringind] = c;
            stringind=stringind+1;
            if(stringind == stringalloc){
                stringalloc = stringalloc * 2;
                string = (char*) realloc(string, stringalloc);
            }
            esc_next = 0;
         }
      }
      
   }
   return NULL;

}*/
