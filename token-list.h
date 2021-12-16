﻿/* token-list.h  */
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
extern void init_parse();
extern int parse_program();
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
extern int error();
extern void prettyPrint(int);

/*Type*/
#define TPINT	1
#define TPCHAR	2
#define TPBOOL	3
#define TPARRAY	4
#define TPARRAYINT	5
#define TPARRAYCHAR 6
#define TPARRAYBOOL	7
#define TPPROC	8

/*
extern struct TYPE {
	int ttype;
	int arraysize;
	struct TYPE *etp;
	struct TYPE *paratp;
	struct TYPE *nextp;
};

extern struct LINE {
	int reflinenum;
	struct LINE *nextlinep;
};
*/

extern struct ID *globalidroot, *localidroot; /*Pointers to root of global & local symbol tables*/

extern int to_ttype(int token);
extern int push_front_id(struct ID **idroot, struct ID *p);
extern int push_back_id(struct ID **idroot, struct ID *back);
/*extern int push_front_type(struct TYPE **type_root, struct TYPE *p);*/
extern struct TYPE *pop_front_type(struct TYPE **type_root);
extern int push_back_type(struct TYPE **type_root, struct TYPE *back);
/*extern int push_back_line(struct LINE **line_root, struct LINE *back);*/
extern int init_tab(void **root);
extern int init_id(struct ID *p);
extern int init_type(struct TYPE *p);
/*extern int init_line(struct LINE *p);*/
extern struct ID *create_newid();
extern struct TYPE *create_newtype();
extern struct LINE *create_newline();
extern char *create_newname(char *np);
extern int release_typetab(struct TYPE **type_root);
extern int release_linetab(struct LINE **line_root);
extern int release_idtab(struct ID **id_root);
extern struct ID *search_id_byname(struct ID *idroot, char *np);
extern int store_idname(char *temp_id_name, char *np);
extern int store_id_byname(struct ID **temp_id, char *np);
extern int store_standard_type(struct TYPE **temp_type, int ttype);
extern int store_array_type(struct TYPE **temp_type, int array_size);
extern int store_procedure_type(struct TYPE **temp_type);
extern int register_id_bytype(struct ID *temp_id, struct TYPE *temp_type);
extern int register_procedure_parameter(struct ID *temp_procedure, int ttype);
extern int store_argument(struct TYPE **temp_argument, int ttype);
extern int check_argument(struct ID *temp_procedure, struct TYPE **temp_argument);
extern int check_newid(char *np);
extern int print_idtab(struct ID *idroot);