#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

#define BADUSAGE 1
#define NOFILE -1
#define ISALPHA(x) ((x > 64 && x < 91) || (x > 96 && x < 123))
#define ISNUMERIC(x) (x > 47 && x < 58) 

//token types
#define ASSIGNMENT 2
#define FUNCTION_CALL 3
#define FUNCTION_DEFN 4
#define FUNCTION_RETURN 8
#define FUNCTION_END 7
#define CONDITIONAL 5
#define ASM 6
#define INVALID -1
extern int compile(long sz, char* data);
// Data structures

//linked list
typedef struct List{
    void* data;
    struct List* next;
} List;
//compiler state
typedef struct State{
    List* variables;
    List* functions;
    FILE* fp;
} State;
extern int matches(char* big, char* small);
extern char* proc_str(char* data, long max, int* index);
extern char* integer(int i);
extern void process_token(char* token, State* state);
extern void write_funcall(char* func, State* state);
extern void write_funcdef(char* func, State* state);
extern void write_funcreturn(State* state);
extern void write_asm(char* str, State* state);
extern void write_varref(char* var, State* state);
extern void write_header(State* state);
extern void write_footer(State* state);
extern int get_token_type(char* token);


int main(int argc, char **argv)
{
   if(argc != 2){
      printf("usage: irec file\n");
      return BADUSAGE;
   }
   char* filename = argv[1];
   printf("compiling %s\n", filename);
   FILE* fp;
   if((fp = fopen(filename, "r")) == NULL){
      return -ENOENT;
   }
   fseek(fp, 0L, SEEK_END);
   long sz = ftell(fp);
   rewind(fp);
   printf("%ld\n", sz);
   char* data;
   if((data = (char*) malloc(sz)) == NULL){
      printf("cannot allocate memory to read file");
      return -ENOMEM;
   }
    fread(data, 1, sz, fp);
    data[sz] = '\0';
    int index0 = -1;
    int index1 = -1;
    int multiline = 0;
   
    FILE* fpo;
    if((fpo = fopen("output.as", "w")) == NULL)
    {
        printf("error writing output");
        return ENOENT;
    }
    State *state = (State*) malloc(sizeof (State));
    state->fp = fpo;
    write_header(state);
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
   return 0;
   /**
   printf("\n%s\n", data);
   printf("%02X%02X %02x%02x", 'x', 'd', 'f', 'p');
   return compile(sz, data);
   */
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
        int i = 0;
        while(token[i] != ' ') i++;
        int j = i+1;
        while(token[j] != ' ' && token[j] != '{') j++;
        char* function = (char*) malloc(j-i);
        memcpy(function, &token[i+1], j-i-1);
        function[j-i-1] = 0;
        write_funcdef(function, state);
    }
    if(type == FUNCTION_RETURN){
        int i = 0;
        while(token[i] != ' ') i++;
        write_varref(&token[i+1], state); 
        write_funcreturn(state);
    }
    if(type == FUNCTION_END){} // TODO 
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

}
