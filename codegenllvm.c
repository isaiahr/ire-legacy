#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include"datastructs.h"
#include"pre_ll.h"
#include"irec.h"
#include"commitid.h"
#include"semantic.h"


void lwrite_header(State* state){
    fprintf(state->fp, "target triple = \"x86_64-pc-linux-gnu\"\n\n\n");
    fprintf(state->fp, "%s\n", pre_ll);
}

void lwrite_footer(State* state){
    fprintf(state->fp, "\n\n\n!llvm.ident = !{!0}\n");
    fprintf(state->fp, "!0 = !{!\"irec version %s build %s\"}\n", VERSION_STRING, COMMIT_ID);
}

void lwrite_varinit(Variable* var, State* state){
    // nop
}

void lwrite_funcreturn(Function* func, Variable* var, State* state){
    fprintf(state->fp, "ret %s %%%i\n}\n", var->type->llvm, var->num);
    func->writ_return = 1;
}
void lwrite_funcend(Function* func, State* state){
    if(!func->writ_return){
        fprintf(state->fp, "ret %s 0\n}\n", func->retval->llvm);
    }
}
void lwrite_funcdef(Function* func, State* state){
    fprintf(state->fp, "define %s @%s (", func->retval->llvm, func->write_name);
    VarList* cur = func->params;
    state->tempnum = 0;
    while(cur != NULL){
        cur->var->num = state->tempnum;
        state->tempnum += 1;
        if(cur == func->params){
            fprintf(state->fp, "%s", cur->var->type->llvm);   
        }
        else{
            fprintf(state->fp, ", %s", cur->var->type->llvm);
        }
        cur = cur->next;
    }
    fprintf(state->fp, "){\n");
    // for inserted block space
    state->tempnum += 1;
}

void lwrite_funcall(FunctionCall* func, State* state){
    int begin = -34234;
    if(func->func->native){
        // convert all numbers to i64 to deal with syscall
        begin = state->tempnum;
        VarList* cur0 = func->vars;
        int i = 0;
        while(cur0 != NULL){
            if(strcmp(cur0->var->type->identifier, "Byte[]") == 0 || strcmp(cur0->var->type->identifier, "Int[]") == 0){
                fprintf(state->fp, "%%%i = ptrtoint %s %%%i to i64\n", begin+i, cur0->var->type->llvm, cur0->var->num);
            }
            else if(strcmp(cur0->var->type->identifier, "Byte") == 0){
                fprintf(state->fp, "%%%i = sext i8 %%%i to i64\n", begin+i, cur0->var->num);
            }
            else{
                fprintf(state->fp, "%%%i = add i64 0, %%%i\n", begin+i, cur0->var->num);
            }
            cur0 = cur0->next;
            i++;
            state->tempnum += 1;
        }
        state->tempnum -= 1;
    }
    if(func->to != NULL){
        func->to->num = state->tempnum;
        state->tempnum += 1;
        fprintf(state->fp, "%%%i = call %s @%s(", func->to->num, func->func->retval->llvm, func->func->write_name);
    }
    else{
        state->tempnum += 1;
        fprintf(state->fp, "%%%i = call %s @%s(", state->tempnum, func->func->retval->llvm, func->func->write_name);
    }
    VarList* cur = func->vars;
    while(cur != NULL){
        if(func->func->native){
            if(cur == func->vars){
                fprintf(state->fp, "i64 %%%i", begin);
            }
            else{
                fprintf(state->fp, ", i64 %%%i", begin);
            }
            begin += 1;
            cur = cur->next;
            continue;
        }
        if(cur == func->vars){
            fprintf(state->fp, "%s %%%i", cur->var->type->llvm, cur->var->num);
        }
        else{
            fprintf(state->fp, ", %s %%%i", cur->var->type->llvm, cur->var->num);
        }
        cur = cur->next;
    }
    if(func->func->native){
        state->tempnum = begin+1;
    }
    fprintf(state->fp, ")\n");
}

void lwrite_varassign(Variable* to, Variable* from, State* state){
    // "rename" variable a to b
    to->num = from->num;
}


void lwrite_byte(Variable* to, char byte, State* state){
    unsigned char b = byte;
    to->num = state->tempnum;
    state->tempnum += 1;
    fprintf(state->fp, "%%%i = add i8 0, 0x%x\n", to->num, b);
}

void lwrite_int(Variable* to, int immediate, State* state){
    to->num = state->tempnum;
    state->tempnum += 1;
    fprintf(state->fp, "%%%i = add i64 0, %i\n", to->num, immediate);
}

void lwrite_indget(Variable* arr, Variable* ind, Variable* to, State* state){
    if((strcmp(arr->type->identifier, "Byte[]") == 0)){
        fprintf(state->fp, "%%%i = getelementptr inbounds i8, i8* %%%i, %s %%%i\n", state->tempnum, arr->num, ind->type->llvm, ind->num);
        fprintf(state->fp, "%%%i = load i8, i8* %%%i\n", state->tempnum + 1, state->tempnum);
        state->tempnum += 1;
        to->num = state->tempnum;
        state->tempnum += 1;
    }
    else {
        fprintf(state->fp, "%%%i = getelementptr inbounds i64, i64* %%%i, %s %%%i\n", state->tempnum, arr->num, ind->type->llvm, ind->num);
        fprintf(state->fp, "%%%i = load i64, i64* %%%i\n", state->tempnum + 1, state->tempnum);
        state->tempnum += 1;
        to->num = state->tempnum;
        state->tempnum += 1;
    }
}

