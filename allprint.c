#include <stdio.h>
#include <stdlib.h>

int main(){
    FILE *fp;
    if((fp = fopen("sample2/sample02a.mpl", "r")) == NULL){
        printf("Can not open file.\n");
        return 1;
    }

    char cbuf;
    while((cbuf = fgetc(fp)) != EOF){
        if(cbuf == '\n'){
            printf("[\\n]");
            continue;
        }
        if(cbuf == '\r'){
            printf("[\\r]");
            continue;
        }
        printf("%c", cbuf);
    }

    fclose(fp);

    return 0;
}