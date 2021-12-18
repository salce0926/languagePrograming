#include "token-list.h"

extern char *scope_p;
extern char *temp_id_name;
extern struct ID *temp_id;
extern struct TYPE *temp_type;
extern struct ID *temp_procedure;
extern struct TYPE *temp_argument;
extern struct ID *globalidroot;
extern struct ID *localidroot;
extern int error(char *mes);

int to_ttype(int token){
	if(token == TINTEGER) return(TPINT);
	if(token == TBOOLEAN) return(TPBOOL);
	if(token == TCHAR) return(TPCHAR);
	return(error("to_ttype error"));
}

int is_array(struct TYPE *p){
	if(p->arraysize > 0) return(1);
	return(0);
}

struct ID **get_idroot(){
	return(scope_p == NULL ? &globalidroot : &localidroot);
}

int push_front_id(struct ID **idroot, struct ID *p){
	p->nextp = *idroot;
	*idroot = p;
	return(NORMAL);
}

int push_back_id(struct ID **idroot, struct ID *back){
	struct ID *p;
	if(*idroot == NULL){
		*idroot = back;
		return(NORMAL);
	}
	for(p = *idroot; p->nextp != NULL; p = p->nextp);
	p->nextp = back;
	return(NORMAL);
}

int push_front_type(struct TYPE **type_root, struct TYPE *p){
	p->nextp = *type_root;
	*type_root = p;
	return(NORMAL);
}

struct TYPE *pop_front_type(struct TYPE **type_root){
	struct TYPE *p;
	p = *type_root;
	*type_root = (*type_root)->nextp;
	p->nextp = NULL;
	return(p);
}

int push_back_type(struct TYPE **type_root, struct TYPE *back){
	struct TYPE *p;
	if(*type_root == NULL){
		*type_root = back;
		return(NORMAL);
	}
	for(p = *type_root; p->nextp != NULL; p = p->nextp);
	p->nextp = back;
	return(NORMAL);
}

int push_back_line(struct LINE **line_root, struct LINE *back){
	struct LINE *p;
	if(*line_root == NULL){
		*line_root = back;
		return(NORMAL);
	}
	for(p = *line_root; p->nextlinep != NULL; p = p->nextlinep);
	p->nextlinep = back;
	return(NORMAL);
}

int check_operand_type(struct TYPE **temp_type){
	struct TYPE *p = pop_front_type(temp_type);
    if(p->ttype != (*temp_type)->ttype){
        return(error("the type of operand does not match\n"));
    }
    free(p);
    p = NULL;
	return(NORMAL);
}

int init_id(struct ID *p){
	p->name = NULL;
	p->procname = NULL;
	p->itp = NULL;
	p->ispara = -1;
	p->deflinenum = -1;
	p->irefp = NULL;
	p->nextp = NULL;
	return(NORMAL);
}

int init_type(struct TYPE *p){
	p->ttype = -1;
	p->arraysize = -1;
	/*p->etp = NULL;*/
	p->paratp = NULL;
	p->nextp = NULL;
	return(NORMAL);
}

int init_line(struct LINE *p){
	p->reflinenum = -1;
	p->nextlinep = NULL;
	return(NORMAL);
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
	if(is_debugmode) printf("%s\n", __func__);
	if(np == NULL){
		return(NULL);
	}
	if((p = (char *)malloc(strlen(np)+1)) == NULL){
		printf("can not malloc in create_newname\n");
		return(NULL);
	}
	strcpy(p, np);
	return(p);
}

int release_typetab(struct TYPE **type_root){
	struct TYPE *p, *q;

	for(p = *type_root; p != NULL; p = q){
		/*free(p->etp);*/
		release_typetab(&(p->paratp));
		q = p->nextp;
		free(p);
	}
	*type_root = NULL;
	return(NORMAL);
}

int release_linetab(struct LINE **line_root){
	struct LINE *p, *q;

	for(p = *line_root; p != NULL; p = q){
		q = p->nextlinep;
		free(p);
	}
	
	*line_root = NULL;
	return(NORMAL);
}

int release_idtab(struct ID **id_root) {	/* Release tha data structure */
	struct ID *p, *q;

	for(p = *id_root; p != NULL; p = q) {
		free(p->name);
		free(p->procname);
		release_typetab(&(p->itp));
		release_linetab(&(p->irefp));
		q = p->nextp;
		free(p);
	}
	*id_root = NULL;
	return(NORMAL);
}

/* search the name pointed by np */
struct ID *search_id_byname(struct ID *idroot, char *np) {	
	struct ID *p;
	if(is_debugmode) printf("%s\n", __func__);
	/*print_idtab(idroot);*/

	for(p = idroot; p != NULL; p = p->nextp) {
		if(strcmp(np, p->name) == 0){
			if(scope_p == NULL && p->procname == NULL) return(p);
			if(strcmp(scope_p, p->procname) == 0) return(p);
		}
	}
	return(NULL);
}

