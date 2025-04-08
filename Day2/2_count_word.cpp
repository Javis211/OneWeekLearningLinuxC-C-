#include<stdio.h>
#include<stdlib.h>
#include<string.h>

//麻了，直接用C++里的哈希表了
#include<unordered_map>
#include<iostream>
using namespace std;


#define OUT 0
#define IN 1
#define INIT OUT

int split(char c){
    if((c == '/')||(c == '\n')||(c == '\t')||
    (c == '\"')||(c == '+')||(c == ',')||
    (c == '.')||(c == '?')||(c == '!')||
    (c == '(')||(c == ')')||(c == '-')||
    (c == ':')||(c == ';')||(c == ' ')){
        return 1;
    }
    else 
        return 0;
}

int count(char* file_addr){
    unordered_map<string, int> wordCount;
    int status = INIT;
    int word = 0;
    string buffer;
    char c; 
    FILE* fp = fopen(file_addr, "r");
    if (fp == NULL) return -1;
    while((c = fgetc(fp)) != EOF){
        if(split(c)){
            if(status == IN){
                for (char& ch : buffer){
                    ch = tolower(ch);
                }
                wordCount[buffer]++;
                buffer.clear();
            }
            status = OUT;
        }
        else{
            if (status == OUT){
                status = IN;
                word++;
            }
            buffer += c;
        }
    }
    if(status == IN){
        for (char& ch : buffer){
            ch = tolower(ch);
        }
        wordCount[buffer]++;
        buffer.clear();
    }
    for (const auto& pair : wordCount) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }

    return word;
    
}

int main(int argc, char* argv[]){
    if(argc < 2){
        printf("Please insert file address!\n");
    }
    else{
        printf("word: %d\n", count(argv[1]));
    }

}