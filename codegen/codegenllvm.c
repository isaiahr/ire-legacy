#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include"common/common.h"
#include"build/pre_ll.h"
#include"irec.h"
#include"build/commitid.h"
#include"ast/ast.h"


void lwrite_header(State* state){
    fprintf(state->fp, "target triple = \"x86_64-pc-linux-gnu\"\n\n\n");
    fprintf(state->fp, "%s\n", pre_ll);
}

void lwrite_footer(State* state){
    fprintf(state->fp, "\n\n\n!llvm.ident = !{!0}\n");
    fprintf(state->fp, "!0 = !{!\"irec version %s build %s\"}\n", VERSION_STRING, COMMIT_ID);
}

void lwrite_varinit(Variable* var, State* state){
    var->num = state->tempnum;
    state->tempnum += 1;
    fprintf(state->fp, "%%%i = alloca %s\n", var->num, var->type->llvm);
}

void lwrite_funcreturn(Function* func, Variable* var, State* state){
    fprintf(state->fp, "%%%i = load %s, %s* %%%i\n", state->tempnum, var->type->llvm, var->type->llvm, var->num);
    fprintf(state->fp, "ret %s %%%i\n", var->type->llvm, state->tempnum);
    state->tempnum += 2;
    // 2 to allow for basic block after.
}
void lwrite_funcend(Function* func, State* state){
    if(!func->writ_return){
        fprintf(state->fp, "ret %s zeroinitializer\n}\n", func->retval->llvm);
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
    cur = func->params;
    while(cur != NULL){
        fprintf(state->fp, "%%%i = alloca %s\n", state->tempnum, cur->var->type->llvm);
        fprintf(state->fp, "store %s %%%i, %s* %%%i\n", cur->var->type->llvm, cur->var->num, cur->var->type->llvm, state->tempnum);
        cur->var->num = state->tempnum;
        state->tempnum += 1;
        cur = cur->next;
    }
}

void lwrite_funcall(FunctionCall* func, State* state){
    int onum = -12345;
    if(func->to != NULL){
        onum = func->to->num;
    }
    int begin = -34234;
    if(func->func->native){
        // convert all numbers to i64 to deal with syscall
        begin = state->tempnum;
        VarList* cur0 = func->vars;
        int i = 0;
        while(cur0 != NULL){
            fprintf(state->fp, "%%%i = load %s, %s* %%%i\n", state->tempnum, cur0->var->type->llvm, cur0->var->type->llvm, cur0->var->num);
            i++;
            state->tempnum += 1;
            if(strcmp(cur0->var->type->identifier, "Byte[]") == 0 || strcmp(cur0->var->type->identifier, "Int[]") == 0){
                fprintf(state->fp, "%%%i = ptrtoint %s %%%i to i64\n", begin+i, cur0->var->type->llvm, begin+i-1);
            }
            else if(strcmp(cur0->var->type->identifier, "Byte") == 0){
                fprintf(state->fp, "%%%i = sext i8 %%%i to i64\n", begin+i, begin+i-1);
            }
            else{
                // extra instruction for no reason.
                fprintf(state->fp, "%%%i = add i64 0, %%%i\n", begin+i, begin+i-1);
            }
            cur0 = cur0->next;
            i++;
            state->tempnum += 1;
        }
        state->tempnum -= 1; // ???? 
    }
    else{
        begin = state->tempnum;
        VarList* cur0 = func->vars;
        while(cur0 != NULL){
            fprintf(state->fp, "%%%i = load %s, %s* %%%i\n", state->tempnum, cur0->var->type->llvm, cur0->var->type->llvm, cur0->var->num);
            state->tempnum += 1;
            cur0 = cur0->next;
        }
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
    if(func->func->native){
        begin += 1;
    }
    while(cur != NULL){
        if(func->func->native){
            if(cur == func->vars){
                fprintf(state->fp, "i64 %%%i", begin);
            }
            else{
                fprintf(state->fp, ", i64 %%%i", begin);
            }
            begin += 2;
            cur = cur->next;
            continue;
        }
        if(cur == func->vars){
            fprintf(state->fp, "%s %%%i", cur->var->type->llvm, begin);
        }
        else{
            fprintf(state->fp, ", %s %%%i", cur->var->type->llvm, begin);
        }
        begin += 1;
        cur = cur->next;
    }
    if(func->func->native){
        state->tempnum = begin;
    }
    fprintf(state->fp, ")\n");
    if(func->to != NULL){
        fprintf(state->fp, "store %s %%%i, %s* %%%i\n", func->to->type->llvm, func->to->num, func->to->type->llvm, onum);
        func->to->num = onum;
    }
}

void lwrite_varassign(Variable* to, Variable* from, State* state){
    fprintf(state->fp, "%%%i = load %s, %s* %%%i\n", state->tempnum, from->type->llvm, from->type->llvm, from->num);
    fprintf(state->fp, "store %s %%%i, %s* %%%i\n", to->type->llvm, state->tempnum, to->type->llvm, to->num);
    state->tempnum += 1;
}


void lwrite_byte(Variable* to, char byte, State* state){
    unsigned char b = byte;
    fprintf(state->fp, "store i8 %d, i8* %%%i\n", b, to->num);
}

void lwrite_bool(Variable* to, int bool, State* state){
    if(bool){
        fprintf(state->fp, "store i8 1, i8* %%%i\n", to->num);
    }
    else{
        fprintf(state->fp, "store i8 0, i8* %%%i\n", to->num);
    }
}

void lwrite_int(Variable* to, int immediate, State* state){
    fprintf(state->fp, "store i64 %i, i64* %%%i\n", immediate, to->num);
}

void lwrite_indget(Variable* arr, Variable* ind, Variable* to, State* state){
    int arr_ = state->tempnum;
    int ind_ = state->tempnum+1;
    state->tempnum += 2;
    fprintf(state->fp, "%%%i = load %s, %s* %%%i\n", arr_, arr->type->llvm, arr->type->llvm, arr->num);
    fprintf(state->fp, "%%%i = load %s, %s* %%%i\n", ind_, ind->type->llvm, ind->type->llvm, ind->num);
    fprintf(state->fp, "%%%i = getelementptr inbounds %s, %s %%%i, %s %%%i\n",
        state->tempnum, arr->type->array_subtype->llvm, arr->type->llvm, arr_, ind->type->llvm, ind_);
    fprintf(state->fp, "%%%i = load %s, %s %%%i\n", state->tempnum + 1, arr->type->array_subtype->llvm, arr->type->llvm, state->tempnum);
    state->tempnum += 1;
    fprintf(state->fp, "store %s %%%i, %s %%%i\n", arr->type->array_subtype->llvm, state->tempnum,  arr->type->llvm, to->num);
    state->tempnum += 1;
}

void lwrite_indset(Variable* arr, Variable* ind, Variable* from, State* state){  
    int arr_ = state->tempnum;
    int ind_ = state->tempnum+1;
    int from_ = state->tempnum+2;
    state->tempnum += 3;
    fprintf(state->fp, "%%%i = load %s, %s* %%%i\n", arr_, arr->type->llvm, arr->type->llvm, arr->num);
    fprintf(state->fp, "%%%i = load %s, %s* %%%i\n", ind_, ind->type->llvm, ind->type->llvm, ind->num);    
    fprintf(state->fp, "%%%i = load %s, %s* %%%i\n", from_, from->type->llvm, from->type->llvm, from->num);
    fprintf(state->fp, "%%%i = getelementptr inbounds %s, %s %%%i, %s %%%i\n",
        state->tempnum, arr->type->array_subtype->llvm, arr->type->llvm, arr_, ind->type->llvm, ind_);
    fprintf(state->fp, "store %s %%%i, %s %%%i\n", arr->type->array_subtype->llvm, from_, arr->type->llvm, state->tempnum);
    state->tempnum += 1;
}

void lwrite_addeq(Variable* arr, Variable* delta, State* state){
    // broken
    // TODO fix this later
    // (not used by program atm.)
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
    int spnum = -12345678;
    for(int i = 0; i < len; i += 1){
        state->tempnum += 1;
        fprintf(state->fp, "%%%i = getelementptr inbounds i8, i8* %%%i, i64 %i\n", state->tempnum, arrloc, i+8);
        if(i == 0){
            spnum = state->tempnum;
        }
        fprintf(state->fp, "store i8 %i, i8* %%%i\n", (unsigned int) str[i], state->tempnum);
    }
    state->tempnum += 1;
    fprintf(state->fp, "store i8* %%%i, i8** %%%i\n", spnum, to->num);
}

void lwrite_card(Variable* to, Variable* from, State* state){
    int from_ = state->tempnum;
    state->tempnum += 1;
    fprintf(state->fp, "%%%i = load %s, %s* %%%i\n", from_, from->type->llvm, from->type->llvm, from->num);
    if(strcmp(from->type->llvm, "i64*") != 0){
        fprintf(state->fp, "%%%i = bitcast %s %%%i to i64*\n", state->tempnum, from->type->llvm, from_);
        state->tempnum += 1;
        fprintf(state->fp, "%%%i = getelementptr inbounds i64, i64* %%%i, i64 -1\n", state->tempnum, state->tempnum-1);
        fprintf(state->fp, "%%%i = load i64, i64* %%%i\n", state->tempnum+1, state->tempnum);
        state->tempnum += 1;
        fprintf(state->fp, "store i64 %%%i, i64* %%%i\n", state->tempnum, to->num);
        state->tempnum += 1;
    }
    else {
        fprintf(state->fp, "%%%i = getelementptr inbounds i64, i64* %%%i, i64 -1\n", state->tempnum, from_);
        fprintf(state->fp, "%%%i = load i64, i64* %%%i\n", state->tempnum+1, state->tempnum);
        fprintf(state->fp, "store i64 %%%i, i64* %%%i\n", state->tempnum+1, to->num);
        state->tempnum += 2;
    }
}

void lwrite_newarr(Variable* to, Variable* size, State* state){
    int size_ = state->tempnum;
    state->tempnum += 1;
    fprintf(state->fp, "%%%i = load %s, %s* %%%i\n", size_, size->type->llvm, size->type->llvm, size->num);
    if(strcmp(to->type->llvm, "i64*") != 0){
        switch(to->type->array_subtype->width){
            case 64:
                fprintf(state->fp, "%%%i = shl nuw i64 %%%i, 3\n", state->tempnum, size_);
                break;
            case 32:
                fprintf(state->fp, "%%%i = shl nuw i64 %%%i, 2\n", state->tempnum, size_);
                break;
            case 16:
                fprintf(state->fp, "%%%i = shl nuw i64 %%%i, 1\n", state->tempnum, size_);
                break;
            case 8:
                state->tempnum = state->tempnum-1;
                break;
        }
        fprintf(state->fp, "%%%i = add i64 %%%i, 8\n", state->tempnum+1, size_);
        state->tempnum += 2;
        fprintf(state->fp, "%%%i = call i8* @alloc(i64 %%%i)\n", state->tempnum, state->tempnum-1);
        fprintf(state->fp, "%%%i = bitcast i8* %%%i to i64*\n", state->tempnum+1, state->tempnum);
        state->tempnum += 1;
        fprintf(state->fp, "store i64 %%%i, i64* %%%i\n", size_, state->tempnum);
        fprintf(state->fp, "%%%i = getelementptr inbounds i64, i64* %%%i, i64 1\n", state->tempnum+1, state->tempnum);
        state->tempnum += 1;
        fprintf(state->fp, "%%%i = bitcast i64* %%%i to %s\n", state->tempnum+1, state->tempnum, to->type->llvm);
        fprintf(state->fp, "store %s %%%i, %s* %%%i\n", to->type->llvm, state->tempnum+1, to->type->llvm, to->num);
        state->tempnum += 2;
    }
    else{
        fprintf(state->fp, "%%%i = shl nuw i64 %%%i, 3\n", state->tempnum, size_);
        fprintf(state->fp, "%%%i = add i64 %%%i, 8\n", state->tempnum+1, state->tempnum);
        state->tempnum += 2;
        fprintf(state->fp, "%%%i = call i8* @alloc(i64 %%%i)\n", state->tempnum, state->tempnum-1);
        fprintf(state->fp, "%%%i = bitcast i8* %%%i to i64*\n", state->tempnum+1, state->tempnum);
        state->tempnum += 1;
        fprintf(state->fp, "store i64 %%%i, i64* %%%i\n", size_, state->tempnum);
        fprintf(state->fp, "%%%i = getelementptr inbounds i64, i64* %%%i, i64 1\n", state->tempnum+1, state->tempnum);
        fprintf(state->fp, "store i64* %%%i, i64** %%%i\n", state->tempnum+1, to->num);
        state->tempnum += 2;
    }
}

void lwrite_invert(Variable* to, Variable* from, State* state){
    fprintf(state->fp, "%%%i = load %s, %s* %%%i\n", state->tempnum, from->type->llvm, from->type->llvm, from->num);
    fprintf(state->fp, "%%%i = xor i8 1, %%%i\n", state->tempnum+1, state->tempnum);
    fprintf(state->fp, "store i8 %%%i, i8* %%%i\n", state->tempnum+1, to->num);
    state->tempnum += 2;
}

void lwrite_arith(Variable* to, Variable* left, Variable* right, int op, State* state){ 
    int left_ = state->tempnum;
    int right_ = state->tempnum+1;
    state->tempnum += 2;
    fprintf(state->fp, "%%%i = load %s, %s* %%%i\n", left_, left->type->llvm, left->type->llvm, left->num);
    fprintf(state->fp, "%%%i = load %s, %s* %%%i\n", right_, right->type->llvm, right->type->llvm, right->num);
    int result = -343434;
    switch(op){
        case PLUS:
            fprintf(state->fp, "%%%i = add %s %%%i, %%%i\n", state->tempnum, to->type->llvm, left_, right_);
            result = state->tempnum;
            state->tempnum += 1;
            break;
        case DOUBLEEQUALS:
            fprintf(state->fp, "%%%i = icmp eq %s %%%i, %%%i\n", state->tempnum, left->type->llvm, left_, right_);
            result = state->tempnum + 1;
            fprintf(state->fp, "%%%i = zext i1 %%%i to i8\n", result, state->tempnum);
            state->tempnum += 2;
            break;
        case LESS:         
            fprintf(state->fp, "%%%i = icmp slt i64 %%%i, %%%i\n", state->tempnum, left_, right_);
            result = state->tempnum + 1;
            fprintf(state->fp, "%%%i = zext i1 %%%i to i8\n", result, state->tempnum);
            state->tempnum += 2;
            break;
        case GREATER:
            fprintf(state->fp, "%%%i = icmp sgt i64 %%%i, %%%i\n", state->tempnum, left_, right_);
            result = state->tempnum + 1;
            fprintf(state->fp, "%%%i = zext i1 %%%i to i8\n", result, state->tempnum);
            state->tempnum += 2;
            break;
        case SUBTRACT:
            fprintf(state->fp, "%%%i = sub %s %%%i, %%%i\n", state->tempnum, to->type->llvm, left_, right_);
            result = state->tempnum;
            state->tempnum += 1;
            break;
        case MULT:
            fprintf(state->fp, "%%%i = mul %s %%%i, %%%i\n", state->tempnum, to->type->llvm, left_, right_);
            result = state->tempnum;
            state->tempnum += 1;
            break;
        case AMPERSAND:
            fprintf(state->fp, "%%%i = and %s %%%i, %%%i\n", state->tempnum, to->type->llvm, left_, right_);
            result = state->tempnum;
            state->tempnum += 1;
            break;
        case PIPE:
            fprintf(state->fp, "%%%i = or %s %%%i, %%%i\n", state->tempnum, to->type->llvm, left_, right_);
            result = state->tempnum;
            state->tempnum += 1;
            break;
        case FSLASH:
            fprintf(state->fp, "%%%i = sdiv %s %%%i, %%%i\n", state->tempnum, to->type->llvm, left_, right_);
            result = state->tempnum;
            state->tempnum += 1;
            break;
        case PERCENT:
            fprintf(state->fp, "%%%i = srem %s %%%i, %%%i\n", state->tempnum, to->type->llvm, left_, right_);
            result = state->tempnum;
            state->tempnum += 1;
            break;
    }
    fprintf(state->fp, "store %s %%%i, %s* %%%i\n", to->type->llvm, result, to->type->llvm, to->num);
}

void lwrite_constructor(Variable* dest, int width, State* state){
    // dest = new (width)
    // width in bytes. 
    int allocwidth = (((int)(width / 8))+1);
    fprintf(state->fp, "%%%i = call i8* @alloc(i64 %i)\n", state->tempnum, allocwidth);
    fprintf(state->fp, "%%%i = bitcast i8* %%%i to i%i*\n", state->tempnum+1, state->tempnum, width);
    fprintf(state->fp, "store %s %%%i, %s* %%%i\n", dest->type->llvm, state->tempnum+1, dest->type->llvm, dest->num);
    state->tempnum += 2;
}

void lwrite_accessor(Variable* dest, Variable* src, int off, State* state){
    // dest = src {off}
    fprintf(state->fp, "%%%i = load %s, %s* %%%i\n", state->tempnum, src->type->llvm, src->type->llvm, src->num);
    fprintf(state->fp, "%%%i = load i%i, %s %%%i\n", state->tempnum+1, src->type->internal_width, src->type->llvm, state->tempnum);
    state->tempnum += 1;
    fprintf(state->fp, "%%%i = lshr i%i %%%i, %i\n", state->tempnum+1, src->type->internal_width, state->tempnum, off);
    fprintf(state->fp, "%%%i = trunc i%i %%%i to i%i\n", state->tempnum+2, src->type->internal_width, state->tempnum+1, dest->type->width);
    if(dest->type->llvm[strlen(dest->type->llvm)-1] == '*'){
        fprintf(state->fp, "%%%i = inttoptr i%i %%%i to %s\n", state->tempnum+3, dest->type->width, state->tempnum+2, dest->type->llvm);
        fprintf(state->fp, "store %s %%%i, %s* %%%i\n", dest->type->llvm, state->tempnum+3, dest->type->llvm, dest->num);
        state->tempnum += 4;
    }
    else{
        fprintf(state->fp, "store %s %%%i, %s* %%%i\n", dest->type->llvm, state->tempnum+2, dest->type->llvm, dest->num);
        state->tempnum += 3;
    }
}
void lwrite_setmember(Variable* dest, Variable* src, int off, State* state){
    // dest {off} = src
    
    // load dest and convert to type.
    fprintf(state->fp, "%%%i = load %s, %s* %%%i\n", state->tempnum, dest->type->llvm, dest->type->llvm, dest->num);
    fprintf(state->fp, "%%%i = bitcast %s %%%i to i8*\n", state->tempnum+1, dest->type->llvm, state->tempnum);
    fprintf(state->fp, "%%%i = getelementptr i8, i8* %%%i, i64 %i\n", state->tempnum+2, state->tempnum+1, off / 8);
    fprintf(state->fp, "%%%i = bitcast i8* %%%i to %s*\n", state->tempnum+3, state->tempnum+2, src->type->llvm);
    fprintf(state->fp, "%%%i = load %s, %s* %%%i\n", state->tempnum+4, src->type->llvm, src->type->llvm, src->num);
    fprintf(state->fp, "store %s %%%i, %s* %%%i\n", src->type->llvm, state->tempnum+4, src->type->llvm, state->tempnum+3);
    state->tempnum += 5;
}
void lwrite_settag(Variable* var, int off, State* state){
    return;
    // special case of setmember, where src is const 0.
    fprintf(state->fp, "%%%i = load %s, %s* %%%i\n", state->tempnum, var->type->llvm, var->type->llvm, var->num);
    fprintf(state->fp, "%%%i = load i%i, %s %%%i\n", state->tempnum+1, var->type->internal_width, var->type->llvm, state->tempnum);
    state->tempnum += 1;
    fprintf(state->fp, "%%%i = zext i1 1 to i%i\n", state->tempnum+1, var->type->internal_width);
    fprintf(state->fp, "%%%i = lshr i%i %%%i, %i\n", state->tempnum+2, var->type->internal_width, state->tempnum+1, off);
    fprintf(state->fp, "%%%i = and i%i %%%i, %%%i\n", state->tempnum+3, var->type->internal_width, state->tempnum+2, state->tempnum);
    fprintf(state->fp, "store i%i %%%i, %s %%%i\n", var->type->internal_width, state->tempnum+3, var->type->llvm, state->tempnum-1);
    state->tempnum += 4;
}

void lwrite_conditional(Variable* test, char* truelbl, char* falselbl, State* state){
    fprintf(state->fp, "%%%i = load %s, %s* %%%i\n", state->tempnum, test->type->llvm, test->type->llvm, test->num);
    fprintf(state->fp, "%%%i = trunc %s %%%i to i1\n", state->tempnum+1, test->type->llvm, state->tempnum);
    fprintf(state->fp, "br i1 %%%i, label %%%s, label %%%s\n", state->tempnum+1, truelbl, falselbl);
    state->tempnum += 2;
}

void lwrite_label(char* lbl, int uncond, State* state){
    if(uncond){
         fprintf(state->fp, "br label %%%s\n", lbl);
    }
    fprintf(state->fp, "%s:\n", lbl);
}
