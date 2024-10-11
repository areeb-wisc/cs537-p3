#define _GNU_SOURCE
#define EXIT_CODE_ZERO       0
#define EXIT_CODE_MINUS_ONE -1

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "wsh.h"

int initial_history_size = 5; // --------- initial history size set to 5
const char* wsh_prompt = "wsh> "; // ----- const wsh prompt "wsh> "
int last_exit_code = EXIT_CODE_ZERO; // -- global last seen exit_code

int copy_in; // -------------------------- copy of stdin for resetting
int copy_out; // ------------------------- copy of stdout for resetting
int copy_err; // ------------------------- copy of stderr for resetting

dict* shell_vars; // --------------------- dictionary to hold shell local variables               
cqueue* history; // ---------------------- circular queue to store history
dict* builtins; // ----------------------- dictionary to hold builtins

// ------------------------------- DICTIONARY START -------------------------------

dict* create_dictionary(int maxsize) {
    dict* dictionary = (dict*)malloc(sizeof(dict));
    dictionary->size = 0;
    dictionary->max_size = maxsize;
    dictionary->entries = (entry**)malloc(dictionary->max_size * sizeof(entry*));
    return dictionary;
}

void free_dict(dict* dictionary) {
    for (int i = 0; i < dictionary->size; i++)
        free(dictionary->entries[i]);
    free(dictionary->entries);
    free(dictionary);
}

entry* make_dict_entry(const char* key, const char* val) {
    entry* new_entry = (entry*)malloc(sizeof(entry));
    new_entry->key = clone_str(key);
    new_entry->val = clone_str(val);
    return new_entry;
}

void resize_if_needed(dict* dictionary) {
    if (dictionary->size > dictionary->max_size) {
        dictionary->max_size *= 2;
        dictionary->entries = (entry**)realloc(dictionary->entries, dictionary->max_size * sizeof(entry*));
    }
}

int get_dict_idx(dict* dictionary, const char* key) {
    for (int i = 0; i < dictionary->size; i++) {
        if (strcmp(dictionary->entries[i]->key, key) == 0)
            return i;
    }
    return -1;
}

int add_dict_var(dict* dictionary, const char* key, const char* val) {
    if (key == NULL || strlen(key) == 0 || val == NULL)
        return -1;
    int idx = get_dict_idx(dictionary, key);
    if (idx == -1) { // add new entry
        dictionary->size++;
        resize_if_needed(dictionary);
        dictionary->entries[dictionary->size - 1] = make_dict_entry(key, val);
    } else // update existing entry
        dictionary->entries[idx]->val = clone_str(val);
    return 0;
}

char* get_dict_var(dict* dictionary, const char* key) {
    if (key == NULL)
        return NULL;
    int idx = get_dict_idx(dictionary, key);
    return (idx != -1) ? clone_str(dictionary->entries[idx]->val) : NULL;
}

// -------------------------------- DICTIONARY END --------------------------------

// -------------------------------- HISTORY START ---------------------------------

cqueue* create_cqueue(int size) {
    cqueue* cq = (cqueue*)malloc(sizeof(cqueue));
    cq->n = size;
    cq->r = -1;
    cq->f = -1;
    cq->words = (char**)malloc(size * sizeof(char*));
    return cq;
}

void free_cq(cqueue* cq) {
    for (int  i = 0; i < cq->n; i++)
        free(cq->words[i]);
    free(cq->words);
    free(cq);
}

char* get(cqueue* cq, int no) {
    if (no < 1 || no > cq->n)
        return NULL;
    int i = (cq->r - (no - 1) + cq->n) % cq->n;
    bool is_valid = false;
    if (cq->r == cq->f)
        is_valid = (i == cq->r);
    else if (cq->f < cq->r)
        is_valid = (cq->f <= i && i <= cq->r);
    else
        is_valid = (0 <= i && i <= cq->r) || (cq->f <= i && i < cq->n);
    if (is_valid)
        return clone_str(cq->words[i]);
    return NULL;
}

void push(cqueue* cq, const char* word) {
    if (cq->n == 0)
        return;
    char* rear = get(cq, 1);
    if (rear != NULL && strcmp(word, rear) == 0)
        return;
    if (cq->f == -1 || cq->f == (cq->r + 1) % cq->n)
        cq->f = (cq->f + 1) % cq->n;
    cq->r = (cq->r + 1) % cq->n;
    cq->words[cq->r] = clone_str(word);
}

