#define _GNU_SOURCE
#define EXIT_CODE_ZERO       0
#define EXIT_CODE_MINUS_ONE -1

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

bool interactive = true;
const char* wsh_prompt = "wsh> ";
const int n_builtins = 7;
const char* builtins[] = {"cd", "exit", "export", "history", "local", "ls", "vars"};
int last_exit_code = EXIT_CODE_ZERO;

FILE* fd_in = NULL;
FILE* fd_out = NULL;
FILE* fd_err = NULL;

// Clone a string
char* clone_str(const char* str) {
    char* clone = (char*)malloc((strlen(str) + 1) * sizeof(char));
    strcpy(clone, str);
    return clone;
}

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

// Return index of key in dictionary if present, else -1 
int get_shell_var_idx(const char* key) {
    for (int i = 0; i < shell_vars->size; i++) {
        if (strcmp(shell_vars->entries[i]->key, key) == 0)
            return i;
    }
    return -1;
}

entry* make_shell_var_entry(const char* key, const char* val) {
    entry* new_entry = (entry*)malloc(sizeof(entry));
    new_entry->key = clone_str(key);
    new_entry->val = clone_str(val);
    return new_entry;
}

void resize_if_needed() {
    if (shell_vars->size > shell_vars->max_size) {
        shell_vars->max_size *= 2;
        shell_vars->entries = (entry**)realloc(shell_vars->entries, shell_vars->max_size * sizeof(entry*));
    }
}

int add_shell_var(const char* key, const char* val) {
    if (key == NULL || strlen(key) == 0 || val == NULL)
        return -1;
    int idx = get_shell_var_idx(key);
    if (idx == -1) { // add new entry
        shell_vars->size++;
        resize_if_needed();
        shell_vars->entries[shell_vars->size - 1] = make_shell_var_entry(key, val);
    } else // update existing entry
        shell_vars->entries[idx]->val = clone_str(val);
    return 0;
}

char* get_shell_var(const char* key) {
    int idx = get_shell_var_idx(key);
    return (idx != -1) ? clone_str(shell_vars->entries[idx]->val) : NULL;
}

char* get_var(const char* key) {
    char* env_result = getenv(key);
    return (env_result != NULL) ? env_result : get_shell_var(key);
}

void print_strings(char** array, int size, char* delim, char* message) {
    printf("%s", message);
    for (int i = 0; i < size + 1; i++) {
        if (array[i] == NULL)
            printf("NULL");
        else
            printf("%s%s", array[i], delim);
    }
    printf("\n");
}

// TODO(Areeb): remove these later
void print_environ() {
    printf("environ=");
    printf("{");
    for (int i = 0; environ[i] != NULL; i++)
        printf("%s,", environ[i]);
    printf("}\n");
}


void print_shell_vars() {
    printf("size=%d, max_size=%d, dict=", shell_vars->size, shell_vars->max_size);
    printf("{");
    for (int i = 0; i < shell_vars->size; i++) {
        entry* dict_entry = shell_vars->entries[i];
        printf("%s:%s,", dict_entry->key, dict_entry->val);
    }
    printf("}\n");
}

void print_vars() {
    print_environ();
    print_shell_vars();
}

/********************************** DICTIONARY END *******************************/

/***************************** HISTORY START **********************************/

typedef struct circular_queue {
    char** words;
    int n, r, f;
} cqueue;

void init(cqueue* cq, int size) {
    // printf("init\n");
    cq->n = size;
    cq->r = -1;
    cq->f = -1;
    cq->words = (char**)malloc(size * sizeof(char*));
}

cqueue* history;

char* get(cqueue* cq, int no) {
    if (no < 1 || no > cq->n)
        return NULL;
    int i = (cq->f - (no - 1) + cq->n) % cq->n;
    bool is_valid = false;
    if (cq->f == cq->r)
        is_valid = (i == cq->f);
    else if (cq->r < cq->f)
        is_valid = (cq->r <= i && i <= cq->f);
    else
        is_valid = (0 <= i && i <= cq->f) || (cq->r <= i && i < cq->n);
    if (is_valid)
        return clone_str(cq->words[i]);
    return NULL;
}

void push(cqueue* cq, const char* word) {
    // printf("push %s\n", word);
    char* front = get(cq, 1);
    if (front != NULL && strcmp(word, front) == 0)
        return;
    if (cq->r == -1 || cq->r == (cq->f + 1) % cq->n)
        cq->r = (cq->r + 1) % cq->n;
    cq->f = (cq->f + 1) % cq->n;
    cq->words[cq->f] = clone_str(word);
}

