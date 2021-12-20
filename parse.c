#include "token-list.h"

#define CALL(function) if(function == ERROR) return(ERROR)
#define JUDGE(code, string) if(token != code) return(error(string));/*prettyPrint(token);*/token = scan()

int token;
char *scope_p = NULL;

char *temp_id_name = NULL;
struct ID *temp_id = NULL;
struct TYPE *temp_type = NULL;
struct ID *temp_procedure = NULL;
struct TYPE *temp_argument = NULL;
struct ID *globalidroot = NULL;
struct ID *localidroot = NULL;

static int is_line_head = 1;
static int indent_count = 0;
static int in_while = 0;

extern char *tokenstr[NUMOFTOKEN+1];
extern char string_attr[MAXSTRSIZE];

int parse_program() {
    JUDGE(TPROGRAM, "Keyword 'program' is not found");
    JUDGE(TNAME, "Program name is not found");
    JUDGE(TSEMI, "Semicolon is not found");
    CALL(block());
    JUDGE(TDOT, "Period is not found at the end of program");
    return(NORMAL);
}

int block(){
    while(token == TVAR || token == TPROCEDURE){
        if(token == TVAR) CALL(variable_declaration());
        if(token == TPROCEDURE) CALL(subprogram_declaration());
    }
    indent_count = 0;
    CALL(compound_statement());
    return(NORMAL);
}

int variable_declaration(){
    indent_count = 1;
    JUDGE(TVAR, "Keyword 'var' is not found");
    CALL(variable_names());
    JUDGE(TCOLON, "Colon is not found");
    CALL(type());
    CALL(register_id_bytype(temp_id, temp_type));
    temp_id = NULL;
    temp_type = NULL;
    JUDGE(TSEMI, "Semicolon is not found");
    while(token == TNAME){
        CALL(variable_names());
        JUDGE(TCOLON, "Colon is not found");
        CALL(type());
        CALL(register_id_bytype(temp_id, temp_type));
        temp_id = NULL;
        temp_type = NULL;
        JUDGE(TSEMI, "Semicolon is not found");
    }
    return(NORMAL);
}

int variable_names(){
    CALL(variable_name());
    CALL(store_id_byname(&temp_id, temp_id_name));
    free(temp_id_name);
    temp_id_name = NULL;
    while(token == TCOMMA){
        JUDGE(TCOMMA, "Comma is not found");
        CALL(variable_name());
        CALL(store_id_byname(&temp_id, temp_id_name));
        free(temp_id_name);
        temp_id_name = NULL;
    }
    return(NORMAL);
}

int variable_name(){
    CALL(store_idname(&temp_id_name, string_attr));
    JUDGE(TNAME, "Variable name is not found");
    string_attr[0] = '\0';/*just in case*/
    return(NORMAL);
}

int type(){
    if(token == TINTEGER || token == TBOOLEAN || token == TCHAR){
        CALL(standard_type());
    }
    else if(token == TARRAY){
        CALL(array_type());
    }
    else return(error("Type is not found"));
    return(NORMAL);
}

int standard_type(){
    int type = to_ttype(token);
    if(token == TINTEGER || token == TBOOLEAN || token == TCHAR){
        if(token == TINTEGER){
            JUDGE(TINTEGER, "Keyword 'integer' is not found");
        }else if(token == TBOOLEAN){
            JUDGE(TBOOLEAN, "Keyword 'boolean' is not found");
        }else if(token == TCHAR){
            JUDGE(TCHAR, "Keyword 'char' is not found");
        }
    }
    else return(error("Standard type is not found"));
    CALL(store_standard_type(&temp_type, type));
    return(NORMAL);
}

int array_type(){
    JUDGE(TARRAY, "Keyword 'array' is not found");
    JUDGE(TLSQPAREN, "'[' is not found");
    JUDGE(TNUMBER, "Index number is not found");
    JUDGE(TRSQPAREN, "']' is not found");
    JUDGE(TOF, "Keyword 'of' is not found");
    CALL(standard_type());
    CALL(store_array_type(&temp_type, num_attr));
    return(NORMAL);
}

