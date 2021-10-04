#include "token-list.h"

int cbuf;
int num_attr;
char string_attr[MAXSTRSIZE];
static int linenum = 0;
static int newline = 1;
static int is_debugmode = 0;
FILE *fp;


int my_getc(){
	int cbuf = fgetc(fp);
	if(cbuf == '\r'){
		cbuf = fgetc(fp);
		//NAME改行NAMEなどの時に困るので, 区切っておく
		if(cbuf == '\n') cbuf = ' ';
		newline = 1;
	}
	else if(cbuf == '\n'){
		cbuf = fgetc(fp);
		//NAME改行NAMEなどの時に困るので, 区切っておく
		if(cbuf == '\r') cbuf = ' ';
		newline = 1;
	}
	return cbuf;
}

int init_scan(char* filename){
	// FILE *fp;
	if((fp = fopen(filename, "r")) == NULL){
		return -1;
	}

	cbuf = my_getc();
	return 0;
}

int scan(){
	while(cbuf != EOF){
	debug();
	// debugPrintChar("loop.\tcbuf:", cbuf);
		//空白は読み飛ばす
		if(isspace(cbuf)){
			check_line();
			continue;
		}
		//注釈も読み飛ばす
		else if(cbuf == '{'){
			while(cbuf != '}') check_line();
			check_line();
		}
		else if(cbuf == '/'){
			printf("comment.\tcbuf:%c\n", cbuf);
			check_line();
			if(cbuf != '*'){
				//注釈ではないし、symbolに"/"は存在しない
				return -1;
			}
			int skip = 0;
			while(!(cbuf == '/' && skip == 1)){
				if(cbuf == '*') skip = 1;
				check_line();
				debugPrintChar("comment2.\tcbuf:", cbuf);
			}
			check_line();
			continue;
		}
		//文字列の場合, シングルクオートを待つ.
		else if(cbuf == '\''){
			check_line();
			while(cbuf != '\'') check_line();
			check_line();
			// printf("isstring.\tcbuf:%c\n", cbuf);
			return TSTRING;
		}
		//英字の場合、英数字が続く限り読み込んで名前かキーワードかを判別する
		else if(isalpha(cbuf)){
			int i;
			for(i = 0; i < MAXSTRSIZE; i++){
				string_attr[i] = '\0';
			}
			i = 0;
			while(isalnum(cbuf)){
			debug();
			// debugPrintChar("isalpha.\tcbuf:", cbuf);
				string_attr[i++] = cbuf;
				check_line();
				// debugPrintChar("cbuf:", cbuf);
			}
			string_attr[i] = '\0';
			printf("string_attr:%s\n", string_attr);
			for(i = 0; i < KEYWORDSIZE; i++){
				// printf("keyword:%s\n", key[i].keyword);
				if(strcmp(string_attr, key[i].keyword) == 0){
					return key[i].keytoken;
				}
			}
			return TNAME;
		}
		//数字の場合、数字が続く限り読み込んで値を格納する
		else if(isdigit(cbuf)){
			int i = 0;
			debugPrintChar("isdigit.\tcbuf:", cbuf);
			while(isdigit(cbuf)){
				string_attr[i++] = cbuf;
				check_line();
				debugPrintChar("cbuf:", cbuf);
			} 
			num_attr = atoi(string_attr);
			return TNUMBER;
		}
		//記号の判別
		else if(cbuf == '+'){
			check_line();
			return TPLUS;
		}
		else if(cbuf == '-'){
			check_line();
			return TMINUS;
		}
		else if(cbuf == '*'){
			check_line();
			return TSTAR;
		}
		else if(cbuf == '='){
			check_line();
			return TEQUAL;
		}
		else if(cbuf == '<'){
			check_line();
			if(cbuf == '>'){
				check_line();
				return TNOTEQ;
			}
			else if(cbuf == '='){
				check_line();
				return TLEEQ;
			}
			else return TLE;
		}
		else if(cbuf == '>'){
			check_line();
			if(cbuf == '='){
				check_line();
				return TGREQ;
			}
			else return TGR;
		}
		else if(cbuf == '('){
			check_line();
			return TLPAREN;
		}
		else if(cbuf == ')'){
			check_line();
			return TRPAREN;
		}
		else if(cbuf == '['){
			check_line();
			return TLSQPAREN;
		}
		else if(cbuf == ']'){
			check_line();
			return TRSQPAREN;
		}
		else if(cbuf == ':'){
			check_line();
			if(cbuf == '='){
				check_line();
				return TASSIGN;
			}
			else return TCOLON;
		}
		else if(cbuf == '.'){
			check_line();
			return TDOT;
		}
		else if(cbuf == ','){
			check_line();
			return TCOMMA;
		}
		else if(cbuf == ';'){
			check_line();
			return TSEMI;
		}
		else{
			printf("unknown symbol:%c\n", cbuf);
			return -1;
		}
	}
}

int get_linenum(){
    return linenum;
}

void end_scan(){
	fclose(fp);
	return;
}

void check_line(){
	if(newline == 1){
		linenum++;
		newline = 0;
		printf("newline--------------------------------------------------\n");
	}
	cbuf = my_getc();
	return;
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