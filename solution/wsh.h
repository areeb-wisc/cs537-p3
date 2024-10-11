#ifndef _WSH_HEADER
#define _WSH_HEADER

#include<ctype.h>
#include<dirent.h>
#include<limits.h>
#include<stdarg.h>
#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>
#include<unistd.h>

/******************************* DICTIONARY START *****************************/

// single dictionary entry (string key, string val) 
typedef struct Entry {
    char* key;
    char* val;
} entry;

/**
 * Dynamic list of Entry (same as vector<Entry> in C++)
 * size is current size, max_size is current capacity
 * Adding a new entry to an already full dictionary doubles max_size
 */
typedef struct Dict {
    int size;
    int max_size;
    entry** entries;

} dict;

// Create a dictionary with given size
dict* create_dictionary(int size);

// Make Entry (key,val)
entry* make_dict_entry(const char* key, const char* val);

// Double dict size if at capacity
void resize_if_needed(dict* dictionary);

// Return index of key in dictionary if present, else -1
int get_dict_idx(dict* dictionary, const char* key);

// Add (key,val) to dict if not present, else update dictionary[key] = val
int add_dict_var(dict* dictionary, const char* key, const char* val);

// Get dictionary[key] if present, else NULL
char* get_dict_var(dict* dictionary, const char* key);

// TODO(Areeb): remove these later
void print_strings(char**, int, char*, char*);
void print_environ();
void print_dict(dict*);
void print_vars();
/********************************** DICTIONARY END *******************************/

/***************************** CIRCULAR QUEUE START **********************************/

/**
 * A circular queue where each entry is a string (called word)
 * Addition (push operation) happens at 'front' or 'f'
 * Deletion (pop operation) happens at 'rear' of 'r'
 * n = max size of queue
 */
typedef struct circular_queue {
    char** words;
    int n, r, f;
} cqueue;

// Create circular queue of given size
cqueue* create_cqueue(int);

// Get 1-indexed k-th element from cqueue counting from front to rear
char* get(cqueue*, int k);

// Push word to cqueue front
void push(cqueue*, const char* word);

// Pop an element from cqueue rear
void pop(cqueue*);

// Get current size of cqueue
int getsize(cqueue*);

// Show cqueue from front to rear
void display(cqueue*);

// Resize cqueue to newsize, dropping elements if needed
void resize(cqueue**, int newsize);

// TODO(Areeb): remove this
void print(cqueue*, int);
/**************************** CIRCULAR QUEUE END ************************************/

/******************** VARNAME DEREFERENCING HELPERS START *******************/
// Dereference single variable
char* dereference(const char*);

// Dereference a list of variables
void dereference_tokens(char*** varnames, int n_varnames);
/********************* VARNAME DEREFERENCING HELPERS END ********************/

/******************************* STRING HELPERS START ****************************/

// Clone a string
char* clone_str(const char*);

// Strip trailing whitespaces from string
void strip(char** word, ssize_t* len);

// produces same output as printf but prefixed with a fixed string "wsh> "
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

// redirect file descriptor 'fd' to 'file_name' opened in 'mode'
int redirect_fd_to_file(int fd, const char* file_name, const char* mode);

/**
 * Handles redirection token and removes it, if present
 * e.g. 
 * Input  :  {"echo","hello","world",">file.txt",NULL}
 * Output :  {"echo,"hello","world",NULL} and perform the redirection
 */
int handle_redirection_if_any(char*** tokens, int* n_tokens, bool* success);

/**
 * Get file name from its path
 * e.g. path/to/file1.txt -> file1.txt
 * e.g. /file2.txt -> file2.txt
 * e.g. file3.txt -> file3.txt
 * e.g. / -> [empty string]
 */
char* get_filename_from_path(const char* path);
/**************************** STRING HELPERS END *************************/

/*************************** NON BUILT-IN CALLS START ************************/
// Call fork(), exec(path,argv)
int execute(char* path, char** argv);

/**
 * Try executing command directly (using absolute/relative path)
 * Then try executing by searching PATH directories
 */
int handle_non_builtin(char** tokens, int n_tokens);
/*************************** NON BUILT-IN CALLS START ************************/

/***************************** BUILT-IN CALLS START ***************************/

bool isValidNumber(const char* numstr);
int non_hidden_dirent(const struct dirent* entry);

/**
 * All builtin functions have this signature: 
 * int (char** tokens, int n_tokens);
 * Return -1 for failure, 0 for success
 */

int wsh_cd(char**,int);
int wsh_exit(char** tokens, int n_tokens);
// TODO(Areeb): [export a] should give error 
int wsh_export(char**,int);
int wsh_history(char**,int);
// TODO(Areeb): [local a] should give error
int wsh_local(char**,int);
int wsh_ls(char**,int);
int wsh_vars(char**,int);

// Array of wsh built-in functions
static int (*wshcalls[])(char**,int) = 
{wsh_cd, wsh_exit, wsh_export, wsh_history, wsh_local, wsh_ls, wsh_vars};

// Check if 'command' is a built-in
bool is_builtin(const char* command);

/**
 * Step 1: Check if arguments are valid/within range
 * Step 2: Call correct built-in function using wshcalls[]
 */
int handle_builtin(char**,int);
/*************************** BUILT-IN CALLS END ****************************/

/*************************** MAIN DECLARATIONS ****************************/

// generic handler for any command
int handle(char** tokens, int n_tokens);

// initialize builtins dictionary
void init_builtins();

// initialize history circular queue
void init_history(int);

// set initial PATH value
void init_env_vars();

// initialize shell_vars dictionary
void init_shell_vars();

// call exit for Ctrl+D or EOF
void force_exit();

#endif