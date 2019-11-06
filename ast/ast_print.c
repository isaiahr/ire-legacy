#include"ast.h"
#include"ast_print.h"

/**
 *
 * ast_print.c -- contains funcs for printing the ast.
 * 
 */

void print_func(Function* func);
void print_type(Type* t);
void print_type_helper(TypeStructure* ts);
void print_body(Body* body);



char* formatvar(Variable* var){
    if(var == NULL){
        return "nullvar";
    }
    if(var->identifier != NULL){
        return var->identifier;
    }
    return "unnamed";
}

void print_func(Function* func){
    printf("%s %s (", func->retval->identifier, func->name);
    VarList* p = func->params;
    while(p != NULL){
            if(p->next == NULL){
                printf("%s %s", p->var->type->identifier, p->var->identifier);
            }
            else{
                printf("%s %s, ", p->var->type->identifier, p->var->identifier);
            }
            p = p->next;
    }
    printf(")\n");
    print_body(func->body);
}
void print_body(Body* cur){
    while(cur != NULL){
        switch(cur->stmt->type){
            case S_ASSIGNMENT:
                ; // nessecary, see above for details
                Assignment* a = (Assignment*) cur->stmt->stmt;
                printf("    %s = %s\n", formatvar(a->to), formatvar(a->from));
                break;
                
            case S_CONSTANTASSIGNMENT:
                ;
                ConstantAssignment* ca = (ConstantAssignment*) cur->stmt->stmt;
                printf("    %s = (const)\n", formatvar(ca->to));
                break;
            case S_FUNCTIONCALL:
                ;
                FunctionCall* fc = (FunctionCall*) cur->stmt->stmt;
                printf("    %s = %s (", formatvar(fc->to), fc->func->name);
                VarList* vl = fc->vars;
                while(vl != NULL){
                    printf("%s, ", formatvar(vl->var));
                    vl = vl->next;
                }
                printf(")\n");
                break;
            case S_VARINIT:
                ;
                VarInit* vi = (VarInit*) cur->stmt->stmt;
                printf("    init %s\n", formatvar(vi->var));
                break;
            case S_INDEX:
                ;
                Index* i = (Index*) cur->stmt->stmt;
                printf("    %s = %s[%s]\n", formatvar(i->to), formatvar(i->arr), formatvar(i->ind));
                break;
            case S_INDEXEQUALS:
                ;
                IndexEquals* ie = (IndexEquals*) cur->stmt->stmt;
                printf("    %s[%s] = %s\n", formatvar(ie->arr), formatvar(ie->ind), formatvar(ie->eq));
                break;
            case S_ADDEQUALS:
                ;
                AddEquals* ae = (AddEquals*) cur->stmt->stmt;
                printf("    %s += %s\n", formatvar(ae->var), formatvar(ae->delta));
                break;
            case S_CARDINALITY:
                ;
                Cardinality* card = (Cardinality*) cur->stmt->stmt;
                printf("    %s = |%s|\n", formatvar(card->to), formatvar(card->from));
                break;
            case S_ARITHMETIC:
                ;
                Arithmetic* arith = (Arithmetic*) cur->stmt->stmt;
                printf("    %s = %s %c %s\n", formatvar(arith->to), formatvar(arith->left), sym(arith->operation), formatvar(arith->right));
                break;
            case S_CONSTRUCTOR:
                ;
                Constructor* cons = (Constructor*) cur->stmt->stmt;
                printf("    %s = new %s\n", formatvar(cons->to), cons->type->identifier);
                break;
            case S_SETMEMBER:
                ;
                Setmember* setm = (Setmember*) cur->stmt->stmt;
                printf("    %s {%i} = %s\n", formatvar(setm->dest), setm->offsetptr, formatvar(setm->from));
                break;
            case S_SETTAG:
                ;
                SetTag* sett = (SetTag*) cur->stmt->stmt;
                printf("    settag %s bit offset %i\n", formatvar(sett->dest), sett->offsetptr);
                break;
            case S_ACCESSOR:
                ;
                Accessor* acce = (Accessor*) cur->stmt->stmt;
                printf("    %s = %s {%i}\n", formatvar(acce->to), formatvar(acce->src), acce->offsetptr);
            case S_RETURN:
                ;
                Return* ret = (Return*) cur->stmt->stmt;
                printf("    return %s\n", formatvar(ret->var));
                break;
            case S_IF:
                ;
                IfStmt* ifs = (IfStmt*) cur->stmt->stmt;
                printf("    if %s\n", formatvar(ifs->test));
                print_body(ifs->scope->body);
                ifs = ifs->elsestmt;
                while(ifs != NULL){
                    printf("    endif\n");
                    if(ifs->test == NULL){
                        printf("    else\n");
                    }
                    else {
                        printf("    elseif %s\n", formatvar(ifs->test));
                    }
                    print_body(ifs->scope->body);
                    ifs = ifs->elsestmt;
                }
                printf("    endif\n");
                
        }
        cur = cur->next;
    }
}

void print_type(Type* t){
    if(t->ts == NULL){
        printf("   %s, native, width %i\n", t->identifier, t->width);
        return;
    }
    printf("   %s {", t->identifier);
    print_type_helper(t->ts);
    printf("}, internalwidth %i\n", t->internal_width);
}

void print_type_helper(TypeStructure* ts){
    if(ts->mode == S_MODE_TYPE){
        if(ts->sbs == NULL){
            printf("(?)");
        }
        else {
            printf("%s %s", ts->sbs->identifier, ts->identifier);
        }
        return;
    }
    TypeStructure* t = ts->sub;
    printf("(");
    while(t != NULL){
        if(ts->mode != S_MODE_AND){
            printf("%s: ", t->segment);
        }
        print_type_helper(t);
        t = t->next;
        if(t != NULL){
            switch(ts->mode){
                case S_MODE_AND: printf(" & "); break;
                case S_MODE_OR: printf(" | "); break;
            }
        }
    }
    printf(")");
}

void print_prog(Program* prog){
    printf("PROGRAM\n");
    printf("FUNCTIONS\n");
    for(int i = 0; i < prog->func_count; i++){
        if(!prog->funcs[i].native){
            print_func(&prog->funcs[i]);
        }
    }
    printf("TYPES\n");
    TypeList* cur = prog->types;
    while(cur != NULL){
        print_type(cur->type);
        cur = cur->next;
    }
}
