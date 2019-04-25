#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include"datastructs.h"
#include"pre_ll.h"
#include"irec.h"
#include"commitid.h"


void lwrite_header(State* state){
    fprintf(state->fp, "target triple = \"x86_64-pc-linux-gnu\"\n\n\n");
    fprintf(state->fp, "%s\n", pre_ll);
}

void lwrite_footer(State* state){
    fprintf(state->fp, "\n\n\n!llvm.ident = !{!0}\n");
    fprintf(state->fp, "!0 = !{!\"irec version %s build %s\"}\n", VERSION_STRING, COMMIT_ID);
}

void lwrite_varinit(Variable* var, State* state){
    if(var->type->id ==  -VARTYPE_BYTE){
        state->tempnum += 1;
        var->num = state->tempnum;
        fprintf(state->fp, "%%%i = call i8* @alloc(i64 1024)\n", var->num);
    }
    else if(var->type->id == -VARTYPE_INTEGER){
        state->tempnum += 1;
        fprintf(state->fp, "%%%i = call i8* @alloc(i64 1024)\n", state->tempnum);
        state->tempnum += 1;
        var->num = state->tempnum;
        fprintf(state->fp, "%%%i = bitcast i8* %%%i to i64*\n", var->num, state->tempnum-1);
    }
}

void lwrite_funcreturn(State* state){
    fprintf(state->fp, "ret i64 %%%i\n}\n", state->tempnum);
}

void lwrite_funcdef(Function* func, State* state){
    fprintf(state->fp, "define i64 @%s (i64){\n", func->write_name);
    state->tempnum = 1; // param = %0  next = 1; ?? 
}

void lwrite_asm(char* str, State* state){
    state->tempnum += 1;
    fprintf(state->fp, "%%%i = call i64 asm sideeffect \"movq $1, %%rax\n", state->tempnum);
    fprintf(state->fp, "%s\n", str);
    fprintf(state->fp, "\", \"=r,r,~{memory}\" (i64 %%%i)\n", (state->tempnum-1) == 1 ? 0: state->tempnum-1);
}

void lwrite_funcall(Function* func, State* state){
    fprintf(state->fp, "%%%i = call i64 @%s(i64 %%%i)\n", state->tempnum+1, func->write_name, (state->tempnum==1?0:state->tempnum));
    state->tempnum += 1;
}

void lwrite_varassign(Variable* a, State* state){
    // a = b
    // "rename" variable a to b 
    a->num = state->tempnum;
}

void lwrite_varref(Variable* ref, State* state){
    state->tempnum += 1;
    if(ref->type->id != VARTYPE_BYTE){
        fprintf(state->fp, "%%%i = add i64 0, %%%i\n", state->tempnum, ref->num);
    } else if(ref->type->id == VARTYPE_BYTE){
        fprintf(state->fp, "%%%i = add i8 0, %%%i\n", state->tempnum, ref->num);
    }
    else{
        fprintf(stderr, "Error computing types while compiling.\nExiting.\n");
        exit(-1);
    }
}

void lwrite_byte(char byte, State* state){
    unsigned char b = byte;
    state->tempnum += 1;
    fprintf(state->fp, "%%%i = add i8 0, 0x%x\n", state->tempnum, b);
}

void lwrite_int(int immediate, State* state){
    state->tempnum += 1;
    fprintf(state->fp, "%%%i = add i64 0, %i\n", state->tempnum, immediate);
}

void lwrite_arrset(Variable* arr, Variable* ind, State* state){
    int val = state->tempnum;
    if(arr->type->id == -VARTYPE_BYTE){
        state->tempnum += 1;
        fprintf(state->fp, "%%%i = add i64 %%%i, 8\n", state->tempnum, ind->num);
        fprintf(state->fp, "%%%i = getelementptr inbounds i8, i8* %%%i, i64 %%%i\n", state->tempnum+1, arr->num, state->tempnum); // location
        state->tempnum += 1;
        fprintf(state->fp, "store i8 %%%i, i8* %%%i\n", val, state->tempnum);
    }else{
        state->tempnum += 1;
        fprintf(state->fp, "%%%i = add i64 %%%i, 8\n", state->tempnum, ind->num);
        fprintf(state->fp, "%%%i = getelementptr inbounds i64, i64* %%%i, i64 %%%i\n", state->tempnum+1, arr->num, state->tempnum); // location
        state->tempnum += 1;
        fprintf(state->fp, "store i64 %%%i, i64* %%%i\n", val, state->tempnum);
    }
}

void lwrite_arradd(Variable* arr, State* state){
    if(arr->type->id == -VARTYPE_BYTE){
        fprintf(state->fp, "call i8* array_addb(i8* %%%i, i8 %%%i)\n", arr->num, state->tempnum);
    }else{
        fprintf(state->fp, "call i64* array_add(i64* %%%i, i64 %%%i)\n", arr->num, state->tempnum);
    }
}

void lwrite_arrind(Variable* arr, State* state){
    if(arr->type->id == -VARTYPE_BYTE){
        state->tempnum += 1;
        fprintf(state->fp, "%%%i = add i64 %%%i, 8\n", state->tempnum, state->tempnum-1);
        fprintf(state->fp, "%%%i = getelementptr inbounds i8, i8* %%%i, i64 %%%i\n", state->tempnum+1, arr->num, state->tempnum); // location
        state->tempnum += 1;
        fprintf(state->fp, "%%%i = load i8 i8* %%%i\n", state->tempnum+1, state->tempnum);
        state->tempnum += 1;
    }
    else{
        state->tempnum += 1;
        fprintf(state->fp, "%%%i = add i64 %%%i, 8\n", state->tempnum, state->tempnum-1);
        fprintf(state->fp, "%%%i = getelementptr inbounds i64, i64* %%%i, i64 %%%i\n", state->tempnum+1, arr->num, state->tempnum); // location
        state->tempnum += 1;
        fprintf(state->fp, "%%%i = load i64 i64* %%%i\n", state->tempnum+1, state->tempnum);
        state->tempnum += 1;
    }
}

void lwrite_string(char* str, int len, State* state){
    int sz = ((len / 128) * 128) + 128;
    // allocated space is len < n * 128 where n is minimized
    fprintf(state->fp, "%%%i = call i8* alloc(i64 %%%i)\n", state->tempnum+1, sz);
    state->tempnum += 1;
    int orig = state->tempnum;
    fprintf(state->fp, "%%%i = bitcast i8* %%%i to i64*\n", state->tempnum+1, state->tempnum);
    state->tempnum += 1;
    fprintf(state->fp, "store i64 %i, i64* %%%i\n", len, state->tempnum); // write len
    for(int i = 0; i < len; i += 1){
        state->tempnum += 1;
        fprintf(state->fp, "%%%i = getelementptr inbounds i8, i8* %%%i, i64 %%%i\n", state->tempnum, orig, i+1);
        fprintf(state->fp, "store i8 0x%02X, i8* %%%i\n", str[i], state->tempnum);
    }
    
}
