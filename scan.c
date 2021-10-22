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
static int is_error = 0;
static int is_debugmode = 0;
FILE *fp;


int my_getc(int type){
	int cbuf = fgetc(fp);

	//EOFは即返却
	if(cbuf == EOF){
		if(type == STRING || type == COMMENT){
			printf("parse error at end of input.\n");
			is_error = 1;
		}
		return -1;
	}

	//改行の判定機構
	if(cbuf == '\r'){
		cbuf = fgetc(fp);
		//NAME改行NAMEなどの時に困るので, 区切っておく
		if(cbuf == '\n') cbuf = ' ';
		newline = 1;
		return cbuf;
	}
	else if(cbuf == '\n'){
		cbuf = fgetc(fp);
		//NAME改行NAMEなどの時に困るので, 区切っておく
		if(cbuf == '\r') cbuf = ' ';
		newline = 1;
		return cbuf;
	}

	//表示文字とタブ以外は通さない
	if(!isprint(cbuf) && cbuf != '\t'){
		return -1;
	}
	return cbuf;
}

int init_scan(char* filename){
	// FILE *fp;
	if((fp = fopen(filename, "r")) == NULL){
		return -1;
	}

	cbuf = my_getc(SEPARATOR);
	return 0;
}

int scan(){
	while(cbuf != EOF){
		if(is_error == 1) return -1;
	debug();
	// debugPrintChar("loop.\tcbuf:", cbuf);
		//空白は読み飛ばす
		if(cbuf == ' ' || cbuf == '\t'){
			check_line(SEPARATOR);
			continue;
		}
		//注釈も読み飛ばす
		else if(cbuf == '{'){
			while(cbuf != '}' && cbuf >= 0) check_line(COMMENT);
			if(cbuf < 0) return -1;
			check_line(SEPARATOR);
		}
		else if(cbuf == '/'){
			printf("comment.\tcbuf:%c\n", cbuf);
			check_line(COMMENT);
			if(cbuf != '*'){
				//注釈ではないし、symbolに"/"は存在しない
				printf("in line %d, unknown symbol:/\n", get_linenum());
				return -1;
			}
			check_line(COMMENT);
			int skip = 0;
			while(!(cbuf == '/' && skip == 1)){
				if(cbuf < 0 || is_error == 1) return -1;
				if(cbuf == '*') skip = 1;
				check_line(COMMENT);
				debugPrintChar("comment2.\tcbuf:", cbuf);
			}
			check_line(SEPARATOR);
			continue;
		}
		//文字列の場合, シングルクオートを待つ.
		else if(cbuf == '\''){
			check_line(STRING);
			int i;
			for(i = 0; i < MAXSTRSIZE; i++){
				string_attr[i] = '\0';
			}
			i = 0;
			while(1){
				if(is_error == 1) return -1;
			debug();
			// debugPrintChar("isstring.\tcbuf:", cbuf);
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
				// debugPrintChar("cbuf:", cbuf);
			}
			string_attr[i] = '\0';
			printf("string_attr:%s\n", string_attr);
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
				check_line(TOKEN);
				// debugPrintChar("cbuf:", cbuf);
			}
			if(i > MAXSTRSIZE){
				printf("in line %d, this string is too long.\n", get_linenum());
				printf("Note:the length of valid string is up to %d.\n", MAXSTRSIZE);
				return -1;
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
				check_line(TOKEN);
				debugPrintChar("cbuf:", cbuf);
			} 
			num_attr = atoi(string_attr);
			if(num_attr > 32767 || i > 5){
				printf("in line %d, this number is too large.\n", get_linenum());
				printf("Note:the range of valid numbers is 0 to 32767.\n");
				return -1;
			}
			return TNUMBER;
		}
		//記号の判別
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
		printf("%d\n", type);
		newline = 0;
		printf("newline:%d(%d)--------------------------------------------------\n", public_linenum, private_linenum);
	}
	if(type != SEPARATOR && type != COMMENT){
		public_linenum = private_linenum;
	}
	cbuf = my_getc(type);
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