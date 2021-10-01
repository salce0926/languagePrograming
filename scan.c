#include "token-list.h"

static int cbuf;
static int num_attr;
static char string_attr[MAXSTRSIZE];
static int linenum = 0;
FILE *fp;


int my_getc(){
	int cbuf = fgetc(fp);
	if(cbuf == '\r'){
		cbuf = fgetc(fp);
		if(cbuf == '\n') cbuf = fgetc(fp);
	}
	else if(cbuf == '\n'){
		cbuf = fgetc(fp);
		if(cbuf == '\r') cbuf = fgetc(fp);
	}
	return cbuf;
}

int init_scan(char* filename){
	// FILE *fp;
	if((fp = fopen(filename, "r")) == NULL){
		return -1;
	}

	cbuf = fgetc(fp);
	return 0;
}

int scan(){
	while(cbuf != EOF){
		//空白は読み飛ばす
		if(isspace(cbuf)) continue;
		//注釈も読み飛ばす
		else if(cbuf == '{'){
			while(cbuf != '}') cbuf = fgetc(fp);
		}
		else if(cbuf == '/'){
			cbuf = fgetc(fp);
			if(cbuf != '*'){
				//注釈ではないし、symbolに"/"は存在しない
				return -1;
			}
			int skip = 0;
			while(!(cbuf == '/' && skip == 1)){
				// if(cbuf == "/" && skip == 1) break;
				if(cbuf == '*') skip = 1;
			}
			continue;
		}
		//英字の場合、英数字が続く限り読み込んで名前かキーワードかを判別する
		else if(isalpha(cbuf)){
			int i = 0;
			while(isalnum(cbuf)){
				string_attr[i++] = cbuf;
			}
			for(i = 0; i < KEYWORDSIZE; i++){
				if(strcmp(string_attr, key[i].keyword) == 0){
					return key[i].keytoken;
				}
				return TNAME;
			}
		}
		//数字の場合、数字が続く限り読み込んで値を格納する
		else if(isdigit(cbuf)){
			int i = 0;
			while(isdigit(cbuf)) string_attr[i++] = cbuf;
			num_attr = atoi(string_attr);
			return TNUMBER;
		}
		//記号の判別
		else if(cbuf == '+') return TPLUS;
		else if(cbuf == '-') return TMINUS;
		else if(cbuf == '*') return TSTAR;
		else if(cbuf == '=') return TEQUAL;
		else if(cbuf == '<'){
			cbuf = fgetc(fp);
			if(cbuf == '>') return TNOTEQ;
			else if(cbuf == '=') return TLEEQ;
			else return TLE;
		}
		else if(cbuf == '>'){
			cbuf = fgetc(fp);
			if(cbuf == '=') return TGREQ;
			else return TGR;
		}
		else if(cbuf == '(') return TLPAREN;
		else if(cbuf == ')') return TRPAREN;
		else if(cbuf == '[') return TLSQPAREN;
		else if(cbuf == ']') return TRSQPAREN;
		else if(cbuf == ':'){
			cbuf = fgetc(fp);
			if(cbuf == '=') return TASSIGN;
			else return TCOLON;
		}
		else if(cbuf == '.') return TDOT;
		else if(cbuf == ',') return TCOMMA;
		else if(cbuf == ';') return TSEMI;
	}
}

int get_linenum(){
    return linenum;
}

void end_scan(){
	fclose(fp);
}