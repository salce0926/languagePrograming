#include "token-list.h"

#define NORMAL 0
#define ERROR 1

#define CALL(function) if(function == ERROR) return(ERROR)
#define JUDGE(code, string) if(token != code) return(error(string));token = scan()

int token;
static int in_while = 0;
static int is_debugmode = 1;

int parse_program() {
    debug();
    debugPrintf("parse_program\n");
    if(token != TPROGRAM) return(error("Keyword 'program' is not found"));
    token = scan();
    if(token != TNAME) return(error("Program name is not found"));
    token = scan();
    if(token != TSEMI) return(error("Semicolon is not found"));
    token = scan();
    if(block() == ERROR) return(ERROR);
    if(token != TDOT) return(error("Period is not found at the end of program"));
    token = scan();
    return(NORMAL);
}

int block(){
    debug();
    debugPrintf("block\n");
    while(token == TVAR || token == TPROCEDURE){
        if(token == TVAR) CALL(variable_declaration());
        if(token == TPROCEDURE) CALL(subprogram_declaration());
    }
    CALL(compound_statement());
    return(NORMAL);
}

int variable_declaration(){
    debug();
    debugPrintf("variable_declaration\n");
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
    debug();
    debugPrintf("variable_names\n");
    CALL(variable_name());
    while(token == TCOMMA){
        JUDGE(TCOMMA, "Comma is not found");
        CALL(variable_name());
    }
    return(NORMAL);
}

int variable_name(){
    debug();
    debugPrintf("variable_name\n");
    JUDGE(TNAME, "Variable name is not found");
    return(NORMAL);
}

int type(){
    debug();
    debugPrintf("type\n");
    if(token == TINTEGER || token == TBOOLEAN || token == TCHAR) CALL(standard_type());
    else if(token == TARRAY) CALL(array_type());
    else return(ERROR);
    return(NORMAL);
}

int standard_type(){
    debug();
    debugPrintf("standard_type\n");
    if(token == TINTEGER || token == TBOOLEAN || token == TCHAR) token = scan();
    else return(error("Standard type is not found"));
    return(NORMAL);
}

int array_type(){
    debug();
    debugPrintf("array_type\n");
    JUDGE(TARRAY, "Keyword 'array' is not found");
    JUDGE(TLSQPAREN, "'[' is not found");
    JUDGE(TNUMBER, "Index number is not found");
    JUDGE(TRSQPAREN, "']' is not found");
    JUDGE(TOF, "Keyword 'of' is not found");
    CALL(standard_type());
    return(NORMAL);
}

int subprogram_declaration(){
    debug();
    debugPrintf("subprogram_declaration\n");
    JUDGE(TPROCEDURE, "Keyword 'procedure' is not found");
    CALL(procedure_name());
    if(token == TLPAREN) CALL(formal_parameters());
    JUDGE(TSEMI, "Semicolon is not found");
    if(token == TVAR) CALL(variable_declaration());
    CALL(compound_statement());
    JUDGE(TSEMI, "Semicolon is not found");
    return(NORMAL);
}

int procedure_name(){
    debug();
    debugPrintf("procedure_name\n");
    JUDGE(TNAME, "Procedure name is not found");
    return(NORMAL);
}

int formal_parameters(){
    debug();
    debugPrintf("formal_parameters\n");
    JUDGE(TLPAREN, "'(' is not found");
    CALL(variable_names());
    JUDGE(TCOLON, "Colon is not found");
    CALL(type());
    while(token == TSEMI){
        JUDGE(TSEMI, "Semicolon is not found");
        CALL(variable_names());
        JUDGE(TCOLON, "Colon is not found");
        CALL(type());
    }
    JUDGE(TRPAREN, "')' is not found");
    return(NORMAL);
}

int compound_statement(){
    debug();
    debugPrintf("compound_statement\n");
    JUDGE(TBEGIN, "Keyword 'begin' is not found");
    CALL(statement());
    while(token == TSEMI){
        JUDGE(TSEMI, "Semicolon is not found");
        CALL(statement());
    }
    JUDGE(TEND, "Keyword 'end' is not found");
    return(NORMAL);
}

int statement(){
    debug();
    debugPrintf("statement\n");
    if(token == TNAME) CALL(assignment_statement());
    else if(token == TIF) CALL(condition_statement());
    else if(token == TWHILE) CALL(iteration_statement());
    else if(token == TBREAK) CALL(exit_statement());
    else if(token == TCALL) CALL(call_statement());
    else if(token == TRETURN) CALL(return_statement());
    else if(token == TREAD || token == TREADLN) CALL(input_statement());
    else if(token == TWRITE || token == TWRITELN) CALL(output_statement());
    else if(token == TBEGIN) CALL(compound_statement());
    else CALL(empty_statement());
    return(NORMAL);
}

int condition_statement(){
    debug();
    debugPrintf("condition_statement\n");
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
    debug();
    debugPrintf("iteration_statement\n");
    in_while++;
    JUDGE(TWHILE, "Keyword 'while' is not found");
    CALL(expression());
    JUDGE(TDO, "Keyword 'do' is not found");
    CALL(statement());
    in_while--;
    return(NORMAL);
}