int subprogram_declaration(){
    indent_count = 1;
    JUDGE(TPROCEDURE, "Keyword 'procedure' is not found");
    CALL(procedure_name());
    CALL(store_id_byname(&temp_procedure, temp_id_name));
    CALL(store_procedure_type(temp_procedure));
    scope_p = create_newname(temp_id_name);
    free(temp_id_name);
    temp_id_name = NULL;
    if(token == TLPAREN) CALL(formal_parameters());
    CALL(register_procedure(temp_procedure));
    temp_procedure = NULL;
    JUDGE(TSEMI, "Semicolon is not found");
    if(token == TVAR) CALL(variable_declaration());
    indent_count = 1;
    CALL(compound_statement());
    JUDGE(TSEMI, "Semicolon is not found");
    free(scope_p);
    scope_p = NULL;
    return(NORMAL);
}

int procedure_name(){
    CALL(store_idname(&temp_id_name, string_attr));
    JUDGE(TNAME, "Procedure name is not found");
    string_attr[0] = '\0';/*just in case*/
    return(NORMAL);
}

int formal_parameters(){
    JUDGE(TLPAREN, "'(' is not found");
    CALL(variable_names());
    JUDGE(TCOLON, "Colon is not found");
    CALL(standard_type());
    CALL(register_parameter_bytype(temp_id, temp_procedure, temp_type));
    temp_id = NULL;
    temp_type = NULL;
    while(token == TSEMI){
        JUDGE(TSEMI, "Semicolon is not found");
        CALL(variable_names());
        JUDGE(TCOLON, "Colon is not found");
        CALL(standard_type());
        CALL(register_parameter_bytype(temp_id, temp_procedure, temp_type));
        temp_id = NULL;
        temp_type = NULL;
    }
    JUDGE(TRPAREN, "')' is not found");
    return(NORMAL);
}

int compound_statement(){
    JUDGE(TBEGIN, "Keyword 'begin' is not found");
    indent_count++;
    CALL(statement());
    while(token == TSEMI){
        JUDGE(TSEMI, "Semicolon is not found");
        CALL(statement());
    }
    indent_count--;
    JUDGE(TEND, "Keyword 'end' is not found");
    return(NORMAL);
}

int statement(){
    if(token == TNAME){
        CALL(assignment_statement());
    } 
    else if(token == TIF){
        CALL(condition_statement());
    } 
    else if(token == TWHILE){
        CALL(iteration_statement());
    }
    else if(token == TBREAK){
        CALL(exit_statement());
    }
    else if(token == TCALL){
        CALL(call_statement());
    }
    else if(token == TRETURN){
        CALL(return_statement());
    }
    else if(token == TREAD || token == TREADLN){
        CALL(input_statement());
    }
    else if(token == TWRITE || token == TWRITELN){
        CALL(output_statement());
    }
    else if(token == TBEGIN){
        CALL(compound_statement());
    }
    else{
        CALL(empty_statement());
    }
    return(NORMAL);
}

int condition_statement(){
    struct TYPE *p;
    JUDGE(TIF, "Keyword 'if' is not found");
    CALL(expression());
    p = pop_front_type(&temp_type);
    if(p->ttype != TPBOOL) return(error("the type of expressions must be boolean"));
    release_typetab(&p);
    JUDGE(TTHEN, "Keyword 'then' is not found");
    CALL(statement());
    if(token != TELSE) return(NORMAL);
    JUDGE(TELSE, "Keyword 'else' is not found");
    CALL(statement());
    return(NORMAL);
}

int iteration_statement(){
    struct TYPE *p;
    in_while++;
    JUDGE(TWHILE, "Keyword 'while' is not found");
    CALL(expression());
    p = pop_front_type(&temp_type);
    if(p->ttype != TPBOOL) return(error("the type of expressions must be boolean"));
    release_typetab(&p);
    JUDGE(TDO, "Keyword 'do' is not found");
    CALL(statement());
    in_while--;
    return(NORMAL);
}

