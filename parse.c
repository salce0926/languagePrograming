#include "token-list.h"

#define FCALL(function) if(function == ERROR) return(ERROR)
#define JUDGE(code, string) if(token != code) return(error(string));/*prettyPrint(token);*/token = scan()

FILE *fq;
int n_label = 1;
struct PRINTDC *list = NULL;
int label_stack[MAXSTACKSIZE];
int stack_i = 0;

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

/*order list*/
struct KEY orderkey[ORDERSIZE] = {
    {"LD", LD},
    {"ST", ST},
    {"LAD", LAD},
    {"ADDA", ADDA},
    {"ADDL", ADDL},
    {"SUBA", SUBA},
    {"SUBL", SUBL},
    {"MULA", MULA},
    {"MULL", MULL},
    {"DIVA", DIVA},
    {"DIVL", DIVL},
    {"AND", AND},
    {"OR", OR},
    {"XOR", XOR},
    {"CPA", CPA},
    {"CPL", CPL},
    {"SLA", SLA},
    {"SRA", SRA},
    {"SLL", SLL},
    {"SRL", SRL},
    {"JPL", JPL},
    {"JMI", JMI},
    {"JNZ", JNZ},
    {"JZE", JZE},
    {"JOV", JOV},
    {"JUMP", JUMP},
    {"PUSH", PUSH},
    {"POP", POP},
    {"CALL", CALL},
    {"RET", RET},
    {"SVC", SVC},
    {"NOP", NOP},
    {"IN", IN},
    {"OUT", OUT},
    {"RPUSH", RPUSH},
    {"RPOP", RPOP},
};

int parse_program(char *filename) {
    char *p = create_newname(filename);
    char *casl_name = strtok(p, ".");
    strcat(casl_name, ".csl");
    if((fq = fopen(casl_name, "w")) == NULL){
        printf("File %s can not open.\n", casl_name);
        return;
    }
    JUDGE(TPROGRAM, "Keyword 'program' is not found");
    JUDGE(TNAME, "Program name is not found");
    createCodeStart(string_attr);
    JUDGE(TSEMI, "Semicolon is not found");
    FCALL(block());
    JUDGE(TDOT, "Period is not found at the end of program");
    createCodeEnd();
    fclose(fq);
    return(NORMAL);
}

int block(){
    while(token == TVAR || token == TPROCEDURE){
        if(token == TVAR) FCALL(variable_declaration());
        if(token == TPROCEDURE) FCALL(subprogram_declaration());
    }
    indent_count = 0;
    createCodeL("L0001");
    FCALL(compound_statement());
    createCodeO(RET);
    return(NORMAL);
}

int variable_declaration(){
    struct ID *p;
    char *label;
    indent_count = 1;
    JUDGE(TVAR, "Keyword 'var' is not found");
    FCALL(variable_names());
    JUDGE(TCOLON, "Colon is not found");
    FCALL(type());
    FCALL(register_id_bytype(temp_id, temp_type));
    for(p = temp_id; p != NULL; p = p->nextp){
        label = create_newlabel(p->name, p->procname);
        if(is_array(p->itp)){
            createCodeDS(label, p->itp->arraysize);
        }else{
            createCodeDC(label, 0);
        }
        free(label);
        label = NULL;
    }
    temp_id = NULL;
    temp_type = NULL;
    JUDGE(TSEMI, "Semicolon is not found");
    while(token == TNAME){
        FCALL(variable_names());
        JUDGE(TCOLON, "Colon is not found");
        FCALL(type());
        FCALL(register_id_bytype(temp_id, temp_type));
        for(p = temp_id; p != NULL; p = p->nextp){
            label = create_newlabel(p->name, p->procname);
            if(is_array(p->itp)){
                createCodeDS(label, p->itp->arraysize);
            }else{
                createCodeDC(label, 0);
            }
            free(label);
            label = NULL;
        }
        temp_id = NULL;
        temp_type = NULL;
        JUDGE(TSEMI, "Semicolon is not found");
    }
    return(NORMAL);
}