int store_idname(char **temp_id_name, char *np){
	if(is_debugmode) printf("%s\n", __func__);
	if(*temp_id_name != NULL){
		/*temp_id_name is unused or not initialised after used it*/
		printf("NOTE: current temp_id_name value is %s\n", *temp_id_name);
		return(error("temp_id_name is not initialised\n"));
	}

	*temp_id_name = create_newname(np);
	return(NORMAL);
}

int store_id_byname(struct ID **temp_id, char *np){
	struct ID *p;
	struct ID **idroot;
	if(is_debugmode) printf("%s\n", __func__);
	idroot = get_idroot();
	if((p = search_id_byname(*idroot, np)) != NULL && strcmp(p->procname, scope_p) == 0){
		return(error("this ID already exists in same scope\n"));
	}

	p = create_newid();
	p->name = create_newname(np);
	p->procname = create_newname(scope_p);
	p->deflinenum = get_linenum();

	push_front_id(temp_id, p);
	return(NORMAL);
}

int store_standard_type(struct TYPE **temp_type, int ttype){
	struct TYPE *p = create_newtype();
	if(is_debugmode) printf("%s\n", __func__);

	p->ttype = ttype;

	push_front_type(temp_type, p);
	return(NORMAL);
}

int store_array_type(struct TYPE **temp_type, int array_size){
	struct TYPE *p = pop_front_type(temp_type);

	p->arraysize = array_size;

	push_front_type(temp_type, p);
	return(NORMAL);
}

int store_procedure_type(struct ID *temp_procedure){
	temp_procedure->itp = create_newtype();

	temp_procedure->itp->ttype = TPPROC;
	return(NORMAL);
}

int register_id_bytype(struct ID *temp_id, struct TYPE *temp_type){
	struct ID *p;
	struct ID **idroot = get_idroot();
	for(p = temp_id; p != NULL; p = p->nextp){
		p->itp = create_newtype();
		*(p->itp) = *temp_type;
	}
	push_back_id(idroot, temp_id);
	return(NORMAL);
}

int register_procedure(struct ID *temp_procedure){
	push_back_id(&globalidroot, temp_procedure);
	return(NORMAL);
}

int register_procedure_parameter(struct ID *temp_procedure, int ttype){
	struct TYPE *p = create_newtype();
	
	p->ttype = ttype;

	push_back_type(&(temp_procedure->itp->paratp), p);
	return(NORMAL);
}

int register_parameter_bytype(struct ID *temp_id, struct ID *temp_procedure, struct TYPE *temp_type){
	struct ID *p;
	struct ID **idroot = get_idroot();
	for(p = temp_id; p != NULL; p = p->nextp){
		p->itp = create_newtype();
		*(p->itp) = *temp_type;
		p->ispara = 1;
		register_procedure_parameter(temp_procedure, temp_type->ttype);
	}
	push_back_id(idroot, temp_id);
	return(NORMAL);
}

int store_argument(struct TYPE **temp_argument, int ttype){
	struct TYPE *p = create_newtype();

	p->ttype = ttype;

	push_back_type(temp_argument, p);
	return(NORMAL);
}

int check_argument(struct ID *procedure, struct TYPE **temp_argument){
	struct TYPE *parameter = procedure->itp->paratp;
	struct TYPE *argument = *temp_argument;
	while(parameter != NULL && argument != NULL){
		if(parameter == NULL || argument == NULL){
			return(error("the number of the argument does not match the number of the parameter\n"));
		}
		if(parameter->ttype != argument->ttype){
			return(error("the type of the argument does not match the difinition of the parameter\n"));
		}

		parameter = parameter->nextp;
		argument = argument->nextp;
	}
	release_typetab(temp_argument); 
	return(NORMAL);
}

int ref_newid(struct ID *p){
	struct LINE *l = create_newline();

	l->reflinenum = get_linenum();

	push_back_line(&(p->irefp), l);
	return(NORMAL);
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

int print_idtab(struct ID *idroot) {	/* Output the registered data */
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
	printf("%*s", define_length, "Def.|");
	printf("%*s\n", -reference_length, "Ref.");

	for(p = idroot; p != NULL; p = p->nextp) {
		if(p->procname == NULL){
			printf("%*s", -name_length, p->name);
		}else{
			printf("%s", p->name);
			printf(":%*s", -(name_length - (int)strlen(p->name) - 1), p->procname);
		}

		if(p->itp->ttype == TPINT || p->itp->ttype == TPCHAR || p->itp->ttype == TPBOOL){
			printf("%*s", -type_length, type_str[p->itp->ttype]);
		}else if(p->itp->ttype == TPARRAY){
			printf("array [%3d] of %*s", p->itp->arraysize, -(type_length - (int)strlen("array [100] of ")), type_str[p->itp->ttype]);
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
			printf("%d", r->reflinenum);
			if(r->nextlinep != NULL){
				printf(",");
			}
		}

		printf("\n");
	}

	printf("--------------------------------------------------------------------------\n");
	return(NORMAL);
}
