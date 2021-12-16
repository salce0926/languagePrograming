#include "token-list.h"

#define CALL(function) if(function == ERROR) return(ERROR)
#define JUDGE(code, string) if(token != code) return(error(string));prettyPrint(token);token = scan()

int token;
char *scope_p = NULL;

char *temp_id_name;
struct ID *temp_id;
struct TYPE *temp_type;
struct ID *temp_procedure;
struct ID *globalidroot;
struct ID *localidroot;

static int is_line_head = 1;
static int indent_count = 0;
static int in_while = 0;

extern char *tokenstr[NUMOFTOKEN+1];
extern char string_attr[MAXSTRSIZE];

void init_parse(){
    globalidroot = NULL;
    localidroot = NULL;
}

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
    JUDGE(TSEMI, "Semicolon is not found");
    while(token == TNAME){
        CALL(variable_names());
        JUDGE(TCOLON, "Colon is not found");
        CALL(type());
        JUDGE(TSEMI, "Semicolon is not found");
    }
    return(NORMAL);
}

int variable_names(){
    CALL(variable_name());
    CALL(store_id_byname(&temp_id, temp_id_name));
    while(token == TCOMMA){
        JUDGE(TCOMMA, "Comma is not found");
        CALL(variable_name());
        CALL(store_id_byname(&temp_id, temp_id_name));  
    }
    return(NORMAL);
}

int variable_name(){
    JUDGE(TNAME, "Variable name is not found");
    CALL(store_idname(temp_id_name, string_attr));
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
    CALL(register_id_bytype(&temp_id, temp_type));
    return(NORMAL);
}

int standard_type(){
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
    CALL(store_standard_type(&temp_type, to_ttype(token)));
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
    if(token == TLPAREN) CALL(formal_parameters());
    JUDGE(TSEMI, "Semicolon is not found");
    if(token == TVAR) CALL(variable_declaration());
    indent_count = 1;
    CALL(compound_statement());
    JUDGE(TSEMI, "Semicolon is not found");
    return(NORMAL);
}

int procedure_name(){
    JUDGE(TNAME, "Procedure name is not found");
    return(NORMAL);
}

int formal_parameters(){
    JUDGE(TLPAREN, "'(' is not found");
    CALL(variable_names());
    JUDGE(TCOLON, "Colon is not found");
    CALL(standard_type());
    while(token == TSEMI){
        JUDGE(TSEMI, "Semicolon is not found");
        CALL(variable_names());
        JUDGE(TCOLON, "Colon is not found");
        CALL(standard_type());
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
    JUDGE(TIF, "Keyword 'if' is not found");
    CALL(expression());
    JUDGE(TTHEN, "Keyword 'then' is not found");
    CALL(statement());
    if(token != TELSE) return(NORMAL);
    JUDGE(TELSE, "Keyword 'else' is not found");
    CALL(statement());
    return(NORMAL);
}

int iteration_statement(){
    in_while++;
    JUDGE(TWHILE, "Keyword 'while' is not found");
    CALL(expression());
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
    JUDGE(TCALL, "Keyword 'call' is not found");
    CALL(procedure_name());
    if(token != TLPAREN) return(NORMAL);
    JUDGE(TLPAREN, "'(' is not found");
    CALL(expressions());
    JUDGE(TRPAREN, "')' is not found");
    return(NORMAL);
}

int expressions(){
    CALL(expression());
    while(token == TCOMMA){
        JUDGE(TCOMMA, "Comma is not found");
        CALL(expression());
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
    return(NORMAL);
}

int left_part(){
    CALL(variable());
    return(NORMAL);
}

int variable(){
    CALL(variable_name());
    if(token != TLSQPAREN) return(NORMAL);
    JUDGE(TLSQPAREN, "'[' is not found");
    CALL(expression());
    JUDGE(TRSQPAREN, "']' is not found");
    return(NORMAL);
}

int expression(){
    CALL(simple_expression());
    while(token == TEQUAL || token == TNOTEQ || token == TLE || token == TLEEQ || token == TGR || token == TGREQ){
        CALL(relational_operator());
        CALL(simple_expression());
    }
    return(NORMAL);
}

int simple_expression(){
    if(token == TPLUS || token == TMINUS){
        if(token == TPLUS){
            JUDGE(TPLUS, "'+' is not found");
        }else if(token == TMINUS){
            JUDGE(TMINUS, "'-' is not found");
        }
    }
    CALL(term());
    while(token == TPLUS || token == TMINUS || token == TOR){
        CALL(additive_operator());
        CALL(term());
    }
    return(NORMAL);
}

int term(){
    CALL(factor());
    while(token == TSTAR || token == TDIV || token == TAND){
        CALL(multiplicative_operator());
        CALL(factor());
    }
    return(NORMAL);
}

int factor(){
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
    }
    else if(token == TINTEGER || token == TBOOLEAN || token == TCHAR){
        CALL(standard_type());
        JUDGE(TLPAREN, "'(' is not found");
        CALL(expression());
        JUDGE(TRPAREN, "')' is not found");
    }
    else return(error("Factor is not found"));
    return(NORMAL);
}

int constant(){
    if(token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING){
        if(token == TNUMBER){
            JUDGE(TNUMBER, "Number is not found");
        }else if(token == TFALSE){
            JUDGE(TFALSE, "Keyword 'false' is not found");
        }else if(token == TTRUE){
            JUDGE(TTRUE, "Keyword 'true' is not found");
        }else if(token == TSTRING){
            JUDGE(TSTRING, "String is not found");
        }
    }
    return(NORMAL);
}

int multiplicative_operator(){
    if(token == TSTAR || token == TDIV || token == TAND){
        if(token == TSTAR){
            JUDGE(TSTAR, "'*' is not found");
        }else if(token == TDIV){
            JUDGE(TDIV, "Keyword 'div' is not found");
        }else if(token == TAND){
            JUDGE(TAND, "Keyword 'and' is not found");
        }
    }
    return(NORMAL);
}

int additive_operator(){
    if(token == TPLUS || token == TMINUS || token == TOR){
        if(token == TPLUS){
            JUDGE(TPLUS, "'+' is not found");
        }else if(token == TMINUS){
            JUDGE(TMINUS, "'-' is not found");
        }else if(token == TOR){
            JUDGE(TOR, "Keyword 'or' is not found");
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
    while(token == TCOMMA){
        JUDGE(TCOMMA, "Comma is not found");
        CALL(variable());
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
    CALL(expression());
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