void pop(cqueue* cq) {
    // printf("pop\n");
    if (cq->f == cq->r) {
        cq->f = -1;
        cq->r = -1;
    } else
        cq->r = (cq->r + 1) % cq->n;
}

int getsize(cqueue* cq) {
    if (cq->f == -1)
        return 0;
    if (cq->f >= cq->r)
        return cq->f - cq->r + 1;
    return cq->f + 1 + cq->n - cq->r;
}

void print(cqueue* cq, int no) {
    char* word = get(cq, no);
    if (word == NULL)
        printf("NULL\n");
    else
        printf("%s\n", word);
}

void display(cqueue* cq) {
    // printf("n = %d, r = %d, f = %d\n", cq->n, cq->r, cq->f);
    // printf("history->words:\n");
    int i = cq->f, size = getsize(cq);
    while (size--) {
        printf("%d) %s\n", i + 1, cq->words[i]);
        i = (i - 1 + cq->n) % cq->n;
    }
    // printf("\n");
}

void resize(cqueue** cq, int size) {

    // printf("resize to %d\n", size);
    if (size == (*cq)->n)
        return;
    
    if (size < (*cq)->n) {
        int drop = (*cq)->n - size;
        while(drop--)
            pop(*cq);
    }
    
    cqueue* newcq = (cqueue*)malloc(sizeof(cqueue));
    init(newcq, size);

    int i = (*cq)->r, oldsize = getsize(*cq);
    while (oldsize--) {
        push(newcq, (*cq)->words[i]);
        i = (i + 1) % (*cq)->n;
    }
    
    display(newcq);
    free(*cq);
    *cq = newcq;

}

/**************************** HISTORY END ************************************/


/******************** VARNAME DEREFERENCING HELPERS START *******************/

char* dereference(const char* ovarname) {
    if (ovarname == NULL || strcmp(ovarname, "\"\"") == 0)
        return "";
    char* const varname = clone_str(ovarname);
    if (varname[0] != '$')
        return varname;
    char* out = get_var(varname + 1);
    return out ? out : "";
}

void dereferences(char*** varnames, int n_varnames) {
    for (int i = 0; i < n_varnames; i++)
        (*varnames)[i] = dereference((*varnames)[i]);
}

/********************* VARNAME DEREFERENCING HELPERS END ********************/

/******************************* STRING HELPERS START ****************************/

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

void try_delim(const char* otoken, const char* delim) {
    char* token = clone_str(otoken);
    char** tokens = NULL;
    int n_tokens = 0;
    tokenize(token, delim, &tokens, &n_tokens);
    char* joined = join(join("delimiting on", delim, ' '),"gives: ", ' ');
    print_strings(tokens, n_tokens, ",", joined);
}

int redirect_fd_to_file(int fd, const char* file_name, const char* mode) {
    // printf("opening %s\n", file_name);
    
    FILE* file = fopen(file_name, mode);
    // printf("opened %s in %s mode\n", file_name, mode);
    if (file == NULL)
        return -1;
    // int fno = fileno(file);
    // printf("valid fileno: %d\n", fno);
    if (dup2(fileno(file), fd) < 0)
        return -1;
    // printf("dup2 success\n");
    if (close(fileno(file)) < 0)
        return -1;
    // printf("close() success\n");
    return 0;
}

int handle_redirection_if_any(char*** tokens, int* n_tokens, bool* success) {
    
    char* last_token = clone_str((*tokens)[*n_tokens - 1]);

    // printf("---------------------------------------------------\n");
    // printf("last_token: %s\n", last_token);

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
    }
    
    else if (found2 != NULL) { // found > (could also be >>)
        // printf("found >\n");
        redirection = true;

        if (strlen(found2) == 1) // [n]>word without word is invalid
            return -1;
        
        if (*(found2 + 1) == '>') { // >> found
            // printf("found >>\n");
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
            // printf("len_fd = %d\n", len_fd);
            char* fdstr = (char*)malloc((len_fd + 1) * sizeof(char));
            strncpy(fdstr, last_token, len_fd);
            // printf("fdstr = %s\n", fdstr);
            fd = atoi(fdstr);
            // printf("fd = %d\n", fd);
        }

        // int flushed = fflush(stdout);
        // printf("flushed = %d\n", flushed);
        // printf("fd = %d, file_name = %s\n", fd, file_name);
        // printf("fd = %d, file_name = %s, mode = %s\n", fd, file_name, mode);
        fflush(stdout);
        if (redirect_fd_to_file(fd, file_name, mode) < 0)
            return -1;
    }

    else if (found3 != NULL) { // < found
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
        // printf("redirection attempt successful\n");
        free((*tokens)[*n_tokens]);
        // printf("free() successful\n");
        (*tokens)[*n_tokens - 1] = NULL;
        *n_tokens -= 1;
        // printf("reduction successful\n");
    }
    *success = redirection;
    // printf("---------------------------------------------------\n");
    return 0;
}

