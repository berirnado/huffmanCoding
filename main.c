#include <stdio.h>
#include <stdlib.h>

void gerarArrayDeFrequencia(FILE* fptr, int* freq){
    int c;

    while((c = fgetc(fptr)) != EOF){ // cada linha do arquivo
        freq[(unsigned char)c]++;
    }
}

void main(){
    int freq[256] = {0}; // array utilizado pra armazenar a frequencia de cada byte

    FILE* fptr = fopen("palavras.txt", "r");

    if(fptr == NULL){
        printf("Erro ao abrir arquivo");
        exit(1);
    }

    gerarArrayDeFrequencia(fptr, freq);
    fclose(fptr);

    for(int i = 0; i < 256; ++i){
        printf("%d\n", freq[i]);
    }
}