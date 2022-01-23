/* token-list.h  */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAXSTRSIZE 1024

#define NORMAL 0
#define ERROR 1

/* Token */
#define	TNAME		1	/* Name : Alphabet { Alphabet | Digit } */
#define	TPROGRAM	2	/* program : Keyword */
#define	TVAR		3	/* var : Keyword */
#define	TARRAY		4	/* array : Keyword */
#define	TOF		5	/* of : Keyword */
#define	TBEGIN		6	/* begin : Keyword */
#define	TEND		7  	/* end : Keyword */
#define	TIF		8  	/* if : Keyword */
#define	TTHEN		9	/* then : Keyword */
#define	TELSE		10	/* else : Keyword */
#define	TPROCEDURE	11	/* procedure : Keyword */
#define	TRETURN		12	/* return : Keyword */
#define	TCALL		13	/* call : Keyword */
#define	TWHILE		14	/* while : Keyword */
#define	TDO		15 	/* do : Keyword */
#define	TNOT		16	/* not : Keyword */
#define	TOR		17	/* or : Keyword */
#define	TDIV		18 	/* div : Keyword */
#define	TAND		19 	/* and : Keyword */
#define	TCHAR		20	/* char : Keyword */
#define	TINTEGER	21	/* integer : Keyword */
#define	TBOOLEAN	22 	/* boolean : Keyword */
#define	TREADLN		23	/* readln : Keyword */
#define	TWRITELN	24	/* writeln : Keyword */
#define	TTRUE		25	/* true : Keyword */
#define	TFALSE		26	/* false : Keyword */
#define	TNUMBER		27	/* unsigned integer */
#define	TSTRING		28	/* String */
#define	TPLUS		29	/* + : symbol */
#define	TMINUS		30 	/* - : symbol */
#define	TSTAR		31 	/* * : symbol */
#define	TEQUAL		32 	/* = : symbol */
#define	TNOTEQ		33 	/* <> : symbol */
#define	TLE		34 	/* < : symbol */
#define	TLEEQ		35 	/* <= : symbol */
#define	TGR		36	/* > : symbol */
#define	TGREQ		37	/* >= : symbol */
#define	TLPAREN		38 	/* ( : symbol */
#define	TRPAREN		39 	/* ) : symbol */
#define	TLSQPAREN	40	/* [ : symbol */
#define	TRSQPAREN	41 	/* ] : symbol */
#define	TASSIGN		42	/* := : symbol */
#define	TDOT		43 	/* . : symbol */
#define	TCOMMA		44	/* , : symbol */
#define	TCOLON		45	/* : : symbol */
#define	TSEMI		46	/* ; : symbol */
#define	TREAD		47	/* read : Keyword */
#define	TWRITE		48	/* write : Keyword */
#define	TBREAK		49	/* break : Keyword */

#define NUMOFTOKEN	49

/* token-list.c */

#define KEYWORDSIZE	28

extern struct KEY {
	char * keyword;
	int keytoken;
} key[KEYWORDSIZE];

extern void scan_error(char *mes);

/* scan.c */
extern int init_scan(char *filename);
extern int scan(void);
extern int cbuf;
extern int num_attr;
extern char string_attr[MAXSTRSIZE];
extern int get_linenum(void);
extern void end_scan(void);
extern void check_line(int is_token);

/* parse.c */
extern int token;
extern int parse_program(char *filename);
extern int block();
extern int variable_declaration();
extern int variable_names();
extern int variable_name();
extern int type();
extern int standard_type();
extern int array_type();
extern int subprogram_declaration();
extern int procedure_name();
extern int formal_parameters();
extern int compound_statement();
extern int statement();
extern int condition_statement();
extern int iteration_statement();
extern int exit_statement();
extern int call_statement();
extern int expressions();
extern int return_statement();
extern int assignment_statement();
extern int left_part();
extern int variable();
extern int expression();
extern int simple_expression();
extern int term();
extern int factor();
extern int constant();
extern int multiplicative_operator();
extern int additive_operator();
extern int relational_operator();
extern int input_statement();
extern int output_statement();
extern int output_format();
extern int empty_statement();
extern void println();

struct PRINTDC{
    char *label;
    char *value;
    struct PRINTDC *nextp;
};

extern char *toOrder(int order);
extern void setLabelL(int n_label, char s_label[6]);
extern void tab();
extern void comma();
extern void ln();
extern void printLabel(char *label);
extern void printOrder(int order);
extern void printRegister(char *reg);
extern void printNumber(int num);
extern void createCodeL(char *label); /*label \n*/
extern void createCodeO(int order);/*  order \n*/
extern void createCodeOL(int order, char *label); /*	order label \n*/
extern void createCodeOR(int order, char *reg);/*   order reg \n*/
extern void createCodeORL(int order, char *reg, char *label);/* order   reg    label \n*/
extern void createCodeOIR(int order, int index, char *reg);/*   order   index,reg \n*/
extern void createCodeORR(int order, char *reg1, char *reg2);/*  order   reg1,   reg2 \n*/
extern void createCodeORI(int order, char *reg, int index);/*  order   reg1,   index \n*/
extern void createCodeORIR(int order, char *reg1, int index, char *reg2);/*  order   reg1,   index,reg2 \n*/
extern void createCodeStart(char *programname);
extern void createCodeDC(char *label, int num);
extern void createCodeDCS(char *label, char *string);
extern void createCodeDS(char *label, int num);
extern void createCodeEnd();/*TODO:Library*/