/**************************** STRING HELPERS END *************************/

/**
 * Return index at which word is present in list, else -1
 */
int is_word_in_list(const char** list, const int size, const char* word) {
    for (int i = 0; i < size; i++) {
        if (strcmp(list[i], word) == 0) // found
            return i;
    }
    return -1;
}

bool is_builtin(const char* command) {
    int idx = is_word_in_list(builtins, n_builtins, command);
    return (idx >=0 && idx < n_builtins);
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

/*************************** BUILT-IN CALLS START ***************************/

void wsh_cd(const char* odir) {
    // printf("wsh_cd() called\n");
    char* const dir = dereference(odir);
    // char* cwd = getcwd(NULL, 0);
    // printf("cwd before cd ../ = %s\n", cwd);
    // printf("running cd %s...\n", dir);
    last_exit_code = chdir(dir);
    // cwd = getcwd(NULL, 0);
    // printf("last_exit_code = %d\n", last_exit_code);
    // printf("cwd after cd ../ = %s\n", cwd);
}

void wsh_exit() {
    // printf("wsh_exit() called\n");
    // last_exit_code ? exit(EXIT_CODE_MINUS_ONE) : exit(EXIT_CODE_ZERO);
    exit(last_exit_code);
}

void wsh_export(const char* otoken) {
    // printf("wsh_export() called\n");
    char* const token = clone_str(otoken);
    char* const key = strtok(token, "=");
    char* const val = dereference(strtok(NULL, "="));
    last_exit_code = setenv(key, val, 1);
    // printf("exported getenv(%s)=%s\n", key, getenv(key));
    // print_vars();
}

void wsh_history() {
    // printf("wsh_history() called\n");
    display(history);
}

void wsh_local(const char* otoken) {
    // printf("wsh_local() called\n");
    char* const token = clone_str(otoken);
    // printf("wsh_local(%s)\n", token);
    char* const key = strtok(token, "=");
    // printf("wsh_local key=%s\n", key);
    char* const val = dereference(strtok(NULL, "="));
    // printf("adding key:val as %s:%s\n", key, val);
    add_shell_var(key, val);
    // print_vars();
}

int non_hidden_dirent(const struct dirent* entry) { return entry->d_name[0] != '.';}

int wsh_ls() {
    // printf("wsh_ls() called\n");
    struct dirent** dirs;
    int n = scandir(".", &dirs, non_hidden_dirent, alphasort);
    if (n == -1)
        last_exit_code = EXIT_CODE_MINUS_ONE;
    else {
        for (int i = 0; i < n; i++)
            printf("%s\n", dirs[i]->d_name);
        fflush(stdout);
        last_exit_code = EXIT_CODE_ZERO;
    }
    return last_exit_code;
}

void wsh_vars() {
    // printf("wsh_vars() called\n");
    for (int i = 0; i < shell_vars->size; i++) {
        entry* dict_entry = shell_vars->entries[i];
        printf("%s=%s\n", dict_entry->key, dict_entry->val);
    }
}

/*************************** BUILT-IN CALLS END ****************************/

void init_history(int size) {
    history = (cqueue*)malloc(sizeof(cqueue));
    init(history, size);
}

void init_env_vars() {
    last_exit_code = putenv("PATH=/bin");
}

void init_shell_vars() {
    shell_vars = (dict*)malloc(sizeof(dict));
    shell_vars->size = 0;
    shell_vars->max_size = 1;
    shell_vars->entries = (entry**)malloc(shell_vars->max_size * sizeof(entry*));
}

int main(int argc, char* argv[]) {

    // printf("argc = %d\n", argc);
    // print_strings(argv, argc, ", ", "argv=");

    if (argc < 1 || argc > 2)
        exit(-1);

    int copy_in = dup(STDIN_FILENO);
    int copy_out = dup(STDOUT_FILENO);
    int copy_err = dup(STDERR_FILENO);

    fflush(stdin);
    fflush(NULL);

    if (argc == 2) {
        interactive = false;
        redirect_fd_to_file(STDIN_FILENO, argv[1], "r");
        copy_in = dup(STDIN_FILENO);
        wsh_prompt = "";
    }

    init_history(5);
    // display(history);
    init_env_vars();
    init_shell_vars();
    // print_vars();

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

        // promptf("line = %s\n", line);

        // Step 2: tokenize without variable expansion
        int n_tokens = 0;
        char** tokens = NULL;
        tokenize(line, " ", &tokens, &n_tokens);
        char* command = tokens[0];

        // printf("command: %s\n", command);
        // printf("n_tokens: %d\n", n_tokens);
        // print_strings(tokens, n_tokens, ",", "tokens = ");

        // Step 3: record history for non built-in commands
        if (!is_builtin(command)) {
            // record history
            // TODO(Areeb): display is reverse of what it should be
            // TODO(Areeb): local x=pwd; $x is not working
            push(history, line);
            // display(history);
        }

        bool redirection = false;
        // Step 4: perform I/O redirection if any
        last_exit_code = handle_redirection_if_any(&tokens, &n_tokens, &redirection);
        // perror("standard error message\n");
        // printf("n_tokens after redirection: %d\n", n_tokens);
        // print_strings(tokens, n_tokens, ",", "tokens after redirection = ");

        // Step 5: expand all variables
        dereferences(&tokens, n_tokens);
        command = tokens[0];
        // print_strings(tokens, n_tokens, ",", "tokens after expansion = ");

        if (strcmp(command,"cd") == 0)
            wsh_cd(tokens[1]);

        // if (strcmp(command,"env") == 0)
        //     print_environ();

        if (strcmp(command, "exit") == 0)
            wsh_exit();

        if (strcmp(command, "export") == 0)
            wsh_export(tokens[1]);

        if (strcmp(command, "history") == 0)
            wsh_history();

        if (strcmp(command, "local") == 0)
            wsh_local(tokens[1]);

        if (strcmp(command, "ls") == 0)
            wsh_ls();
        
        if (strcmp(command, "vars") == 0)
            wsh_vars();

        if (!is_builtin(command)) {
            // printf("non built-in command, exec to execute\n");
            last_exit_code = access(command, X_OK);
            int can_execute = (last_exit_code == 0);
            if (can_execute) {
                // printf("can_execute = 0\n");
                int pid = fork();
                if (pid < 0)
                    printf("fork failed!!\n");
                else if (pid == 0) {
                    char** argv = tokens;
                    char* executable = get_filename_from_path(command);
                    // printf("filename=%s, len=%ld\n", executable, strlen(executable));
                    argv[0] = clone_str(executable);
                    // printf("Child of wsh, exec into %s\n", executable);
                    execv(command, argv);
                    // printf("exec failed\n");
                    exit(EXIT_FAILURE);
                } else {
                    int status = 0;;
                    wait(&status);
                    // printf("child complete, status=%d!\n", status);
                    // printf("WEXITSTATUS=%d\n", WEXITSTATUS(status));
                }
            } else {
                // printf("can not execute, try searching $PATH!!\n");
                char* path = getenv("PATH");
                // printf("PATH=%s\n", path);
                char** paths = NULL;
                int n_paths = 0;
                tokenize(path, ":", &paths, &n_paths);
                // print_strings(paths, n_paths, "\n", "PATH:\n");
                for (int i = 0; i < n_paths; i++) {
                    char* newpath = join(paths[i], command, '/');
                    last_exit_code = access(newpath, X_OK);
                    // printf("last_exit_code = %d\n", last_exit_code);
                    // fflush(stdout);
                    int can_execute = (last_exit_code == 0);
                    if (can_execute) {
                        int pid = fork();
                        if (pid < 0)
                            printf("fork failed!!\n");
                        else if (pid == 0) {
                            char** argv = tokens;
                            // printf("newpath=%s, len=%ld\n", newpath, strlen(newpath));
                            // printf("Child of wsh, exec into %s\n", command);
                            // print_strings(argv, n_tokens, ",", "with argv = ");
                            execv(newpath, argv);
                            // printf("exec failed\n");
                            exit(EXIT_FAILURE);
                        } else {
                            int status = 0;;
                            wait(&status);
                            break;
                            // printf("child complete, status=%d!\n", status);
                            // printf("WEXITSTATUS=%d\n", WEXITSTATUS(status));
                        }
                    }
                }
            }
        }

        // flush sreams before next command
        fflush(stdin);
        fflush(NULL);

        // reset any redirection
        if (redirection) {
            dup2(copy_in, STDIN_FILENO);
            dup2(copy_out, STDOUT_FILENO);
            dup2(copy_err, STDERR_FILENO);
        }

        promptf("");

        free(line);
        line = NULL;
        len = 0;
    }

    wsh_exit();
    // printf("\n");
    return 0;
}