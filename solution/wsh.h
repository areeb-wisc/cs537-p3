#ifndef _WSH_HEADER
#define _WSH_HEADER

// #include<ctype.h>
#include<dirent.h>
// #include<limits.h>
// #include<stdarg.h>
// #include<stdbool.h>
// #include<stdio.h>
// #include<stdlib.h>
// #include<string.h>
// #include<sys/wait.h>
#include<unistd.h>

// Clone a string
char* clone_str(const char*);
/******************************* DICTIONARY START *****************************/
typedef struct Entry {
    char* key;
    char* val;
} entry;

typedef struct Dict {
    int size;
    int max_size;
    entry** entries;

} dict;
dict* shell_vars;
dict* create_dictionary(int);
// Return index of key in dictionary if present, else -1 
int get_dict_idx(dict*, const char*);
entry* make_dict_entry(const char*, const char*);
void resize_if_needed(dict*);
int add_dict_var(dict* dictionary, const char*,const char*);
char* get_dict_var(dict*, const char*);
void print_strings(char**, int, char*, char*);
// TODO(Areeb): remove these later
void print_environ();
void print_dict(dict*);
void print_vars();
/********************************** DICTIONARY END *******************************/

/***************************** HISTORY START **********************************/

typedef struct circular_queue {
    char** words;
    int n, r, f;
} cqueue;

cqueue* create_cqueue(int);
cqueue* history;
char* get(cqueue*, int);
void push(cqueue*, const char*);
void pop(cqueue*);
int getsize(cqueue*);
void print(cqueue*, int);
void display(cqueue*);
void resize(cqueue**, int);
/**************************** HISTORY END ************************************/
/******************** VARNAME DEREFERENCING HELPERS START *******************/
char* dereference(const char*);
void dereference_tokens(char*** varnames, int n_varnames);
/********************* VARNAME DEREFERENCING HELPERS END ********************/
/******************************* STRING HELPERS START ****************************/
void strip(char** word, ssize_t* len);
void promptf(char* fmtstr, ...);
/**
 * Tokenize the given input string using delimiter = delim
 * Also appends a NULL to the output array for exec argv
 * e.g.
 * Input: "echo hello world"
 * Output: {"echo", "hello", "world", NULL}
 */
void tokenize(const char* oline, const char* delim, char*** ptokens, int* n_tokens);
/**
 * Joins 2 strings using 'joiner'
 * e.g. ("str1","",'/') -> "str1/"
 * e.g. ("","str2",'/') -> "/str2"
 * e.g. ("","",'*') -> "*"
 * e.g. ("path/to","filename",'/) -> "path/to/filename"
 */
char* join(const char* str1, const char* str2, const char joiner);
int redirect_fd_to_file(int fd, const char* file_name, const char* mode);
int handle_redirection_if_any(char*** tokens, int* n_tokens, bool* success);
/**************************** STRING HELPERS END *************************/

/**
 * Get file name from its path
 * e.g. path/to/file1.txt -> file1.txt
 * e.g. /file2.txt -> file2.txt
 * e.g. file3.txt -> file3.txt
 * e.g. / -> [empty string]
 */
char* get_filename_from_path(const char* path);
/*************************** NON BUILT-IN CALLS START ************************/
int execute(char* path, char** argv);
int handle_non_builtin(char**,int);
/*************************** NON BUILT-IN CALLS START ************************/
/***************************** BUILT-IN CALLS START ***************************/
int wsh_cd(char**,int);
int wsh_exit(char** tokens, int n_tokens);
// TODO(Areeb): [export a] should give error 
int wsh_export(char**,int);
int handle(char**, int);
bool isValidNumber(const char* numstr);
int wsh_history(char**,int);
// TODO(Areeb): [local a] should give error
int wsh_local(char**,int);
int non_hidden_dirent(const struct dirent* entry);
int wsh_ls(char**,int);
int wsh_vars(char**,int);
dict* builtins;
bool is_builtin(const char*);
static int (*wshcalls[])(char**,int) = {wsh_cd,wsh_exit,wsh_export,wsh_history,wsh_local,wsh_ls,wsh_vars};
int handle_builtin(char**,int);
/*************************** BUILT-IN CALLS END ****************************/
int handle(char**,int);
void force_exit();
void init_builtins();
void init_history(int);
void init_env_vars();
void init_shell_vars();

#endif