void pop(cqueue* cq) {
    if (cq->f == cq->r) {
        cq->f = -1;
        cq->r = -1;
    } else
        cq->f = (cq->f + 1) % cq->n;
}

int getsize(cqueue* cq) {
    if (cq->r == -1)
        return 0;
    if (cq->r >= cq->f)
        return cq->r - cq->f + 1;
    return cq->r + 1 + cq->n - cq->f;
}

void display(cqueue* cq) {
    int i = cq->r, k = 0, size = getsize(cq);
    while (k < size) {
        printf("%d) %s\n", ++k, cq->words[i]);
        i = (i - 1 + cq->n) % cq->n;
    }
    fflush(stdout);
}

void resize(cqueue** cq, int size) {
    if (size < 0 || size == (*cq)->n)
        return;
    if (size < (*cq)->n) {
        int drop = getsize(*cq) - size;
        while(drop--)
            pop(*cq);
    }
    cqueue* newcq = create_cqueue(size);
    int i = (*cq)->f, oldsize = getsize(*cq);
    while (oldsize--) {
        push(newcq, (*cq)->words[i]);
        i = (i + 1) % (*cq)->n;
    }
    free(*cq);
    *cq = newcq;
}

// --------------------------------- HISTORY END ----------------------------------

// --------------------- VARNAME DEREFERENCING HELPERS START ----------------------

char* dereference(const char* ovarname) {
    if (ovarname == NULL)
        return "";
    char* const varname = clone_str(ovarname);
    if (varname[0] != '$')
        return varname;
    char* env_result = getenv(varname + 1);
    if (env_result != NULL) 
        return env_result;
    char* out = get_dict_var(shell_vars, varname + 1);
    return out ? out : "";
}

void dereference_tokens(char*** varnames, int n_varnames) {
    for (int i = 0; i < n_varnames; i++)
        (*varnames)[i] = dereference((*varnames)[i]);
}

// ---------------------- VARNAME DEREFERENCING HELPERS END -----------------------

// ----------------------------- STRING HELPERS START -----------------------------

char* clone_str(const char* str) {
    char* clone = (char*)malloc((strlen(str) + 1) * sizeof(char));
    strcpy(clone, str);
    return clone;
}

void strip(char** word, ssize_t* len) {
    if (*len > 0) {
        int i = 0, j = *len - 1;
        while (i < *len && isspace((*word)[i]))
            i++;
        while (j > i && isspace((*word)[j]))
            j--;
        *len = (i <= j) ? j - i + 1 : 0;
        char* stripped = (char*)malloc((*len + 1) * sizeof(char));
        for (int k = i; k <= j; k++)
            stripped[k-i] = (*word)[k];
        stripped[*len] = '\0';
        char* original = *word;
        *word = stripped;
        free(original);
    }
}

void promptf(char* fmtstr, ...) {
    va_list params;
    va_start(params, fmtstr);
    int len = 2*strlen(fmtstr);
    char* buff1 = (char*)malloc(len * sizeof(char));
    int out = vsnprintf(buff1, len, fmtstr, params);
    va_start(params, fmtstr);
    char* buff2 = (char*)malloc((out + 1) * sizeof(char));
    vsnprintf(buff2, out + 1, fmtstr, params);
    printf("%s%s", wsh_prompt, buff2);
    fflush(stdout);
    va_end(params);
    free(buff1);
    free(buff2);
}

/**
 * Tokenize the given input string using delimiter = delim
 * Also appends a NULL to the output array for exec argv
 * e.g.
 * Input: "echo hello world"
 * Output: {"echo", "hello", "world", NULL}
 */
void tokenize(const char* oline, const char* delim, char*** ptokens, int* n_tokens) {

    char* const line = clone_str(oline);
    int buff_size = 0;
    int max_buff_size = 1;
    char** tokens = (char**)malloc(buff_size * sizeof(char*));

    char* token = strtok(line, delim);
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
}

/**
 * Joins 2 strings using 'joiner'
 * e.g. ("str1","",'/') -> "str1/"
 * e.g. ("","str2",'/') -> "/str2"
 * e.g. ("","",'*') -> "*"
 * e.g. ("path/to","filename",'/) -> "path/to/filename"
 */
