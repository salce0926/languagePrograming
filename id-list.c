#include "token-list.h"

extern char *scope_p;
extern struct ID *temp_id;
extern struct TYPE *temp_type;
extern struct ID *temp_procedure;
extern struct ID *globalidroot;
extern struct ID *localidroot;
extern int error(char *mes);

struct TYPE {
	int ttype;
	int arraysize;/*size of array, if TPARRAY*/
	struct TYPE *etp;/*pointer to element type if TPARRAY*/
	struct TYPE *paratp;/*pointer to parameter's type list if ttype is TPPROC*/
	struct TYPE *nextp;/*pointer to next parameter's type*/
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

void push_front_id(struct ID **idroot, struct ID *p){
	p->nextp = *idroot;
	*idroot = p;
}

void push_front_type(struct TYPE **type_root, struct TYPE *p){
	p->nextp = *type_root;
	*type_root = p;
}

void push_back_type(struct TYPE **type_root, struct TYPE *back){
	struct TYPE *p;
	if(*type_root == NULL){
		*type_root = back;
		return;
	}
	for(p = *type_root; p->nextp != NULL; p = p->nextp);
	p->nextp = back;
}

void push_back_line(struct LINE **line_root, struct LINE *back){
	struct LINE *p;
	if(*line_root == NULL){
		*line_root = back;
		return;
	}
	for(p = *line_root; p->nextlinep != NULL; p = p->nextlinep);
	p->nextlinep = back;
}

void init_tab(void *root) {		/* Initialise the table */
	root = NULL;
}

void init_id(struct ID *p){
	p->name = NULL;
	p->procname = NULL;
	p->itp = NULL;
	p->ispara = -1;
	p->deflinenum = -1;
	p->irefp = NULL;
	p->nextp = NULL;
}

void init_type(struct TYPE *p){
	p->ttype = -1;
	p->arraysize = -1;
	p->etp = NULL;
	p->paratp = NULL;
	p->nextp = NULL;
}

void init_line(struct LINE *p){
	p->reflinenum = -1;
	p->nextlinep = NULL;
}

struct ID *create_newid(){
	struct ID *p;
	if((p = (struct ID *)malloc(sizeof(struct ID))) == NULL) {
		printf("can not malloc in create_newid\n");
		return(NULL);
	}
	init_id(p);
	return(p);
}

struct TYPE *create_newtype(){
	struct TYPE *p;
	if((p = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
		printf("can not malloc in create_newtype\n");
		return(NULL);
	}
	init_type(p);
	return(p);
}

struct LINE *create_newline(){
	struct LINE *p;
	if((p = (struct LINE *)malloc(sizeof(struct LINE))) == NULL) {
		printf("can not malloc in create_newline\n");
		return(NULL);
	}
	init_line(p);
	return(p);
}

char *create_newname(char *np){
	char *p;
	if((p = (char *)malloc(strlen(np)+1)) == NULL){
		printf("can not malloc in create_newname\n");
		return(NULL);
	}
	strcpy(p, np);
	return(p);
}

/* search the name pointed by np */
struct ID *search_id_byname(struct ID *idroot, char *np) {	
	struct ID *p;

