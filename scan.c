#include "token-list.h"

#define SEPARATOR 0
#define TOKEN 1
#define STRING 2
#define COMMENT 3

int cbuf;
int num_attr;
char string_attr[MAXSTRSIZE];
static int public_linenum = 0;
static int private_linenum = 0;
static int newline = 1;
static int pre_cbuf = 0;
static int is_error = 0;
FILE *fp;


int my_getc(int type){
	int cbuf = fgetc(fp);

	/*EOF is returned immediately*/
	if(cbuf == EOF){
		if(type == STRING || type == COMMENT){
			printf("parse error at end of input.\n");
			is_error = 1;
		}
		return -1;
	}

	/*judging a new line*/
	if(cbuf == '\r'){
		/*assume that whitespace is read ahead*/
		cbuf = ' ';
		if(pre_cbuf == '\n') {
			pre_cbuf = 0;
			return cbuf;
		}
		pre_cbuf = '\r';
		newline = 1;
		return cbuf;
	}
	else if(cbuf == '\n'){
		cbuf = ' ';
		/*assume that whitespace is read ahead*/
		if(pre_cbuf == '\r'){
			pre_cbuf = 0;
			return cbuf;
		}
		pre_cbuf = '\n';
		newline = 1;
		return cbuf;
	}
	else{
		pre_cbuf = 0;
	}

	/*Do not pass anything but display characters and tabs*/
	if(!isprint(cbuf) && cbuf != '\t'){
		return -1;
	}
	return cbuf;
}

int init_scan(char* filename){
	if((fp = fopen(filename, "r")) == NULL){
		return -1;
	}

	cbuf = my_getc(SEPARATOR);
	return 0;
}

int scan(){
	while(cbuf != EOF){
		if(is_error == 1) return -1;
		/*spaces are skip over*/
		if(cbuf == ' ' || cbuf == '\t'){
			check_line(SEPARATOR);
			continue;
		}
		/*skip the annotations too*/
		else if(cbuf == '{'){
			while(cbuf != '}' && cbuf >= 0) check_line(COMMENT);
			if(cbuf < 0) return -1;
			check_line(SEPARATOR);
		}
		else if(cbuf == '/'){
			int skip = 0;
			check_line(COMMENT);
			if(cbuf != '*'){
				/*it's not an annotation and "/" is not a symbol*/
				printf("in line %d, unknown symbol:/\n", get_linenum());
				return -1;
			}
			check_line(COMMENT);
			while(!(cbuf == '/' && skip == 1)){
				if(cbuf < 0 || is_error == 1) return -1;
				if(cbuf == '*') skip = 1;
				check_line(COMMENT);
			}
			check_line(SEPARATOR);
			continue;
		}
		/*for strings, wait for single quote*/
		else if(cbuf == '\''){
			int i;
			check_line(STRING);
			for(i = 0; i < MAXSTRSIZE; i++){
				string_attr[i] = '\0';
			}
			i = 0;
			while(1){
				if(is_error == 1) return -1;
				if(cbuf == '\''){
					check_line(STRING);
					if(cbuf == '\''){
						string_attr[i++] = '\'';
					}else{
						break;
					}
				}
				string_attr[i++] = cbuf;
				check_line(STRING);
			}
			string_attr[i] = '\0';
			return TSTRING;
		}
		/*for alphabets, read as long as the letter or number follows*/
		else if(isalpha(cbuf)){
			int i;
			for(i = 0; i < MAXSTRSIZE; i++){
				string_attr[i] = '\0';
			}
			i = 0;
			while(isalnum(cbuf)){
				string_attr[i++] = cbuf;
				check_line(TOKEN);
			}
			if(i > MAXSTRSIZE){
				printf("in line %d, this string is too long.\n", get_linenum());
				printf("Note:the length of valid string is up to %d.\n", MAXSTRSIZE);
				return -1;
			}
			string_attr[i] = '\0';
			for(i = 0; i < KEYWORDSIZE; i++){
				if(strcmp(string_attr, key[i].keyword) == 0){
					return key[i].keytoken;
				}
			}
			return TNAME;
		}
		/*for numbers, read as long as the number follows*/
		else if(isdigit(cbuf)){
			int i = 0;
			for(i = 0; i < MAXSTRSIZE; i++){
				string_attr[i] = '\0';
			}
			i = 0;
			while(isdigit(cbuf)){
				string_attr[i++] = cbuf;
				check_line(TOKEN);
			} 
			num_attr = atoi(string_attr);
			if(num_attr > 32767 || i > 5){
				printf("in line %d, this number is too large.\n", get_linenum());
				printf("Note:the range of valid numbers is 0 to 32767.\n");
				return -1;
			}
			return TNUMBER;
		}
		/*the case of a symbol*/
		else if(cbuf == '+'){
			check_line(TOKEN);
			return TPLUS;
		}
		else if(cbuf == '-'){
			check_line(TOKEN);
			return TMINUS;
		}
		else if(cbuf == '*'){
			check_line(TOKEN);
			return TSTAR;
		}
		else if(cbuf == '='){
			check_line(TOKEN);
			return TEQUAL;
		}
		else if(cbuf == '<'){
			check_line(TOKEN);
			if(cbuf == '>'){
				check_line(TOKEN);
				return TNOTEQ;
			}
			else if(cbuf == '='){
				check_line(TOKEN);
				return TLEEQ;
			}
			else return TLE;
		}
		else if(cbuf == '>'){
			check_line(TOKEN);
			if(cbuf == '='){
				check_line(TOKEN);
				return TGREQ;
			}
			else return TGR;
		}
		else if(cbuf == '('){
			check_line(TOKEN);
			return TLPAREN;
		}
		else if(cbuf == ')'){
			check_line(TOKEN);
			return TRPAREN;
		}
		else if(cbuf == '['){
			check_line(TOKEN);
			return TLSQPAREN;
		}
		else if(cbuf == ']'){
			check_line(TOKEN);
			return TRSQPAREN;
		}
		else if(cbuf == ':'){
			check_line(TOKEN);
			if(cbuf == '='){
				check_line(TOKEN);
				return TASSIGN;
			}
			else return TCOLON;
		}
		else if(cbuf == '.'){
			check_line(TOKEN);
			return TDOT;
		}
		else if(cbuf == ','){
			check_line(TOKEN);
			return TCOMMA;
		}
		else if(cbuf == ';'){
			check_line(TOKEN);
			return TSEMI;
		}
		else{
			printf("in line %d, unknown symbol:%c\n", get_linenum(), cbuf);
			return -1;
		}
	}
}

int get_linenum(){
    return public_linenum;
}

void end_scan(){
	fclose(fp);
	public_linenum = private_linenum;
	return;
}

void check_line(const int type){
	if(newline == 1){
		private_linenum++;
		newline = 0;
	}
	if(type != SEPARATOR && type != COMMENT){
		public_linenum = private_linenum;
	}
	cbuf = my_getc(type);
	if(cbuf == '\r' || cbuf == '\n') check_line(type);
	return;
}