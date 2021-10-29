#include "token-list.h"

#define NORMAL 0
#define ERROR 1

int token;

int parse_program() {
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

int error(char *mes){
    printf("¥n ERROR: %s¥n", mes);
    end_scan(); /* 入力ファイルを閉じる */
    return(ERROR);
}