	for(p = idroot; p != NULL; p = p->nextp) {
		if(strcmp(np, p->name) == 0) return(p);
	}
	return(NULL);
}

void store_id_byname(struct ID **temp_id, char *np){
	struct ID *p, *idroot;
	idroot = (scope_p == NULL ? globalidroot : localidroot);
	if((p = search_id_byname(idroot, np)) != NULL && strcmp(p->procname, scope_p) == 0){
		error("this ID already exists in same scope\n");
		return;
	}

	p = create_newid();
	p->name = create_newname(np);
	p->procname = create_newname(scope_p);
	p->deflinenum = get_linenum();

	push_front_id(temp_id, p);
	return;
}

void store_standard_type(struct TYPE **temp_type, int ttype){
	struct TYPE *p = create_newtype();

	p->ttype = ttype;

	push_front_type(temp_type, p);
	return;
}

void store_array_type(struct TYPE **temp_type, int array_size, int element_type){
	struct TYPE *p = create_newtype();

	p->ttype = TPARRAY;
	p->arraysize = array_size;
	p->etp = create_newtype();
	p->etp->ttype = element_type;

	push_front_type(temp_type, p);
	return;
}

void store_procedure_type(struct TYPE **temp_type){
	struct TYPE *p = create_newtype();

	p->ttype = TPPROC;

	push_front_type(temp_type, p);
	return;
}

void register_id_bytype(struct ID **idroot, struct ID **temp_id, struct TYPE *temp_type){
	struct ID *p;
	for(p = *temp_id; p != NULL; p = p->nextp){
		p->itp = create_newtype();
		*(p->itp) = *temp_type;
	}
	for(p = *temp_id; p->nextp != NULL; p = p->nextp);
	p->nextp = *idroot;
	*idroot = *temp_id;
	return;
}

void register_procedure_parameter(struct ID *temp_procedure, int ttype){
	struct TYPE *p = create_newtype();
	
	p->ttype = ttype;

	push_back_type(&(temp_procedure->itp->paratp), p);
	return;
}

void store_argument(struct TYPE **temp_argument, int ttype){
	struct TYPE *p = create_newtype();

	p->ttype = ttype;

	push_back_type(temp_argument, p);
	return;
}

void check_argument(struct ID *temp_procedure, struct TYPE *temp_argument){
	struct TYPE *parameter = temp_procedure->itp->paratp;
	struct TYPE *argument = temp_argument;
	while(parameter != NULL && argument != NULL){
		if(parameter == NULL || argument == NULL){
			error("the number of the argument does not match the number of the parameter\n");
			return;
		}
		if(parameter->ttype != argument->ttype){
			error("the type of the argument does not match the difinition of the parameter\n");
			return;
		}

		parameter = parameter->nextp;
		argument = argument->nextp;
	}
	release_typetab(temp_argument); 
	return;
}

void check_newid(char *np){
	struct ID *p, *idroot;
	struct LINE *l;
	
	idroot = (scope_p == NULL ? globalidroot : localidroot);

	if((p = search_id_byname(idroot, np)) == NULL){
		error("the difinition of this identifier is not found");
		return;
	}

	l = create_newline();

	l->reflinenum = get_linenum();

	push_back_line(&(p->irefp), l);
	return;
}

/*
void id_countup(char *np) {
	struct ID *p;
	char *cp;

	if((p = search_idtab(np)) != NULL) p->count++;
	else {
		if((p = (struct ID *)malloc(sizeof(struct ID))) == NULL) {
			printf("can not malloc in id_countup\n");
			return;
		}
		if((cp = (char *)malloc(strlen(np)+1)) == NULL) {
			printf("can not malloc-2 in id_countup\n");
			return;
		}
		strcpy(cp, np);
		p->name = cp;
		p->count = 1;
		p->nextp = idroot;
		idroot = p;
	}
}
*/

void print_idtab(struct ID *idroot) {	/* Output the registered data */
	struct ID *p;
	struct TYPE *q;
	struct LINE *r;
	int name_length = 20;
	int type_length = 30;
	int define_length = 4;
	int reference_length = 30;
	int temp_length = 0;

	char *type_str[8+1] = {
		"", 
		"integer", "char", "boolean", "array", 
		"array of int", "array of char", "array of boolean", "procedure"
	};

	printf("--------------------------------------------------------------------------\n");
	printf("%*s", -name_length, "Name");
	printf("%*s", -type_length, "Type");
	printf("%*s", define_length, "Def.");
	printf("%*s\n", -reference_length, "Ref.");

	for(p = idroot; p != NULL; p = p->nextp) {
		if(p->procname == NULL){
			printf("%*s", -name_length, "Name");
		}else{
			printf("%s", p->name);
			printf(":%*s", -(name_length - strlen(p->name) + 1), p->procname);
		}

		if(p->itp->ttype == TPINT || p->itp->ttype == TPCHAR || p->itp->ttype == TPBOOL){
			printf("%*s", -type_length, type_str[p->itp->ttype]);
		}else if(p->itp->ttype == TPARRAY){
			printf("array [%3d] of %*s", -(type_length - strlen("array [100] of ")), type_str[p->itp->ttype]);
		}else if(p->itp->ttype == TPPROC){
			if(p->itp->paratp == NULL){
				printf("%*s", -type_length, type_str[TPPROC]);
			}else{
				printf("%s", type_str[TPPROC]);
				temp_length += strlen(type_str[TPPROC]);
				q = p->itp->paratp;
				printf("(%s", type_str[q->ttype]);
				temp_length += strlen(type_str[q->ttype]) + 1;
				for(q = q->nextp; q != NULL; q = q->nextp){
					printf(",%s", type_str[q->ttype]);
					temp_length += strlen(type_str[q->ttype]) + 1;
				}
				printf("%*s", -temp_length, ")");
				temp_length = 0;
			}
		}

		printf("%3d | ", p->deflinenum);

		for(r = p->irefp; r != NULL; r = r->nextlinep){
			printf("%d,", r->reflinenum);
			if(r->nextlinep != NULL){
				printf(",");
			}
		}

		printf("\n");
	}

	printf("--------------------------------------------------------------------------\n");
}

void release_idtab(struct ID *id_root) {	/* Release tha data structure */
	struct ID *p, *q;

	for(p = id_root; p != NULL; p = q) {
		free(p->name);
		free(p->procname);
		release_typetab(p->itp);
		release_linetab(p->irefp);
		q = p->nextp;
		free(p);
	}
	init_tab(id_root);
}

void release_typetab(struct TYPE *type_root){
	struct TYPE *p, *q;

	for(p = type_root; p != NULL; p = q){
		free(p->etp);
		release_typetab(p->paratp);
		q = p->nextp;
		free(p);
	}
	init_tab(type_root);
	return;
}

void release_linetab(struct LINE *line_root){
	struct LINE *p, *q;

	for(p = line_root; p != NULL; p = q){
		q = p->nextlinep;
		free(p);
	}
	
	init_tab(line_root);
}