int variable_names(){
    FCALL(variable_name());
    FCALL(store_id_byname(&temp_id, temp_id_name));
    free(temp_id_name);
    temp_id_name = NULL;
    while(token == TCOMMA){
        JUDGE(TCOMMA, "Comma is not found");
        FCALL(variable_name());
        FCALL(store_id_byname(&temp_id, temp_id_name));
        free(temp_id_name);
        temp_id_name = NULL;
    }
    return(NORMAL);
}

int variable_name(){
    FCALL(store_idname(&temp_id_name, string_attr));
    JUDGE(TNAME, "Variable name is not found");
    string_attr[0] = '\0';/*just in case*/
    return(NORMAL);
}

int type(){
    if(token == TINTEGER || token == TBOOLEAN || token == TCHAR){
        FCALL(standard_type());
    }
    else if(token == TARRAY){
        FCALL(array_type());
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
    FCALL(store_standard_type(&temp_type, type));
    return(NORMAL);
}

int array_type(){
    JUDGE(TARRAY, "Keyword 'array' is not found");
    JUDGE(TLSQPAREN, "'[' is not found");
    JUDGE(TNUMBER, "Index number is not found");
    JUDGE(TRSQPAREN, "']' is not found");
    JUDGE(TOF, "Keyword 'of' is not found");
    FCALL(standard_type());
    FCALL(store_array_type(&temp_type, num_attr));
    return(NORMAL);
}

int subprogram_declaration(){
    char subprogramname[MAXSTRSIZE];
    indent_count = 1;
    JUDGE(TPROCEDURE, "Keyword 'procedure' is not found");
    FCALL(procedure_name());
    FCALL(store_id_byname(&temp_procedure, temp_id_name));
    FCALL(store_procedure_type(temp_procedure));
    scope_p = create_newname(temp_id_name);
    free(temp_id_name);
    temp_id_name = NULL;

    strcpy(subprogramname, "$");
    strcat(subprogramname, scope_p);
    createCodeL(subprogramname);
    createCodeOR(POP, gr2);

    if(token == TLPAREN) FCALL(formal_parameters());
    FCALL(register_procedure(temp_procedure));
    temp_procedure = NULL;
    JUDGE(TSEMI, "Semicolon is not found");
    if(token == TVAR) FCALL(variable_declaration());
    indent_count = 1;

    createCodeOIR(PUSH, 0, gr2);

    FCALL(compound_statement());
    JUDGE(TSEMI, "Semicolon is not found");

    createCodeO(RET);

    free(scope_p);
    scope_p = NULL;
    return(NORMAL);
}

int procedure_name(){
    FCALL(store_idname(&temp_id_name, string_attr));
    JUDGE(TNAME, "Procedure name is not found");
    string_attr[0] = '\0';/*just in case*/
    return(NORMAL);
}

int formal_parameters(){
    struct ID *p, *para_idlist;
    struct REV *q, *rev_root = NULL;
    char *label;
    JUDGE(TLPAREN, "'(' is not found");
    FCALL(variable_names());
    JUDGE(TCOLON, "Colon is not found");
    FCALL(standard_type());
    FCALL(register_parameter_bytype(temp_id, temp_procedure, temp_type));
    para_idlist = temp_id;
    temp_id = NULL;
    temp_type = NULL;
    while(token == TSEMI){
        JUDGE(TSEMI, "Semicolon is not found");
        FCALL(variable_names());
        JUDGE(TCOLON, "Colon is not found");
        FCALL(standard_type());
        FCALL(register_parameter_bytype(temp_id, temp_procedure, temp_type));
        temp_id = NULL;
        temp_type = NULL;
    }
    for(p = para_idlist; p != NULL && p->ispara == 1; p = p->nextp){
        q = create_newREV();
        q->idp = p;
        push_front_REV(&rev_root, q);
    }

    for(q = rev_root; q != NULL; q = q->nextp){
        label = create_newlabel(q->idp->name, q->idp->procname);
        createCodeDC(label, 0);
        createCodeOR(POP, gr1);
        createCodeORL(ST, gr1, label);
        free(label);
        label = NULL;
    }
    rev_root = NULL;
    JUDGE(TRPAREN, "')' is not found");
    return(NORMAL);
}

int compound_statement(){
    JUDGE(TBEGIN, "Keyword 'begin' is not found");
    indent_count++;
    FCALL(statement());
    while(token == TSEMI){
        JUDGE(TSEMI, "Semicolon is not found");
        FCALL(statement());
    }
    indent_count--;
    JUDGE(TEND, "Keyword 'end' is not found");
    return(NORMAL);
}

int statement(){
    if(token == TNAME){
        FCALL(assignment_statement());
    } 
    else if(token == TIF){
        FCALL(condition_statement());
    } 
    else if(token == TWHILE){
        FCALL(iteration_statement());
    }
    else if(token == TBREAK){
        FCALL(exit_statement());
    }
    else if(token == TCALL){
        FCALL(call_statement());
    }
    else if(token == TRETURN){
        FCALL(return_statement());
    }
    else if(token == TREAD || token == TREADLN){
        FCALL(input_statement());
    }
    else if(token == TWRITE || token == TWRITELN){
        FCALL(output_statement());
    }
    else if(token == TBEGIN){
        FCALL(compound_statement());
    }
    else{
        FCALL(empty_statement());
    }
    return(NORMAL);
}

int condition_statement(){
    struct TYPE *p;
    char true_label[6], false_label[6];
    JUDGE(TIF, "Keyword 'if' is not found");
    FCALL(expression());
    p = pop_front_type(&temp_type);
    if(p->ttype != TPBOOL) return(error("the type of expressions must be boolean"));
    release_typetab(&p);

    setLabelL(n_label++, false_label);
    createCodeOR(POP, gr1);
    createCodeORR(CPA, gr1, gr0);
    createCodeOL(JZE, false_label);
    JUDGE(TTHEN, "Keyword 'then' is not found");
    FCALL(statement());
    setLabelL(n_label++, true_label);
    createCodeOL(JUMP, true_label);
    createCodeL(false_label);
    if(token == TELSE){
        JUDGE(TELSE, "Keyword 'else' is not found");
        FCALL(statement());
    }
    createCodeL(true_label);
    return(NORMAL);
}

int iteration_statement(){
    struct TYPE *p;
    char true_label[6], false_label[6];
    in_while++;
    JUDGE(TWHILE, "Keyword 'while' is not found");
    FCALL(expression());
    p = pop_front_type(&temp_type);
    if(p->ttype != TPBOOL) return(error("the type of expressions must be boolean"));
    release_typetab(&p);
    JUDGE(TDO, "Keyword 'do' is not found");
    setLabelL(n_label++, true_label);
    createCodeL(true_label);
    label_stack[stack_i++] = n_label;
    setLabelL(n_label++, false_label);
    createCodeOR(POP, gr1);
    createCodeORR(CPA, gr1, gr0);
    createCodeOL(JZE, false_label);
    FCALL(statement());
    createCodeOL(JUMP, true_label);
    createCodeL(false_label);
    stack_i--;
    in_while--;
    return(NORMAL);
}

int exit_statement(){
    char label[6];
    if(in_while < 1) return(error("Keyword 'break' must be included in at least one iteration sentence."));
    JUDGE(TBREAK, "Keyword 'break' is not found");
    setLabelL(label_stack[--stack_i], label);
    stack_i--;
    createCodeOL(JUMP, label);
    return(NORMAL);
}

int call_statement(){
    struct ID *proc;
    JUDGE(TCALL, "Keyword 'call' is not found");
    FCALL(procedure_name());
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
    FCALL(ref_newid(proc));
    if(token != TLPAREN){
        FCALL(check_argument(temp_procedure, &temp_argument));
    }else{
        JUDGE(TLPAREN, "'(' is not found");
        FCALL(expressions());
        JUDGE(TRPAREN, "')' is not found");
        FCALL(check_argument(proc, &temp_argument));
    }
    release_typetab(&temp_argument);
    createCodeOL(CALL, create_newlabel(proc->name, NULL));
    return(NORMAL);
}

int expressions(){
    struct TYPE *p;
    FCALL(expression());
    p = pop_front_type(&temp_type);
    FCALL(store_argument(&temp_argument, p->ttype));
    release_typetab(&p);
    while(token == TCOMMA){
        JUDGE(TCOMMA, "Comma is not found");
        FCALL(expression());
        p = pop_front_type(&temp_type);
        FCALL(store_argument(&temp_argument, p->ttype));
        release_typetab(&p);
    }
    return(NORMAL);
}

int return_statement(){
    JUDGE(TRETURN, "Keyword 'return' is not found");
    createCodeO(RET);
    return(NORMAL);
}

int assignment_statement(){
    FCALL(left_part());
    JUDGE(TASSIGN, "':=' is not found");
    FCALL(expression());
    FCALL(check_operand_type(&temp_type));
    temp_type = NULL;
    return(NORMAL);
}

int left_part(){   
    FCALL(variable());
    return(NORMAL);
}

int variable(){
    struct ID *list, *p;
    struct TYPE *q;
    FCALL(variable_name());
    list = *get_idroot();
    p = search_id_byname(list, temp_id_name, scope_p);
    if(scope_p != NULL && p == NULL){
        p = search_id_byname(globalidroot, temp_id_name, NULL);
    }
    free(temp_id_name);
    temp_id_name = NULL;
    if(p == NULL || p->itp->ttype == TPPROC) return(error("this variable name is undifined\n"));
    FCALL(store_standard_type(&temp_type, p->itp->ttype));
    if(is_array(p->itp)){
        FCALL(store_array_type(&temp_type, p->itp->arraysize));
    }
    FCALL(ref_newid(p));
    if(token != TLSQPAREN) return(NORMAL);
    if(!is_array(p->itp)){
        return(error("the type of this id must be array\n"));
    }
    JUDGE(TLSQPAREN, "'[' is not found");
    FCALL(expression());
    q = pop_front_type(&temp_type);
    if(q->ttype != TPINT) return(error("the type of index must be integer\n"));
    release_typetab(&q);
    JUDGE(TRSQPAREN, "']' is not found");
    temp_type->arraysize = -1;
    return(NORMAL);
}

int expression(){
    struct TYPE *p;
    FCALL(simple_expression());
    while(token == TEQUAL || token == TNOTEQ || token == TLE || token == TLEEQ || token == TGR || token == TGREQ){
        FCALL(relational_operator());
        FCALL(simple_expression());
        FCALL(check_operand_type(&temp_type));
        p = pop_front_type(&temp_type);
        release_typetab(&p);
        FCALL(store_standard_type(&temp_type, TPBOOL));
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
        FCALL(store_standard_type(&temp_type, TPINT));
    }
    FCALL(term());
    if(check){
        FCALL(check_operand_type(&temp_type));
    }
    while(token == TPLUS || token == TMINUS || token == TOR){
        FCALL(additive_operator());
        FCALL(term());
        FCALL(check_operand_type(&temp_type));
    }
    return(NORMAL);
}

int term(){
    FCALL(factor());
    while(token == TSTAR || token == TDIV || token == TAND){
        FCALL(multiplicative_operator());
        FCALL(factor());
        FCALL(check_operand_type(&temp_type));
    }
    return(NORMAL);
}

int factor(){
    struct TYPE *p;
    if(token == TNAME){
        FCALL(variable());
    }
    else if(token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING){
        FCALL(constant());
    }
    else if(token == TLPAREN){
        JUDGE(TLPAREN, "'(' is not found");
        FCALL(expression());
        JUDGE(TRPAREN, "')' is not found");
    }
    else if(token == TNOT){
        JUDGE(TNOT, "Keyword 'not' is not found");
        FCALL(factor());
        if(temp_type->ttype != TPBOOL){
            return(error("unsupported operand type for 'not'\n"));
        }
    }
    else if(token == TINTEGER || token == TBOOLEAN || token == TCHAR){
        FCALL(standard_type());
        JUDGE(TLPAREN, "'(' is not found");
        FCALL(expression());
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
            FCALL(store_standard_type(&temp_type, TPINT));
                createCodeORI(LAD, gr1, num_attr);
        }else if(token == TFALSE){
            JUDGE(TFALSE, "Keyword 'false' is not found");
            FCALL(store_standard_type(&temp_type, TPBOOL));
                createCodeORR(LD, gr1, gr0);
        }else if(token == TTRUE){
            JUDGE(TTRUE, "Keyword 'true' is not found");
            FCALL(store_standard_type(&temp_type, TPBOOL));
                createCodeORI(LAD, gr1, 1);
        }else if(token == TSTRING){
            length = strlen(string_attr);
            JUDGE(TSTRING, "String is not found");
            if(length != 1) return(error("the length of the string must be 1\n"));
            FCALL(store_standard_type(&temp_type, TPCHAR));
                createCodeORI(LAD, gr1, string_attr[0]);
        }
    }
    createCodeOIR(PUSH, 0, gr1);
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
    int has_ln = 0;
    if(token == TREAD || token == TREADLN){
        if(token == TREAD){
            JUDGE(TREAD, "Keyword 'read' is not found");
        }else if(token == TREADLN){
            has_ln = 1;
            JUDGE(TREADLN, "Keyword 'readln' is not found");
        }
    }
    if(token == TLPAREN){
        JUDGE(TLPAREN, "'(' is not found");
        FCALL(variable());
        p = pop_front_type(&temp_type);
        if(p->ttype != TPINT && p->ttype != TPCHAR) return(error("the type of variable must be integer or char\n"));
        createCodeOR(POP, gr1);
        if(p->ttype == TPINT){
            createCodeOL(CALL, "READINT");
        }else if(p->ttype == TPCHAR){
            createCodeOL(CALL, "READCHAR");
        }
        release_typetab(&p);
        while(token == TCOMMA){
            JUDGE(TCOMMA, "Comma is not found");
            FCALL(variable());
            p = pop_front_type(&temp_type);
            if(p->ttype != TPINT && p->ttype != TPCHAR) return(error("the type of variable must be integer or char\n"));
            createCodeOR(POP, gr1);
            if(p->ttype == TPINT){
                createCodeOL(CALL, "READINT");
            }else if(p->ttype == TPCHAR){
                createCodeOL(CALL, "READCHAR");
            }
            release_typetab(&p);
        }
        JUDGE(TRPAREN, "')' is not found");
    }
    if(has_ln) createCodeOL(CALL, "READLINE");
    return(NORMAL);
}