int exit_statement(){
    debug();
    debugPrintf("exit_statement\n");
    if(in_while < 1) return(error("Keyword 'break' must be included in at least one iteration sentence."));
    JUDGE(TBREAK, "Keyword 'break' is not found");
    return(NORMAL);
}

int call_statement(){
    debug();
    debugPrintf("call_statement\n");
    JUDGE(TCALL, "Keyword 'call' is not found");
    CALL(procedure_name());
    if(token != TLPAREN) return(NORMAL);
    JUDGE(TLPAREN, "'(' is not found");
    CALL(expressions());
    JUDGE(TRPAREN, "')' is not found");
    return(NORMAL);
}

int expressions(){
    debug();
    debugPrintf("expressions\n");
    CALL(expression());
    while(token == TCOMMA){
        JUDGE(TCOMMA, "Comma is not found");
        CALL(expression());
    }
    return(NORMAL);
}

int return_statement(){
    debug();
    debugPrintf("return_statement\n");
    JUDGE(TRETURN, "Keyword 'return' is not found");
    return(NORMAL);
}

int assignment_statement(){
    debug();
    debugPrintf("assignment_statement\n");
    CALL(left_part());
    JUDGE(TASSIGN, "':=' is not found");
    CALL(expression());
    return(NORMAL);
}

int left_part(){
    debug();
    debugPrintf("left_part\n");
    CALL(variable());
    return(NORMAL);
}

int variable(){
    debug();
    debugPrintf("variable\n");
    CALL(variable_name());
    if(token != TLSQPAREN) return(NORMAL);
    JUDGE(TLSQPAREN, "'[' is not found");
    CALL(expression());
    JUDGE(TRSQPAREN, "']' is not found");
    return(NORMAL);
}

int expression(){
    debug();
    debugPrintf("expression\n");
    CALL(simple_expression());
    while(token == TEQUAL || token == TNOTEQ || token == TLE || token == TLEEQ || token == TGR || token == TGREQ){
        CALL(relational_operator());
        CALL(simple_expression());
    }
    return(NORMAL);
}

int simple_expression(){
    debug();
    debugPrintf("simple_expresssion\n");
    if(token == TPLUS || token == TMINUS) token = scan();
    CALL(term());
    while(token == TPLUS || token == TMINUS || token == TOR){
        CALL(additive_operator());
        CALL(term());
    }
    return(NORMAL);
}

int term(){
    debug();
    debugPrintf("term\n");
    CALL(factor());
    while(token == TSTAR || token == TDIV || token == TAND){
        CALL(multiplicative_operator());
        CALL(factor());
    }
    return(NORMAL);
}

int factor(){
    debug();
    debugPrintf("factor\n");
    if(token == TNAME) CALL(variable());
    else if(token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING) CALL(constant());
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
    return(NORMAL);
}

int constant(){
    debug();
    debugPrintf("constant\n");
    if(token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING) token = scan();
    else return(ERROR);
    return(NORMAL);
}

int multiplicative_operator(){
    debug();
    debugPrintf("multiplicative_operator\n");
    if(token == TSTAR || token == TDIV || token == TAND) token = scan();
    else return(ERROR);
    return(NORMAL);
}

int additive_operator(){
    debug();
    debugPrintf("additive_operator\n");
    if(token == TPLUS || token == TMINUS || token == TOR) token = scan();
    else return(ERROR);
    return(NORMAL);
}

int relational_operator(){
    debug();
    debugPrintf("relational_operator\n");
    if(token == TEQUAL || token == TNOTEQ || token == TLE || token == TLEEQ ||token == TGR || token == TGREQ) token = scan();
    else return(ERROR);
    return(NORMAL);
}

int input_statement(){
    debug();
    debugPrintf("input_statement\n");
    if(token == TREAD || token == TREADLN) token = scan();
    else return(ERROR);
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
    debug();
    debugPrintf("output_statement\n");
    if(token == TWRITE || token == TWRITELN) token = scan();
    else return(ERROR);
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
    debug();
    debugPrintf("output_format\n");
    CALL(expression());
    if(token != TCOLON) return(NORMAL);
    JUDGE(TCOLON, "Colon is not found");
    JUDGE(TNUMBER, "Number is not found");
    return(NORMAL);
}

int empty_statement(){
    debug();
    debugPrintf("empty_statement\n");
    return(NORMAL);
}
/*TODO:return(ERRER)はありえない(必ずerror(mes)でメッセージを出力)
 *     動くかは試したい
 *     */

int error(char *mes){
    printf("¥n ERROR: %s¥n", mes);
    end_scan(); /* 入力ファイルを閉じる */
    return(ERROR);
}

void debug(){
  if(!is_debugmode) return;
  char input[MAXSTRSIZE];
  printf("---Please press any key to continue it---");
  fgets(input, MAXSTRSIZE, stdin);
  return;
}

void debugPrintf(char *message){
  if(!is_debugmode) return;
  printf("%s\n", message);
  fflush(stdout);
  return;
}

void debugPrintChar(char *message, char c){
	if(!is_debugmode) return;
	printf("%s%c\n", message, c);
	fflush(stdout);
	return;
}