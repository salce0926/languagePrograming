#include <stdio.h>
#include <stdlib.h>

int main(){
    FILE *fp;
    if((fp = fopen("sample1/percent.mpl", "w")) == NULL){
        printf("Can not open file.\n");
        return 1;
    }

    char alphabet = '\a';
    int i;
    for(i = 0; i < 7; i++){
        fprintf(fp, "%c", alphabet + i);
    }

    fclose(fp);

    return 0;
}