extern int error();
extern void prettyPrint(int);
extern int push_front_printDC(struct PRINTDC **idroot, struct PRINTDC *p);
extern int init_printDC(struct PRINTDC *p);
extern struct PRINTDC *create_newprintDC();
extern char *create_newlabel(char *name, char *procname);
extern void caslPrint(char* filename);

/*Type*/
#define TPINT	1
#define TPCHAR	2
#define TPBOOL	3
#define TPARRAY	4
#define TPARRAYINT	5
#define TPARRAYCHAR 6
#define TPARRAYBOOL	7
#define TPPROC	8

struct TYPE {
	int ttype; /*TPINT TPCHAR TPBOOL TPPROC*/
	int arraysize; /*size of array, if array*/
	struct TYPE *etp; /*unused*/
	struct TYPE *paratp; /*pointer to parameter's type list if type is TPPROC*/
	struct TYPE *nextp; /*pointer to next parameter's type if parameter's type*/
};

struct LINE {
	int reflinenum;
	struct LINE *nextlinep;
};

struct ID {
	char *name;
	char *procname; /*procedure name within which this name is defined*//*NULL if global name*/
	struct TYPE *itp;
	int ispara; /*1:formal parameter, 0:else(variable)*/
	int deflinenum;
	struct LINE *irefp;
	struct ID *nextp;
};

extern struct ID *globalidroot, *localidroot; /*Pointers to root of global & local symbol tables*/

extern int to_ttype(int token);

extern int is_array(struct TYPE *p);
extern struct ID **get_idroot();
extern int push_front_id(struct ID **idroot, struct ID *p);
extern int push_back_id(struct ID **idroot, struct ID *back);
extern int push_front_type(struct TYPE **type_root, struct TYPE *p);
extern struct TYPE *pop_front_type(struct TYPE **type_root);
extern int push_back_type(struct TYPE **type_root, struct TYPE *back);
extern int push_back_line(struct LINE **line_root, struct LINE *back);
extern int check_operand_type(struct TYPE **temp_type);
extern int init_id(struct ID *p);
extern int init_type(struct TYPE *p);
extern int init_line(struct LINE *p);
extern struct ID *create_newid();
extern struct TYPE *create_newtype();
extern struct LINE *create_newline();
extern char *create_newname(char *np);
extern int release_typetab(struct TYPE **type_root);
extern int release_linetab(struct LINE **line_root);
extern int release_idtab(struct ID **id_root);
extern struct ID *search_id_byname(struct ID *idroot, char *np, char *scope_p);
extern int store_idname(char **temp_id_name, char *np);
extern int store_id_byname(struct ID **temp_id, char *np);
extern int store_standard_type(struct TYPE **temp_type, int ttype);
extern int store_array_type(struct TYPE **temp_type, int array_size);
extern int store_procedure_type(struct ID *temp_procedure);
extern int register_id_bytype(struct ID *temp_id, struct TYPE *temp_type);
extern int register_procedure(struct ID *temp_procedure);
extern int register_parameter_bytype(struct ID *temp_id, struct ID *temp_procedure, struct TYPE *temp_type);
extern int register_procedure_parameter(struct ID *temp_procedure, int ttype);
extern int store_argument(struct TYPE **temp_argument, int ttype);
extern int check_argument(struct ID *procedure, struct TYPE **temp_argument);
extern int ref_newid(struct ID *p);
extern int print_idtab(struct ID *idroot);

extern void swap(struct ID **p, struct ID **q);
extern int compare_id(struct ID *p, struct ID *q);
extern struct ID *sort(struct ID *idroot);

/*order*/
#define LD 0x10
#define ST 0x11
#define LAD 0x12
#define ADDA 0x20
#define ADDL 0x21
#define SUBA 0x22
#define SUBL 0x23
#define MULA 0x24
#define MULL 0x25
#define DIVA 0x26
#define DIVL 0x27
#define AND 0x28
#define OR 0x29
#define XOR 0x2A
#define CPA 0x30
#define CPL 0x31
#define SLA 0x38
#define SRA 0x39
#define SLL 0x3A
#define SRL 0x3B
#define JPL 0x40
#define JMI 0x41
#define JNZ 0x42
#define JZE 0x43
#define JOV 0x44
#define JUMP 0x45
#define PUSH 0x50
#define POP 0x51
#define CALL 0x58
#define RET 0x59
#define SVC 0x60
#define NOP 0x61
#define IN 0x74
#define OUT 0x75
#define RPUSH 0x76
#define RPOP 0x77

#define ORDERSIZE 36

#define gr0 "gr0"
#define gr1 "gr1"
#define gr2 "gr2"
#define gr3 "gr3"
#define gr4 "gr4"
#define gr5 "gr5"
#define gr6 "gr6"
#define gr7 "gr7"