int exit_statement(){
    if(in_while < 1) return(error("Keyword 'break' must be included in at least one iteration sentence."));
    JUDGE(TBREAK, "Keyword 'break' is not found");
    return(NORMAL);
}

int call_statement(){
    struct ID *proc;
    JUDGE(TCALL, "Keyword 'call' is not found");
    CALL(procedure_name());
    proc = search_id_byname(globalidroot, temp_id_name, NULL);
    free(temp_id_name);
    temp_id_name = NULL;
    if(proc == NULL){
        return(error("this procedure is not found\n"));
    }
    if(proc->itp->ttype != TPPROC){
        return(error("this id is not defined as procedure\n"));
    }
    if(proc->name != NULL && scope_p != NULL && strcmp(proc->name, scope_p) == 0){
        return(error("recursive calls are not allowed\n"));
    }
    CALL(ref_newid(proc));
    if(token != TLPAREN){
        CALL(check_argument(temp_procedure, &temp_argument));
        return(NORMAL);
    }
    JUDGE(TLPAREN, "'(' is not found");
    CALL(expressions());
    JUDGE(TRPAREN, "')' is not found");
    CALL(check_argument(proc, &temp_argument));
    release_typetab(&temp_argument);
    return(NORMAL);
}

int expressions(){
    struct TYPE *p;
    CALL(expression());
    p = pop_front_type(&temp_type);
    CALL(store_argument(&temp_argument, p->ttype));
    release_typetab(&p);
    while(token == TCOMMA){
        JUDGE(TCOMMA, "Comma is not found");
        CALL(expression());
        p = pop_front_type(&temp_type);
        CALL(store_argument(&temp_argument, p->ttype));
        release_typetab(&p);
    }
    return(NORMAL);
}

int return_statement(){
    JUDGE(TRETURN, "Keyword 'return' is not found");
    return(NORMAL);
}

int assignment_statement(){
    CALL(left_part());
    JUDGE(TASSIGN, "':=' is not found");
    CALL(expression());
    CALL(check_operand_type(&temp_type));
    temp_type = NULL;
    return(NORMAL);
}

int left_part(){   
    CALL(variable());
    return(NORMAL);
}

int variable(){
    struct ID *list, *p;
    struct TYPE *q;
    CALL(variable_name());
    list = *get_idroot();
    p = search_id_byname(list, temp_id_name, scope_p);
    if(scope_p != NULL && p == NULL){
        p = search_id_byname(globalidroot, temp_id_name, NULL);
    }
    free(temp_id_name);
    temp_id_name = NULL;
    if(p == NULL || p->itp->ttype == TPPROC) return(error("this variable name is undifined\n"));
    CALL(store_standard_type(&temp_type, p->itp->ttype));
    if(is_array(p->itp)){
        CALL(store_array_type(&temp_type, p->itp->arraysize));
    }
    CALL(ref_newid(p));
    if(token != TLSQPAREN) return(NORMAL);
    if(!is_array(p->itp)){
        return(error("the type of this id must be array\n"));
    }
    JUDGE(TLSQPAREN, "'[' is not found");
    CALL(expression());
    q = pop_front_type(&temp_type);
    if(q->ttype != TPINT) return(error("the type of index must be integer\n"));
    release_typetab(&q);
    JUDGE(TRSQPAREN, "']' is not found");
    temp_type->arraysize = -1;
    return(NORMAL);
}

int expression(){
    struct TYPE *p;
    CALL(simple_expression());
    while(token == TEQUAL || token == TNOTEQ || token == TLE || token == TLEEQ || token == TGR || token == TGREQ){
        CALL(relational_operator());
        CALL(simple_expression());
        CALL(check_operand_type(&temp_type));
        p = pop_front_type(&temp_type);
        release_typetab(&p);
        CALL(store_standard_type(&temp_type, TPBOOL));
    }
    return(NORMAL);
}