char* join(const char* str1, const char* str2, const char joiner) {
    int len1 = strlen(str1), len2 = strlen(str2);
    char* joined = (char*)malloc((len1 + len2 + 2) * sizeof(char));
    strcpy(joined, str1);
    joined[len1] = joiner;
    joined[len1 + 1] = '\0';
    strcat(joined, str2);
    return joined;
}

int redirect_fd_to_file(int fd, const char* file_name, const char* mode) {
    FILE* file = fopen(file_name, mode);
    if (file == NULL)
        return -1;
    if (dup2(fileno(file), fd) < 0)
        return -1;
    if (close(fileno(file)) < 0)
        return -1;
    return 0;
}

int handle_redirection_if_any(char*** tokens, int* n_tokens, bool* success) {
    char* last_token = clone_str((*tokens)[*n_tokens - 1]);
    bool redirection = false;
    int fd = -1;
    char* file_name = NULL;
    char* mode = NULL;
    char *found1 = NULL, *found2 = NULL, *found3 = NULL;
    // Check for &> and &>>
    found1 = strstr(last_token, "&>");
    // Check for > and >>
    found2 = strstr(last_token, ">");
    // Check for <
    found3 = strstr(last_token, "<");
    if (found1 != NULL) { // found &> (could be &>> also)
        redirection = true;
        if (found1 != last_token) // [n]&> is invalid
            return -1;
        if (strlen(found1) == 2)
            return -1; // &> without word is invalid
        if (*(found1 + 2) == '>') { // &>> found
            if (strlen(found1) == 3) // &>> without word is invalid
                return -1;
            file_name = dereference(found1 + 3);
            mode = "a";
        } else {
            file_name = dereference(found1 + 2);
            mode = "w";
        }
        fflush(stdout);
        if (redirect_fd_to_file(STDOUT_FILENO, file_name, mode) < 0)
            return -1;
        fflush(stderr);
        if (redirect_fd_to_file(STDERR_FILENO, file_name, mode) < 0)
            return -1;
    } else if (found2 != NULL) { // found > (could also be >>)
        redirection = true;
        if (strlen(found2) == 1) // [n]>word without word is invalid
            return -1;        
        if (*(found2 + 1) == '>') { // >> found
            if (strlen(found2) == 2) // [n]>>word without word is invalid
                return -1;
            file_name = dereference(found2 + 2);
            mode = "a";
        } else {
            file_name = dereference(found2 + 1);
            mode = "w";
        }
        if (found2 == last_token) // [n]>word, n = 1 then
            fd = STDOUT_FILENO;
        else {
            int len_fd = found2 - last_token;
            char* fdstr = (char*)malloc((len_fd + 1) * sizeof(char));
            strncpy(fdstr, last_token, len_fd);
            fd = atoi(fdstr);
        }
        fflush(stdout);
        if (redirect_fd_to_file(fd, file_name, mode) < 0)
            return -1;
    } else if (found3 != NULL) { // < found
        redirection = true;
        if (strlen(found3) == 1) // [n]<word without word is invalid
            return -1;
        file_name = dereference(found3 + 1);
        if (found3 == last_token)
            fd = STDIN_FILENO;
        else {
            int len_fd = found3 - last_token;
            char* fdstr = (char*)malloc((len_fd + 1) * sizeof(char));
            strncpy(fdstr, last_token, len_fd);
            fd = atoi(fdstr);
        }
        fflush(stdin);
        if (redirect_fd_to_file(fd, file_name, "r") < 0)
            return -1;
    }
    if (redirection) {
        free((*tokens)[*n_tokens]);
        (*tokens)[*n_tokens - 1] = NULL;
        *n_tokens -= 1;
    }
    *success = redirection;
    return 0;
}

/**
 * Get file name from its path
 * e.g. path/to/file1.txt -> file1.txt
 * e.g. /file2.txt -> file2.txt
 * e.g. file3.txt -> file3.txt
 * e.g. / -> [empty string]
 */
