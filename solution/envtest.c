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

#include "dict.h"

#define WSH_cd      0
#define WSH_exit    1
#define WSH_export  2
#define WSH_history 3
#define WSH_local   4
#define WSH_ls      5
#define WSH_vars    6

const int n_builtins = 7;
const char* builtins[] = {"cd", "exit", "export", "history", "local", "ls", "vars"};

int wsh_cd() {
    printf("wsh_cd() called\n");
    return 0;
}
int wsh_exit() {
    printf("wsh_exit() called\n");
    return 0;
}
int wsh_export() {
    printf("wsh_export() called\n");    
    return 0;
}
int wsh_history() {
    printf("wsh_history() called\n");    
    return 0;
}
int wsh_local() {
    printf("wsh_local() called\n");    
    return 0;
}
int wsh_ls() {
    printf("wsh_ls() called\n");    
    return 0;
}
int wsh_vars() {
    printf("wsh_vars() called\n");    
    return 0;
}

static int (*wshcalls[])(void) = {
[WSH_cd]      = wsh_cd,
[WSH_exit]    = wsh_exit,
[WSH_export]  = wsh_export,
[WSH_history] = wsh_history,
[WSH_local]   = wsh_local,
[WSH_ls]      = wsh_ls,
[WSH_vars]    = wsh_vars
};

// char* clone_str(char* str) {
//     char* clone = (char*)malloc((strlen(str) + 1) * sizeof(char));
//     strcpy(clone, str);
//     return(clone);
// }

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

void tokenize(const char* oline, const char* delim, char*** ptokens, int* n_tokens) {

    char* const line = clone_str(oline);
    // printf("tokenize\n");
    int buff_size = 0;
    int max_buff_size = 1;
    char** tokens = (char**)malloc(buff_size * sizeof(char*));

    // printf("right before strtok\n");
    char* token = strtok(line, delim);
    // printf("right after strtok\n");
    while (token != NULL) {
        buff_size++;
        if (buff_size > max_buff_size) {
            max_buff_size *= 2;
            tokens = (char**)realloc(tokens, max_buff_size * sizeof(char*));
        }
        tokens[buff_size - 1] = token;
        token = strtok(NULL, delim);
    }
    tokens = (char**)realloc(tokens, (buff_size + 1) * sizeof(char*));
    tokens[buff_size] = NULL;

    *ptokens = tokens; // list of all token strings (including NULL)
    *n_tokens = buff_size; // size does not include NULL
    // printf("tokenize complete\n");
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

int handle_builtin(dict* mydicts, char* command) {
    // char* wsh_command = join("WSH", command, '_');
    // printf("wsh_command = %s\n", wsh_command);
    int num = atoi(get_val(mydicts, command));
    wshcalls[num]();
    return -1;
}

char* get_filename_from_path1(const char* path) {
    
    if (path == NULL)
        return NULL;
    
    int pathlen = strlen(path);
    int i = pathlen - 1;
    while (i >= 0 && path[i] != '/')
        i--;
    
    int n = pathlen - i - 1;
    char* filename = (char*)malloc((n + 1) * sizeof(char));
    return strncpy(filename, path + i + 1, n);
}

char* get_filename_from_path2(const char* path) {
    
    if (path == NULL)
        return NULL;
    
    int len = strlen(path);
    if (len == 0 || path[len - 1] == '/')
        return "";

    char** tokens = NULL;
    int n_tokens = 0;
    tokenize(path,"/",&tokens,&n_tokens);

    // print_tokens(tokens, n_tokens);
    if (n_tokens == 0)
        return "";

    return tokens[n_tokens - 1];
}

bool areEqual(const char* str1, const char* str2) {
    if (str1 == NULL && str2 == NULL)
        return true;
    if (str1 == NULL || str2 == NULL)
        return false;
    return strcmp(str1,str2) == 0;
}

int main() {

    dict* mydicts = (dict*)malloc(sizeof(dict));
    mydicts->size = 0;
    mydicts->max_size = 1;
    mydicts->entries = (entry**)malloc(mydicts->max_size * sizeof(entry*));

    for (int i = 0; i < n_builtins; i++) {
        const char* key = builtins[i];
        char num = (char)(i + 48);
        char* val = &num;
        add_entry(mydicts, key, val);
    }

    print_dict(mydicts);

    handle_builtin(mydicts, "cd");
    handle_builtin(mydicts, "exit");
    handle_builtin(mydicts, "export");
    handle_builtin(mydicts, "history");
    handle_builtin(mydicts, "local");
    handle_builtin(mydicts, "ls");
    handle_builtin(mydicts, "vars");

    // int n_paths = 8;
    // char* paths[] = {""," ","path","/","/path","/path/","/path/file","/path/file/"};

    // for (int i = 0; i < n_paths; i++) {
    //     char* out1 = get_filename_from_path1(paths[i]);
    //     if (!out1) out1 = "NULL";
    //     char* out2 = get_filename_from_path2(paths[i]);
    //     if (!out2) out2 = "NULL";
    //     printf("paths = %s\n",paths[i]);
    //     printf("out1 = %s, out2 = %s, equal = %d\n\n", out1, out2, areEqual(out1,out2));
    // }

    // char* str1 = "hello world &>> abc";
    // char* str2 = "&&";

    // if (str2 + 2 == NULL)
    //     printf("NULL\n");
    
    // if (*(str2 + 2) == '\0')
    //     printf("Yup, also NULL\n");

    // printf("found %s? at %s\n", str2, strstr(str1,str2));

    // printf("hello world\n");
    // dup2(STDOUT_FILENO, STDOUT_FILENO);
    // printf("is it working?\n");

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