int simple_expression(){
    int check = 0;
    if(token == TPLUS || token == TMINUS){
        check = 1;
        if(token == TPLUS){
            JUDGE(TPLUS, "'+' is not found");
        }else if(token == TMINUS){
            JUDGE(TMINUS, "'-' is not found");
        }
        CALL(store_standard_type(&temp_type, TPINT));
    }
    CALL(term());
    if(check){
        CALL(check_operand_type(&temp_type));
    }
    while(token == TPLUS || token == TMINUS || token == TOR){
        CALL(additive_operator());
        CALL(term());
        CALL(check_operand_type(&temp_type));
    }
    return(NORMAL);
}

int term(){
    CALL(factor());
    while(token == TSTAR || token == TDIV || token == TAND){
        CALL(multiplicative_operator());
        CALL(factor());
        CALL(check_operand_type(&temp_type));
    }
    return(NORMAL);
}

int factor(){
    struct TYPE *p;
    if(token == TNAME){
        CALL(variable());
    }
    else if(token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING){
        CALL(constant());
    }
    else if(token == TLPAREN){
        JUDGE(TLPAREN, "'(' is not found");
        CALL(expression());
        JUDGE(TRPAREN, "')' is not found");
    }
    else if(token == TNOT){
        JUDGE(TNOT, "Keyword 'not' is not found");
        CALL(factor());
        if(temp_type->ttype != TPBOOL){
            return(error("unsupported operand type for 'not'\n"));
        }
    }
    else if(token == TINTEGER || token == TBOOLEAN || token == TCHAR){
        CALL(standard_type());
        JUDGE(TLPAREN, "'(' is not found");
        CALL(expression());
        JUDGE(TRPAREN, "')' is not found");
        p = pop_front_type(&temp_type);
        if(is_array(p)) return(error("using array type as standard type is not allowed\n"));
        release_typetab(&p);
    }
    else return(error("Factor is not found"));
    return(NORMAL);
}

int constant(){
    int length;
    if(token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING){
        if(token == TNUMBER){
            JUDGE(TNUMBER, "Number is not found");
            CALL(store_standard_type(&temp_type, TPINT));
        }else if(token == TFALSE){
            JUDGE(TFALSE, "Keyword 'false' is not found");
            CALL(store_standard_type(&temp_type, TPBOOL));
        }else if(token == TTRUE){
            JUDGE(TTRUE, "Keyword 'true' is not found");
            CALL(store_standard_type(&temp_type, TPBOOL));
        }else if(token == TSTRING){
            length = strlen(string_attr);
            JUDGE(TSTRING, "String is not found");
            if(length != 1) return(error("the length of the string must be 1\n"));
            CALL(store_standard_type(&temp_type, TPCHAR));
        }
    }
    return(NORMAL);
}

int multiplicative_operator(){
    if(token == TSTAR || token == TDIV || token == TAND){
        if(token == TSTAR){
            JUDGE(TSTAR, "'*' is not found");
            if(temp_type->ttype != TPINT){
                return(error("unsupported operand type for '*'\n"));
            }
        }else if(token == TDIV){
            JUDGE(TDIV, "Keyword 'div' is not found");
            if(temp_type->ttype != TPINT){
                return(error("unsupported operand type for 'div'\n"));
            }
        }else if(token == TAND){
            JUDGE(TAND, "Keyword 'and' is not found");
            if(temp_type->ttype != TPBOOL){
                return(error("unsupported operand type for 'and'\n"));
            }
        }
    }
    return(NORMAL);
}

int additive_operator(){
    if(token == TPLUS || token == TMINUS || token == TOR){
        if(token == TPLUS){
            JUDGE(TPLUS, "'+' is not found");
            if(temp_type->ttype != TPINT){
                return(error("unsupported operand type for '+'\n"));
            }
        }else if(token == TMINUS){
            JUDGE(TMINUS, "'-' is not found");
            if(temp_type->ttype != TPINT){
                return(error("unsupported operand type for '-'\n"));
            }
        }else if(token == TOR){
            JUDGE(TOR, "Keyword 'or' is not found");
            if(temp_type->ttype != TPBOOL){
                return(error("unsupported operand type for 'or'\n"));
            }
        }
    }
    return(NORMAL);
}

