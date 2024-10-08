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
#include<fcntl.h>

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

char* join(const char* str1, const char* str2, const char joiner) {
    int len1 = strlen(str1), len2 = strlen(str2);
    char* joined = (char*)malloc((len1 + len2 + 2) * sizeof(char));
    strcpy(joined, str1);
    joined[len1] = joiner;
    joined[len1 + 1] = '\0';
    strcat(joined, str2);
    return joined;
}

int main() {

    // char* str1 = "hello world &>> abc";
    // char* str2 = "&&";

    // if (str2 + 2 == NULL)
    //     printf("NULL\n");
    
    // if (*(str2 + 2) == '\0')
    //     printf("Yup, also NULL\n");

    // printf("found %s? at %s\n", str2, strstr(str1,str2));

    printf("hello world\n");
    dup2(STDOUT_FILENO, STDOUT_FILENO);
    printf("is it working?\n");

    // int copyout = dup(fileno(stdout));

    // printf("printing to console\n");

    // FILE* fileout = fopen("output.txt", "w");
    // dup2(fileno(fileout), fileno(stdout));
    // close(fileno(fileout));

    // printf("printing to file!!\n");

    // dup2(copyout, fileno(stdout));
    // close(copyout);

    // printf("printing to console again\n");


    // int copyin = dup(fileno(stdin));
    // printf("stdin = %d\n", fileno(stdin));
    // printf("copyin = %d\n", copyin);

    // char* line = NULL;
    // size_t len = 0;

    // int read = getline(&line, &len, stdin);
    // printf("read %d characters from console = %s\n", read, line);

    // FILE* filein = fopen("input.txt", "r");
    // printf("filein = %d\n", fileno(filein));

    // int ret = dup2(fileno(filein), fileno(stdin));
    // printf("dup2(file, stdin) = %d, fileno(stdin) = %d\n", ret, fileno(stdin));

    // // close(fileno(filein));

    // read = getline(&line, &len, stdin);
    // printf("read %d characters from file = %s\n", read, line);

    // fflush(stdin);
    // ret = dup2(copyin, fileno(stdin));
    // printf("dup2(copyin, stdin) = %d, fileno(stdin) = %d\n", ret, fileno(stdin));
    // // close(copyin);

    // read = getline(&line, &len, stdin);
    // printf("read %d characters from console = %s\n", read, line);

    // ret = dup2(fileno(filein), fileno(stdin));
    // printf("dup2(file, stdin) = %d, fileno(stdin) = %d\n", ret, fileno(stdin));

    // read = getline(&line, &len, stdin);
    // printf("again read %d characters from file = %s\n", read, line);

    // fflush(stdin);
    // ret = dup2(copyin, fileno(stdin));
    // printf("dup2(copyin, stdin) = %d, fileno(stdin) = %d\n", ret, fileno(stdin));

    // read = getline(&line, &len, stdin);
    // printf("again read %d characters from console = %s\n", read, line);

    // close(fileno(filein));
    // close(copyin);

    // read = getline(&line, &len, stdin);
    // printf("finally read %d characters from console = %s\n", read, line);
    
    // FILE* out = stdout;
    // int copyout = dup(fileno(out)); // create copy for restoring later

    // FILE* fileout = fopen("output.txt", "w");

    // printf("fileno(console) = %d, fileno(fileout) = %d\n", fileno(stdout), fileno(fileout));
    
    // dup2(fileno(fileout), fileno(out));

    // printf("This should be printed to output.txt\n");

    // dup2(copyout, fileno(out));
    // close(copyout);

    // printf("This should again be printed to console\n");

    // char* line = NULL;
    // size_t len = 0;

    // FILE* in = stdin;
    // int copyin = dup(fileno(in));
    // FILE* filein = fopen("input.txt", "r");

    // int read = getline(&line, &len, stdin);
    // printf("%d characters read from console = %s\n", read, line);

    // dup2(fileno(filein), fileno(in));

    // read = getline(&line, &len, stdin);
    // printf("%d characters read from input.txt = %s\n", read, line);

    // read = getline(&line, &len, stdin);
    // printf("%d characters read from input.txt = %s\n", read, line);

    // dup2(copyin, fileno(in));
    // close(copyin);

    // read = getline(&line, &len, stdin);
    // printf("%d characters read from console = %s\n", read, line);

    // printf("%s\n", join("hello", "world", '-'));
    // printf("%s\n", join("hello", "world", ':'));
    // printf("%s\n", join("hello", "world", ' '));
    // printf("%s\n", join("hello", "", '/'));
    // printf("%s\n", join("", "world", '/'));
    // printf("%s\n", join("", "", '/'));

    // printf("access(/bin/, F_OK) = %d\n", access("/bin/areeb", F_OK));

    // printf("access(/bin/, R_OK) = %d\n", access("/bin/areeb", R_OK));

    // printf("access(/bin/, W_OK) = %d\n", access("/bin/areeb", W_OK));

    // printf("access(/bin/, X_OK) = %d\n", access("/bin/areeb", X_OK));

    // printf("access("", F_OK) = %d\n", access("/", F_OK));

    // printf("access("", R_OK) = %d\n", access("/", R_OK));

    // printf("access("", W_OK) = %d\n", access("/", W_OK));

    // printf("access("", X_OK) = %d\n", access("/", X_OK));

    // char* empty = "\"\"";
    // printf("empty=%s\n", empty);
    // printf("sizeof(empty)=%ld\n", sizeof(empty));
    // printf("strlen(empty)=%ld\n", strlen(empty));
    // for (size_t i = 0; i < strlen(empty); i++)
    //     printf("empty[%ld]=%c,", i, empty[i]);
    // printf("\n");

    // printf("wsh in dir2\n");

    // char* oline = "abc=";
    // printf("original line before TOKENIZE = %s\n", oline);

    // int n_tokens = 0;
    // char** tokens = NULL;
    // tokenize(oline, &tokens, &n_tokens, "=");

    // printf("MAIN: original line after TOKENIZE = %s\n", oline);

    // printf("MAIN: tokens after TOKENIZE\n");
    // print_tokens(tokens, n_tokens);

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