char* get_filename_from_path(const char* path) {
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

// ------------------------------ STRING HELPERS END ------------------------------

// --------------------------- NON BUILT-IN CALLS START ---------------------------

int execute(char* path, char** argv) {
    int pid = fork();
    if (pid < 0) {
        return -1;
    } else if (pid == 0) {
        execv(path, argv);
        exit(-1);
    } else {
        if (waitpid(pid, 0, 0) == -1)
            return -1;
    }
    return 0;
}

int handle_non_builtin(char** tokens, int n_tokens) {
    char* command = tokens[0];
    char** argv = tokens;
    n_tokens = n_tokens;
    if (access(command, X_OK) == 0) {
        argv[0] = get_filename_from_path(command);
        return execute(command, argv);
    }
    char* path = getenv("PATH");
    char** paths = NULL;
    int n_paths = 0;
    tokenize(path, ":", &paths, &n_paths);
    for (int i = 0; i < n_paths; i++) {
        char* newpath = join(paths[i], command, '/');
        if (access(newpath, X_OK) == 0)
            return execute(newpath, argv);
    }
    return -1;
}

// ---------------------------- NON BUILT-IN CALLS END ----------------------------

// ----------------------------- BUILT-IN CALLS START -----------------------------

bool isValidNumber(const char* numstr) {
    if (numstr == NULL || strlen(numstr) == 0)
            return false;
    char* buff = (char*)malloc((strlen(numstr) + 1) * sizeof(char));
    strtol(numstr, &buff, 10);
    if (strcmp(buff,"") != 0)
        return false;
    return true;
}

int wsh_cd(char** tokens, int n_tokens) {
    if (n_tokens < 1 || n_tokens > 2)
        return -1;
    char* odir = tokens[1];
    char* const dir = dereference(odir);
    if (chdir(dir) < 0)
        return -1;
    return 0;
}

int wsh_exit(char** tokens, int n_tokens) {
    if (n_tokens != 1)
        return -1;
    tokens = tokens;
    free_dict(shell_vars);
    free_dict(builtins);
    free_cq(history);
    exit(last_exit_code);
    return 0;
}

int wsh_export(char** tokens, int n_tokens) {
    if (n_tokens != 2)
        return -1;
    const char* otoken = tokens[1];
    char* const token = clone_str(otoken);
    char* const key = strtok(token, "=");
    const char* val = strtok(NULL, "=");
    if (val == NULL) { // either [export abc] OR [export abc=] case
        if (otoken[strlen(otoken) - 1] != '=') // [export abc] is invalid
            return -1;
        val = "";
    }
    val = dereference(val);
    if(setenv(key, val, 1) < 0)
        return -1;
    return 0;
}

int wsh_history(char** tokens, int n_tokens) {
    if (n_tokens < 1 || n_tokens > 3)
        return -1;
    if (n_tokens == 1) // [history]
        display(history);
    else if (n_tokens == 3) { // [history set <n>]
        if (strcmp(tokens[1],"set") != 0)
            return -1;
        if (!isValidNumber(tokens[2]))
            return -1;
        resize(&history, atoi(tokens[2]));
    } else { // [history n]
        if (!isValidNumber(tokens[1]))
            return -1;
        int line_number = atoi(tokens[1]);
        char* line = get(history, line_number);
        if (line == NULL) // ignoring unpopulated entries
            return 0;
        char** line_tokens = NULL;
        int n_line_tokens = 0;
        tokenize(line, " ", &line_tokens, &n_line_tokens);
        return handle(line_tokens, n_line_tokens);
    }
    return 0;
}

int wsh_local(char** tokens, int n_tokens) {
    if (n_tokens != 2)
        return -1;
    const char* otoken = tokens[1];
    char* const token = clone_str(otoken);
    char* const key = strtok(token, "=");
    const char* val = strtok(NULL, "=");
    if (val == NULL) { // either [local abc] OR [local abc=] case
        if (otoken[strlen(otoken) - 1] != '=') // [local abc] is invalid
            return -1;
        val = "";
    }
    val = dereference(val);
    if (add_dict_var(shell_vars, key, val) < 0)
        return -1;
    return 0;
}

int non_hidden_dirent(const struct dirent* entry) { return entry->d_name[0] != '.';}

int wsh_ls(char** tokens, int n_tokens) {
    if (n_tokens != 1)
        return -1;
    tokens = tokens;
    struct dirent** dirs;
    int n = scandir(".", &dirs, non_hidden_dirent, alphasort);
    if (n == -1)    
        return -1;
    for (int i = 0; i < n; i++)
        printf("%s\n", dirs[i]->d_name);
    fflush(stdout);
    return 0;
}

int wsh_vars(char** tokens, int n_tokens) {
    if (n_tokens != 1)
        return -1;
    tokens = tokens;
    for (int i = 0; i < shell_vars->size; i++) {
        entry* dict_entry = shell_vars->entries[i];
        printf("%s=%s\n", dict_entry->key, dict_entry->val);
    }
    fflush(stdout);
    return 0;
}

bool is_builtin(const char* command) {
    return get_dict_idx(builtins, command) != -1;
}

int handle_builtin(char** tokens, int n_tokens) {
    char* command = tokens[0];
    int idx = atoi(get_dict_var(builtins, command));
    return wshcalls[idx](tokens, n_tokens);
}

// ------------------------------ BUILT-IN CALLS END ------------------------------

// ------------------------------ MAIN HELPERS START ------------------------------

int reset_redirection() {
    if (dup2(copy_in, STDIN_FILENO) < 0)
        return -1;
    if (dup2(copy_out, STDOUT_FILENO) < 0)
        return -1;
    if (dup2(copy_err, STDERR_FILENO) < 0)
        return -1;
    return 0;
}

int handle(char** tokens, int n_tokens) {
    int exit_code = -1;
    // Step 4: perform I/O redirection if any
    bool redirection = false;    
    exit_code = handle_redirection_if_any(&tokens, &n_tokens, &redirection);
    if (exit_code  < 0) {
        // reset any redirection
        if (redirection)
            reset_redirection();
        return -1;
    }
    // Step 5: expand all variables
    dereference_tokens(&tokens, n_tokens);
    // Step 6: handle built-in / non built-in command
    if (is_builtin(tokens[0]))
        exit_code = handle_builtin(tokens, n_tokens);
    else
        exit_code = handle_non_builtin(tokens, n_tokens);
    // flush streams before next command
    fflush(stdin);
    fflush(NULL);
    // reset any redirection
    if (redirection)
        exit_code = reset_redirection();
    return exit_code;
}

void force_exit() {
    char* tokens[] = {"exit"};
    wsh_exit(tokens, 1);
}

void init_builtins() {
    builtins = create_dictionary(7);
    add_dict_var(builtins, "cd",      "0");
    add_dict_var(builtins, "exit",    "1");
    add_dict_var(builtins, "export",  "2");
    add_dict_var(builtins, "history", "3");
    add_dict_var(builtins, "local",   "4");
    add_dict_var(builtins, "ls",      "5");
    add_dict_var(builtins, "vars",    "6");
}

void init_history(int size) { history = create_cqueue(size); }

void init_env_vars() { last_exit_code = putenv("PATH=/bin"); }

void init_shell_vars() { shell_vars = create_dictionary(5); }

// ------------------------------- MAIN HELPERS END -------------------------------

int main(int argc, char* argv[]) {
    if (argc < 1 || argc > 2)
        exit(-1);
    copy_in = dup(STDIN_FILENO);
    copy_out = dup(STDOUT_FILENO);
    copy_err = dup(STDERR_FILENO);
    fflush(stdin);
    fflush(NULL);
    if (argc == 2) {
        if (redirect_fd_to_file(STDIN_FILENO, argv[1], "r") < 0)
            exit(-1);
        copy_in = dup(STDIN_FILENO);
        wsh_prompt = "";
    }
    init_history(initial_history_size);
    init_env_vars();
    init_shell_vars();
    init_builtins();
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    promptf("");
    // Step 1: take user input
    while ((read = getline(&line, &len, stdin)) != -1) {
        strip(&line, &read); // strip trailing whitespaces
        if (read == 0) { // ignore blank input lines
            promptf("");
            continue;
        }
        if (line[0] == '#') { // ignore comments
            promptf("");
            continue;
        }
        // Step 2: tokenize without variable expansion
        int n_tokens = 0;
        char** tokens = NULL;
        tokenize(line, " ", &tokens, &n_tokens);
        // Step 3: record history for non built-in commands
        if (!is_builtin(tokens[0]))
            push(history, line);
        last_exit_code = handle(tokens, n_tokens);
        promptf("");
        free(line); line = NULL; len = 0;
    }
    force_exit();
    return 0;
}