int relational_operator(){
    if(token == TEQUAL || token == TNOTEQ || token == TLE || token == TLEEQ ||token == TGR || token == TGREQ){
        if(token == TEQUAL){
            JUDGE(TEQUAL, "'=' is not found");
        }else if(token == TNOTEQ){
            JUDGE(TNOTEQ, "'<>' is not found");
        }else if(token == TLE){
            JUDGE(TLE, "'<' is not found");
        }else if(token == TLEEQ){
            JUDGE(TLEEQ, "'<=' is not found");
        }else if(token == TGR){
            JUDGE(TGR, "'>' is not found");
        }else if(token == TGREQ){
            JUDGE(TGREQ, "'>=' is not found");
        }
    }
    return(NORMAL);
}

int input_statement(){
    struct TYPE *p;
    if(token == TREAD || token == TREADLN){
        if(token == TREAD){
            JUDGE(TREAD, "Keyword 'read' is not found");
        }else if(token == TREADLN){
            JUDGE(TREADLN, "Keyword 'readln' is not found");
        }
    }
    if(token != TLPAREN) return(NORMAL);
    JUDGE(TLPAREN, "'(' is not found");
    CALL(variable());
    p = pop_front_type(&temp_type);
    if(p->ttype != TPINT && p->ttype != TPCHAR) return(error("the type of variable must be integer or char\n"));
    release_typetab(&p);
    while(token == TCOMMA){
        JUDGE(TCOMMA, "Comma is not found");
        CALL(variable());
        p = pop_front_type(&temp_type);
        if(p->ttype != TPINT && p->ttype != TPCHAR) return(error("the type of variable must be integer or char\n"));
        release_typetab(&p);
    }
    JUDGE(TRPAREN, "')' is not found");
    return(NORMAL);
}

int output_statement(){
    if(token == TWRITE || token == TWRITELN){
        if(token == TWRITE){
            JUDGE(TWRITE, "Keyword 'write' is not found");
        }else if(token == TWRITELN){
            JUDGE(TWRITELN, "Keyword 'writeln' is not found");
        }
    }
    if(token != TLPAREN) return(NORMAL);
    JUDGE(TLPAREN, "'(' is not found");
    CALL(output_format());
    while(token == TCOMMA){
        JUDGE(TCOMMA, "Comma is not found");
        CALL(output_format());
    }
    JUDGE(TRPAREN, "')' is not found");
    return(NORMAL);
}

int output_format(){
    struct TYPE *p;
    if(token == TSTRING){
        JUDGE(TSTRING, "String is not found");
        return(NORMAL);
    }
    CALL(expression());
    p = pop_front_type(&temp_type);
    if(is_array(p)) return(error("using array type as output format is not allowed\n"));
    release_typetab(&p);
    if(token != TCOLON) return(NORMAL);
    JUDGE(TCOLON, "Colon is not found");
    JUDGE(TNUMBER, "Number is not found");
    return(NORMAL);
}

int empty_statement(){
    return(NORMAL);
}

void prettyPrint(int token){
    int i;
    if(!is_line_head && (token == TBEGIN || token == TEND || token == TELSE)){
        printf("\n");
        is_line_head = 1;
    }
    if(is_line_head){
        for(i = 0; i < indent_count; i++){
            printf("    ");
        }
    }else if(token != TSEMI && token != TCOMMA && token != TDOT){
        printf(" ");
    }

    if(token == TSTRING){
        printf("'%s'", string_attr);
    }else if(token == TNAME){
        printf("%s", string_attr);
    }else if(token == TNUMBER){
        sprintf(string_attr, "%d", num_attr);
        printf("%s", string_attr);
    }else{
        printf("%s", tokenstr[token]);
    }
    is_line_head = 0;

    if(token == TSEMI || token == TBEGIN){
        printf("\n");
        is_line_head = 1;
    }
}

int error(char *mes){
    printf("\n in line %d, ERROR: %s\n", get_linenum(), mes);
    end_scan(); 
    return(ERROR);
}