int output_statement(){
    int has_ln = 0;
    if(token == TWRITE || token == TWRITELN){
        if(token == TWRITE){
            JUDGE(TWRITE, "Keyword 'write' is not found");
        }else if(token == TWRITELN){
            has_ln = 1;
            JUDGE(TWRITELN, "Keyword 'writeln' is not found");
        }
    }
    if(token == TLPAREN){
        JUDGE(TLPAREN, "'(' is not found");
        FCALL(output_format());
        while(token == TCOMMA){
            JUDGE(TCOMMA, "Comma is not found");
            FCALL(output_format());
        }
        JUDGE(TRPAREN, "')' is not found");
    }
    if(has_ln) createCodeOL(CALL, "WRITELINE");
    return(NORMAL);
}

int output_format(){
    struct TYPE *p;
    char string[MAXSTRSIZE];
    char s_label[6];
    if(token == TSTRING && strlen(string_attr) != 1){
        JUDGE(TSTRING, "String is not found");
        string_attr[1000] = '\0';
        strcpy(string, "'");
        strcat(string, string_attr);
        strcat(string, "'");
        setLabelL(n_label++, s_label);
        createCodeDCS(s_label, string);
        createCodeORL(LAD, gr1, s_label);
        createCodeOL(CALL, "WRITESTR");
        return(NORMAL);
    }
    FCALL(expression());
    p = pop_front_type(&temp_type);
    if(is_array(p)) return(error("using array type as output format is not allowed\n"));
    createCodeOR(POP, gr1);
    if(token != TCOLON){
        createCodeORR(LD, gr2, gr0);
    }else{
        JUDGE(TCOLON, "Colon is not found");
        JUDGE(TNUMBER, "Number is not found");
        createCodeORI(LAD, gr2, num_attr);
    }
    if(p->ttype == TPINT){
        createCodeOL(CALL, "WRITEINT");
    }else if(p->ttype == TPCHAR){
        createCodeOL(CALL, "WRITECHAR");
    }else if(p->ttype = TPBOOL){
        createCodeOL(CALL, "WRITEBOOL");
    }
    release_typetab(&p);
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

int init_REV(struct REV *p){
    p->idp = NULL;
    p->nextp = NULL;
    return(NORMAL);
}

struct REV *create_newREV(){
	struct REV *p;
	if((p = (struct REV *)malloc(sizeof(struct REV))) == NULL) {
		printf("can not malloc in create_newREV\n");
		return(NULL);
	}
	init_REV(p);
	return(p);
}

int release_REVtab(struct REV **rev_root){
	struct REV *p, *q;

	for(p = *rev_root; p != NULL; p = q){
		q = p->nextp;
		free(p);
	}
	
	*rev_root = NULL;
	return(NORMAL);
}

int push_front_REV(struct REV **rev_root, struct REV *front){
	front->nextp = *rev_root;
	*rev_root = front;
	return(NORMAL);
}

int push_front_printDC(struct PRINTDC **idroot, struct PRINTDC *p){
	p->nextp = *idroot;
	*idroot = p;
	return(NORMAL);
}

int init_printDC(struct PRINTDC *p){
    p->label = NULL;
    p->value = NULL;
    p->nextp = NULL;
    return(NORMAL);
}

struct PRINTDC *create_newprintDC(){
	struct PRINTDC *p;
	if((p = (struct PRINTDC *)malloc(sizeof(struct PRINTDC))) == NULL) {
		printf("can not malloc in create_newprintDC\n");
		return(NULL);
	}
	init_printDC(p);
	return(p);
}

char *create_newlabel(char *name, char *procname){
	char *p;
	if(name == NULL){
		return(NULL);
	}
    if(procname == NULL){
        if((p = (char *)malloc(1+strlen(name)+1)) == NULL){
            printf("can not malloc in create_newlabel\n");
            return(NULL);
        }
        strcpy(p, "$");
        strcat(p, name);
    }else{
        if((p = (char *)malloc(1+strlen(name)+1+strlen(procname)+1)) == NULL){
            printf("can not malloc in create_newname\n");
            return(NULL);
        }
        strcpy(p, "$");
        strcat(p, name);
        strcat(p, "%");
        strcat(p, procname);
    }
	return(p);
}

char *toOrder(int order){
    int i;
    for(i = 0; i < ORDERSIZE; i++){
        if(order == orderkey[i].keytoken){
            return orderkey[i].keyword;
        }
    }
    return "toOrder error";
}

void setLabelL(int n_label, char s_label[6]){
    sprintf(s_label, "L%04d", n_label);
    return;
}

void tab(){
    fprintf(fq, "\t");
    return;
}

void comma(){
    fprintf(fq, ",");
}

void ln(){
    fprintf(fq, "\n");
}

void printLabel(char *label){
    fprintf(fq, "%s", label);
    return;
}

void printOrder(int order){
    fprintf(fq, "%-5s", toOrder(order));
    return;
}

void printRegister(char *reg){
    fprintf(fq, "%s", reg);
    return;
}

void printNumber(int num){
    fprintf(fq, "%d", num);
    return;
}

void createCodeL(char *label){ /*label \n*/
    printLabel(label); ln();
    return;
}

void createCodeO(int order){/*  order \n*/
    tab(); printOrder(order); ln();
    return;
}

void createCodeOL(int order, char *label){/*   order label \n*/
    tab(); printOrder(order); tab(); printLabel(label); ln();
    return;
}

void createCodeOR(int order, char *reg){/*   order reg \n*/
    tab(); printOrder(order); tab(); printRegister(reg); ln();
    return;
}

void createCodeORL(int order, char *reg, char *label){/* order   reg,    label \n*/
    tab(); printOrder(order); tab(); printRegister(reg); comma(); tab(); printLabel(label); ln();
    return;
}

void createCodeOIR(int order, int index, char *reg){/*   order   index,reg \n*/
    tab(); printOrder(order); tab(); printNumber(index); comma(); printRegister(reg); ln();
    return;
}

void createCodeORR(int order, char *reg1, char *reg2){/*  order   reg1,   reg2 \n*/
    tab(); printOrder(order); tab(); printRegister(reg1); comma(); tab(); printRegister(reg2); ln();
    return;
}

void createCodeORI(int order, char *reg, int index){/*  order   reg,   index \n*/
    tab(); printOrder(order); tab(); printRegister(reg); comma(); tab();printNumber(index); ln();
    return; 
}

void createCodeORIR(int order, char *reg1, int index, char *reg2){/*  order   reg1,   index,reg2 \n*/
    tab(); printOrder(order); tab(); printRegister(reg1); comma(); tab();printNumber(index); comma(); printRegister(reg2); ln();
    return; 
}

void createCodeStart(char *programname){
    char s_label[6];
    char program_name_label[MAXSTRSIZE];
    sprintf(program_name_label, "$$%s", programname);
    printLabel(program_name_label); tab(); fprintf(fq, "START"); ln();
    createCodeORI(LAD, gr0, 0);
    setLabelL(n_label++, s_label);
    createCodeOL(CALL, s_label);
    createCodeOL(CALL, "FLUSH");
    tab(); printOrder(SVC); tab(); printNumber(0); ln();
    return;
}

void createCodeDC(char *label, int num){
    char s_label[6];
    setLabelL(n_label++, s_label);
    createCodeOL(JUMP, s_label);
    printLabel(label); tab(); fprintf(fq, "DC"); tab(); printNumber(num); ln();
    createCodeL(s_label);
    return;
}

void createCodeDCS(char *label, char *string){
    char s_label[6];
    setLabelL(n_label++, s_label);
    createCodeOL(JUMP, s_label);
    printLabel(label); tab(); fprintf(fq, "DC"); tab(); printLabel(string); ln();
    createCodeL(s_label);
    return;
}

void createCodeDS(char *label, int num){
    char s_label[6];
    setLabelL(n_label++, s_label);
    createCodeOL(JUMP, s_label);
    printLabel(label); tab(); fprintf(fq, "DS"); tab(); printNumber(num); ln();
    createCodeL(s_label);
    return;
}

void createCodeEnd(){/*TODO:Library*/
    tab(); fprintf(fq, "END"); ln();
    return;
}

int error(char *mes){
    printf("\n in line %d, ERROR: %s\n", get_linenum(), mes);
    end_scan(); 
    return(ERROR);
}