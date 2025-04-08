#include<stdio.h>
#include<stdlib.h>
#include<string.h>


#define OUT 0
#define IN 1
#define INIT OUT

int split(char c){
    if((c == '/')||(c == '\n')||(c == '\t')||
    (c == '\"')||(c == '+')||(c == ',')||
    (c == '.')||(c == '?')||(c == '!')||
    (c == '(')||(c == ')')||(c == '-')||
    (c == ':')||(c == ';')){
        return 1;
    }
    else 
        return 0;
}

int count(char* file_addr){
    int status = INIT;
    int word = 0;
    char c; 
    FILE* fp = fopen(file_addr, "r");
    if (fp == NULL) return -1;
    while((c = fgetc(fp)) != EOF){
        if(split(c)){
            status = OUT;
        }
        else if(status == OUT){
            status = IN;
            word++;
        }
    }
    return word;
    
}

int main(int argc, char* argv[]){
    if(argc < 2){
        printf("Please insert file address!\n");
    }
    else{
        printf("the word number of file is %d\n", count(argv[1]));
    }

}