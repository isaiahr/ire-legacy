#include<stddef.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include"lexer.h"

/**
 *
 *  Lexer. Performs lexegraphical analysis on the input string
 * 
 * 
 */
#define LEXERROR -1
#define LEFT_PAREN 1
#define RIGHT_PAREN 2
#define LEFT_SQPAREN 3
#define RIGHT_SQPAREN 4
#define LEFT_CRPAREN 5
#define RIGHT_CRPAREN 6
#define INTEGER 7
#define LCHAR 8
#define LSTRING 9
#define IDENTIFIER 10
#define TERM 11
#define MINUS_SYM 12
#define COMMA 13
#define EQUALS 14
#define LEOF 15
Lextoken* lex(char* input){
    char** i = &input;
    int line = 1;
    Lextoken* cur = lexone(i, &line);
    Lextoken* first = cur;
    cur->prev = NULL;
    while(input[0] != 0){
        cur->next = lexone(i, &line);
        cur->next->prev = cur;
        cur = cur->next;
    }
    Lextoken* final = malloc(sizeof(struct Lextoken));
    final->line = line;
    final->next = NULL;
    final->type = LEOF;
    final->str = NULL;
    cur->next = final;
    cur = first;
    printf("Lexer symbol stream\n");
    while(cur->next != NULL){
        char* str;
        switch(cur->type){
            case LEXERROR: str = "ERROR"; break;
            case LEFT_PAREN: str = "LEFT_PAREN"; break;
            case RIGHT_PAREN: str = "RIGHT_PAREN"; break;
            case LEFT_SQPAREN: str = "LEFT_SQPAREN"; break;
            case RIGHT_SQPAREN: str = "RIGHT_SQPAREN"; break;
            case LEFT_CRPAREN: str = "LEFT_CRPAREN"; break;
            case RIGHT_CRPAREN: str = "RIGHT_CRPAREN"; break;
            case INTEGER: str = "INTEGER"; break;
            case LCHAR: str = "CHAR"; break;
            case LSTRING: str = "STRING"; break;
            case IDENTIFIER: str = "IDENTIFIER"; break;
            case RETURN: str = "RETURN"; break;
            case TERM: str = "TERM"; break;
            case MINUS_SYM: str = "MINUS_SYM"; break;
            case COMMA: str = "COMMA"; break;
            case EQUALS: str = "EQUALS"; break;
            case LEOF: str = "EOF"; break;
            default: str = "???"; break;
        }
        if(cur->str){
            printf("%s [%s], ", str, cur->str);
        }
        else{
            printf("%s, ", str);
        }
        cur = cur->next;
    }
    printf("\n");
    return first;
}


Lextoken* lexone(char** i, int* line){
    char* input = *i;
    if(input[0] == ' '){
        (*i)++;
        return lexone(i, line);
    }
    Lextoken* l = malloc(sizeof(struct Lextoken));
    l->line = *line;
    l->next = NULL;
    l->type = LEXERROR;
    l->str = NULL;
    int match = 1;
    switch(input[0]){
        case '(': l->type = LEFT_PAREN; break;
        case ')': l->type = RIGHT_PAREN; break;
        case '[': l->type = LEFT_SQPAREN; break;
        case ']': l->type = RIGHT_SQPAREN; break;
        case '{': l->type = LEFT_CRPAREN; break;
        case '}': l->type = RIGHT_CRPAREN; break;
        case '-': l->type = MINUS_SYM; break;
        case ',': l->type = COMMA; break;
        case '=': l->type = EQUALS; break;
        case ';': l->type = TERM; break;
        case '\n': l->type = TERM; (*line)++; break;
        default: match = 0; break;
    }
    if(match){
        (*i) ++ ; // advance one char
        return l;
    }
    if(ISNUMERIC(input[0])){
        l->lnt = 0;
        l->type = INTEGER;
        int j = 0;
        while(ISNUMERIC(input[j])){
            l->lnt = (l->lnt * 10) + digit(input[j]);
            j++;
        }
        (*i) += j;
        return l;
    }
    if(input[0] == '\''){
        l->type = LCHAR;
        if(input[1] == '\\'){
            if(input[2] == 'n'){
                l->chr = '\n';
            }
            else if(input[2] == '\\'){
                l->chr = '\\';
            }
            if(input[3] != '\''){
                l->type = LEXERROR;
            }
            (*i) += 3;
        }
        else {
            l->chr = input[1];
            if(input[2] != '\''){
                l->type = LEXERROR;
            }
            (*i) += 2;
        }
        return l;
    }
    if(beginswith("import ", input)){
        int z = 0;
        while(input[z] != 0 && input[z] != '\n' && input[z] != ';'){
            z += 1;
        }
        (*i) += z;
        return lexone(i, line);
    }
    if(beginswith("return ", input)){
        (*i) += strlen("return ");
        l->type = RETURN;
        return l;
    }
    if(beginswith("//", input)){
        int z = 0;
        while(input[z] != 0 && input[z] != '\n'){
            z += 1;
        }
        (*i) += z;
        return lexone(i, line);
    }
    if(ISALPHA(input[0]) || input[0] == '_'){
        l->type = IDENTIFIER;
        int j = 1;
        l->str = malloc(2);
        l->str[0] = input[0];
        l->str[1] = 0;
        while(ISALPHA(input[j]) || ISNUMERIC(input[j]) || input[j] == '_'){
            l->str = realloc(l->str, j+2);
            l->str[j] = input[j];
            l->str[j+1] = 0;
            j += 1;
        }
        (*i) += strlen(l->str);
        return l;
    }
    if(input[0] == '\"'){
        l->type = LSTRING;
        l->str = proc_str(&input[1]);
        if(l->str == NULL){
            l->type = LEXERROR;
            return l;
        }
        (*i) += strlen(l->str)+2;
        return l;
    }
    l->type = LEXERROR;

    // skip to end of token
    int z = 1;
    while(input[z] != ' ' && input[z] != ';' && input[z] != '\n' && input[z] != 0){
        z += 1;
    }
    (*i) += z;
    return l;
}

int digit(char input){
    switch(input){
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        default: return -1; 
    }
}

char* proc_str(char* data){
    int esc_next = 0;
    int stringalloc = 32;
    int stringind = 0;
    char* string = (char*) malloc(stringalloc);
    for(long i=0; data[i] != 0; i++){ 
        char c = data[i];
        if(c == '"' && (esc_next == 0)){
            string[stringind] = 0;
            esc_next = 0;
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
        else {
            if(c == '\\' && esc_next == 0){
                esc_next = 1;
            }
            else {
                if(esc_next == 1){
                    if(c == 'n')
                        string[stringind] = '\n';
                }
                else{
                    string[stringind] = c;
                }
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

int beginswith(char* begin, char* token){

    int matches = 1;
    int j = 0;
    while(matches && token[j] != 0){
        matches = matches && (begin[j] == token[j]);
        j += 1;
        if(begin[j] == 0)
            return matches;
    }
    return 0;
}