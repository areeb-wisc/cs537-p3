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

void print_tokens(char ** tokens, int n_tokens) {
    // char* abc="abc";
    // tokens[0] = abc;
    // tokens=&abc;
    // **tokens ='a';
    printf("n_tokens = %d\n", n_tokens);
    printf("tokens={");
    for (int i = 0; i < n_tokens; i++)
        printf("%s,", tokens[i]);
    printf("}\n");
}

void tokenize(char* oline, char*** ptokens, int* p_n_tokens, const char* delim) {

    char* line = clone_str(oline);
    // printf("TOKENIZE: copied line before strtok = %s\n", line);
    
    char** tokens = (char**)malloc(20*sizeof(char*));
    int size = -1;
    char* token = strtok(line, delim);
    while (token != NULL) {
        tokens[++size] = clone_str(token);
        token = strtok(NULL, delim);        
    }

    // printf("TOKENIZE: copied line after strtok = %s\n", line);
    
    // printf("TOKENIZE: tokens before free\n");
    // print_tokens(tokens, size + 1);

    free(line);

    // printf("TOKENIZE: copied line after free = %s\n", line);
    
    // printf("TOKENIZE: tokens after free\n");
    // print_tokens(tokens, size + 1);

    *ptokens = tokens;
    *p_n_tokens = size + 1;
}

int main() {

    // char* empty = "\"\"";
    // printf("empty=%s\n", empty);
    // printf("sizeof(empty)=%ld\n", sizeof(empty));
    // printf("strlen(empty)=%ld\n", strlen(empty));
    // for (size_t i = 0; i < strlen(empty); i++)
    //     printf("empty[%ld]=%c,", i, empty[i]);
    // printf("\n");

    // printf("wsh in dir2\n");

    char* oline = "abc=";
    // printf("original line before TOKENIZE = %s\n", oline);

    int n_tokens = 0;
    char** tokens = NULL;
    tokenize(oline, &tokens, &n_tokens, "=");

    // printf("MAIN: original line after TOKENIZE = %s\n", oline);

    // printf("MAIN: tokens after TOKENIZE\n");
    print_tokens(tokens, n_tokens);

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