void lwrite_indset(Variable* arr, Variable* ind, Variable* from, State* state){
    if((strcmp(arr->type->identifier, "Byte[]") == 0)){
        fprintf(state->fp, "%%%i = getelementptr inbounds i8, i8* %%%i, %s %%%i\n", state->tempnum, arr->num, ind->type->llvm, ind->num);
        fprintf(state->fp, "store i8 %%%i, i8* %%%i\n", from->num, state->tempnum);
        state->tempnum += 1;
    }
    else {
        fprintf(state->fp, "%%%i = getelementptr inbounds i64, i64* %%%i, %s %%%i\n", state->tempnum, arr->num, ind->type->llvm, ind->num);
        fprintf(state->fp, "store i64 %%%i, i64* %%%i\n", from->num, state->tempnum);
        state->tempnum += 1;
    }
}

void lwrite_addeq(Variable* arr, Variable* delta, State* state){
    if((strcmp(arr->type->identifier, "Byte[]") == 0)){
        fprintf(state->fp, "call i8* @array_addb(i8* %%%i, i8 %%%i)\n", arr->num, delta->num);
    }
    else {
        fprintf(state->fp, "call i64* @array_add(i64* %%%i, i64 %%%i)\n", arr->num, delta->num);
    }
    state->tempnum += 1;
}

void lwrite_string(Variable* to, char* str, int len, State* state){
    int sz = ((len / 128) * 128) + 128;
    // allocated space is len < n * 128 where n is minimized
    fprintf(state->fp, "%%%i = call i8* @alloc(i64 %i)\n", state->tempnum, sz);
    fprintf(state->fp, "%%%i = bitcast i8* %%%i to i64*\n", state->tempnum+1, state->tempnum);
    int arrloc = state->tempnum;
    state->tempnum += 1;
    fprintf(state->fp, "store i64 %i, i64* %%%i\n", len, state->tempnum); // write len
    for(int i = 0; i < len; i += 1){
        state->tempnum += 1;
        fprintf(state->fp, "%%%i = getelementptr inbounds i8, i8* %%%i, i64 %i\n", state->tempnum, arrloc, i+8);
        if(i == 0){
            to->num = state->tempnum;
        }
        fprintf(state->fp, "store i8 %i, i8* %%%i\n", (unsigned int) str[i], state->tempnum);
    }
    state->tempnum += 1;
}

void lwrite_card(Variable* to, Variable* from, State* state){
    if((strcmp(from->type->identifier, "Byte[]") == 0)){
        fprintf(state->fp, "%%%i = bitcast i8* %%%i to i64*\n", state->tempnum, from->num);
        state->tempnum += 1;
        fprintf(state->fp, "%%%i = getelementptr inbounds i64, i64* %%%i, i64 -1\n", state->tempnum, state->tempnum-1);
        to->num = state->tempnum+1;
        fprintf(state->fp, "%%%i = load i64, i64* %%%i\n", to->num, state->tempnum);
        state->tempnum = to->num+1;
    }
    else {
        fprintf(state->fp, "%%%i = getelementptr inbounds i64, i64* %%%i, i64 -1\n", state->tempnum, from->num);
        to->num = state->tempnum + 1;
        fprintf(state->fp, "%%%i = load i64, i64* %%%i\n", to->num, state->tempnum);
        state->tempnum += 2;
    }
}

void lwrite_newarr(Variable* to, Variable* size, State* state){
    if(strcmp(to->type->identifier, "Byte[]") == 0){
        fprintf(state->fp, "%%%i = call i8* @alloc(i64 %%%i)\n", state->tempnum, size->num);
        fprintf(state->fp, "%%%i = bitcast i8* %%%i to i64*\n", state->tempnum+1, state->tempnum);
        state->tempnum += 1;
        fprintf(state->fp, "store i64 %%%i, i64* %%%i\n", size->num, state->tempnum);
        fprintf(state->fp, "%%%i = getelementptr inbounds i64, i64* %%%i, i64 1\n", state->tempnum+1, state->tempnum);
        state->tempnum += 1;
        to->num = state->tempnum+1;
        fprintf(state->fp, "%%%i = bitcast i64* %%%i to i8*\n", to->num, state->tempnum);
        state->tempnum += 2;
    }
    else{
        fprintf(state->fp, "%%%i = shl nuw i64 %%%i, 3\n", state->tempnum, size->num);
        state->tempnum += 1;
        fprintf(state->fp, "%%%i = call i8* @alloc(i64 %%%i)\n", state->tempnum, state->tempnum-1);
        fprintf(state->fp, "%%%i = bitcast i8* %%%i to i64*\n", state->tempnum+1, state->tempnum);
        state->tempnum += 1;
        fprintf(state->fp, "store i64 %%%i, i64* %%%i\n", size->num, state->tempnum);
        to->num = state->tempnum+1;
        fprintf(state->fp, "%%%i = getelementptr inbounds i64, i64* %%%i, i64 1\n", to->num, state->tempnum);
        state->tempnum += 2;
    }
}
