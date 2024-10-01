#define _GNU_SOURCE

#include<ctype.h>
#include<limits.h>
#include<stdarg.h>
#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>
#include<unistd.h>

char* clone_str(char* str) {
    char* clone = (char*)malloc((strlen(str) + 1) * sizeof(char));
    strcpy(clone, str);
    return(clone);
}

void tokenize(char* oline, char*** ptokens, int* p_n_tokens) {

    char* line = clone_str(oline);
    printf("TOKENIZE: copied line before strtok = %s\n", line);
    
    char** tokens = (char**)malloc(20*sizeof(char*));
    int size = -1;
    char* token = strtok(line, " ");
    while (token != NULL) {
        tokens[++size] = clone_str(token);
        token = strtok(NULL, " ");        
    }

    printf("TOKENIZE: copied line after strtok = %s\n", line);
    
    printf("TOKENIZE: tokens before free ={");
    for (int i = 0; i < size; i++)
        printf("%s,", tokens[i]);
    printf("}\n");

    free(line);

    printf("TOKENIZE: copied line after free = %s\n", line);
    
    printf("TOKENIZE: tokens after free ={");
    for (int i = 0; i < size; i++)
        printf("%s,", tokens[i]);
    printf("}\n");

    *ptokens = tokens;
    *p_n_tokens = size;
}

int main() {

    char* oline = "hello my name is areeb";
    printf("original line before TOKENIZE = %s\n", oline);

    int n_tokens = 0;
    char** tokens = NULL;
    tokenize(oline, &tokens, &n_tokens);

    printf("MAIN: original line after TOKENIZE = %s\n", oline);

    printf("MAIN: tokens after TOKENIZE ={");
    for (int i = 0; i < n_tokens; i++)
        printf("%s,", tokens[i]);
    printf("}\n");

    // free(line);

    // printf("copied line after free = %s\n", line);
    
    // printf("tokens after free ={");
    // for (int i = 0; i < size; i++)
    //     printf("%s,", tokens[i]);
    // printf("}\n");

    // char* xyz = getenv("xyz");
    // printf("getenv(xyz)=%s\n", xyz);
    
    // char** start = environ;
    // while (*start != NULL) {
    //     printf("%s\n", *start);
    //     start++;
    // }

    // putenv("xyz=areeb");
    // xyz = getenv("xyz");
    // printf("getenv(xyz)=%s\n", xyz);
    
    // start = environ;
    // while (*start != NULL) {
    //     printf("%s\n", *start);
    //     start++;
    // }

    // unsetenv("xyz");
    // xyz = getenv("xyz");
    // printf("getenv(xyz)=%s\n", xyz);
    
    // start = environ;
    // while (*start != NULL) {
    //     printf("%s\n", *start);
    //     start++;
    // }

    return 0;
}