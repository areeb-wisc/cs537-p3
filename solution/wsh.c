#define _GNU_SOURCE

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

// Clone a string
char* clone_str(const char* str) {
    char* clone = (char*)malloc((strlen(str) + 1) * sizeof(char));
    strcpy(clone, str);
    return(clone);
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

void add_shell_var(const char* key, const char* val) {
    int idx = get_shell_var_idx(key);
    if (idx == -1) { // add new entry
        shell_vars->size++;
        resize_if_needed();
        shell_vars->entries[shell_vars->size - 1] = make_shell_var_entry(key, val);
    } else // update existing entry
        shell_vars->entries[idx]->val = clone_str(val);
}

char* get_shell_var(const char* key) {
    int idx = get_shell_var_idx(key);
    return (idx != -1) ? shell_vars->entries[idx]->val : NULL;
}

char* get_var(const char* key) {
    char* env_result = getenv(key);
    return (env_result != NULL) ? env_result : get_shell_var(key);
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
    va_end(params);
    free(buff1);
    free(buff2);
}

/**
 * Tokenize the given input string using delimiter = ' '
 * Also appends a NULL to the output array for exec argv
 * e.g.
 * Input: "echo hello world"
 * Output: {"echo", "hello", "world", NULL}
 */
void tokenize(const char* oline, char*** ptokens, int* n_tokens) {

    char* const line = clone_str(oline);
    printf("tokenize\n");
    int buff_size = 0;
    int max_buff_size = 1;
    char** tokens = (char**)malloc(buff_size * sizeof(char*));

    // printf("right before strtok\n");
    char* token = strtok(line, " ");
    // printf("right after strtok\n");
    while (token != NULL) {
        buff_size++;
        if (buff_size > max_buff_size) {
            max_buff_size *= 2;
            tokens = (char**)realloc(tokens, max_buff_size * sizeof(char*));
        }
        tokens[buff_size - 1] = token;
        token = strtok(NULL, " ");
    }
    tokens = (char**)realloc(tokens, (buff_size + 1) * sizeof(char*));
    tokens[buff_size] = NULL;

    *ptokens = tokens; // list of all token strings (including NULL)
    *n_tokens = buff_size + 1; // size includes NULL
    printf("tokenize complete\n");
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

/******************** VARNAME DEREFERENCING HELPERS START *******************/

char* dereference(const char* ovarname) {
    char* const varname = clone_str(ovarname);
    if (varname[0] != '$')
        return varname;
    char* out = get_var(varname + 1);
    return out ? out : "";
}

char** dereferences(char** varnames, int n_varnames) {
    char** out = (char**)malloc(n_varnames * sizeof(char*));
    for (int i = 0; i < n_varnames; i++)
        out[i] = clone_str(dereference(varnames[i]));
    return out;
}

/********************* VARNAME DEREFERENCING HELPERS END ********************/

/*************************** BUILT-IN CALLS START ***************************/

void wsh_cd(const char* odir) {
    printf("wsh_cd() called\n");
    char* const dir = dereference(odir);
    char* cwd = getcwd(NULL, 0);
    printf("cwd before cd ../ = %s\n", cwd);
    printf("running cd %s...\n", dir);
    int ret = chdir(dir);
    cwd = getcwd(NULL, 0);
    printf("ret = %d\n", ret);
    printf("cwd after cd ../ = %s\n", cwd);
}

void wsh_exit() {
    printf("wsh_exit() called\n");
    exit(0);
}

void wsh_export(const char* otoken) {
    printf("wsh_export() called\n");
    char* const token = clone_str(otoken);
    char* const key = strtok(token, "=");
    char* const val = dereference(strtok(NULL, "="));
    setenv(key, val, 1);
    printf("exported getenv(%s)=%s\n", key, getenv(key));
    print_vars();
}

void wsh_history() {
    printf("wsh_history() called\n");
}

void wsh_local(const char* otoken) {
    printf("wsh_local() called\n");
    char* const token = clone_str(otoken);
    printf("wsh_local(%s)\n", token);
    char* const key = strtok(token, "=");
    printf("wsh_local key=%s\n", key);
    char* const val = dereference(strtok(NULL, "="));
    printf("adding key:val as %s:%s\n", key, val);
    add_shell_var(key, val);
    print_vars();
}

#ifdef _DIRENT_HAVE_D_TYPE
bool a = true;
#endif

int non_hidden_dirent(const struct dirent* entry) { return entry->d_name[0] != '.';}

void wsh_ls() {
    printf("wsh_ls() called\n");
    printf("a=%d\n", a);
    struct dirent** dirs;
    int n = scandir(".", &dirs, non_hidden_dirent, alphasort);
    for (int i = 0; i < n; i++)
        printf("%s, %c\n", dirs[i]->d_name, dirs[i]->d_type);
    printf("\n");
}

void wsh_vars() {
    printf("wsh_vars() called\n");
}

/*************************** BUILT-IN CALLS END ****************************/

void init_shell_vars() {
    shell_vars = (dict*)malloc(sizeof(dict));
    shell_vars->size = 0;
    shell_vars->max_size = 1;
    shell_vars->entries = (entry**)malloc(shell_vars->max_size * sizeof(entry*));
}

int main(int argc, char* argv[]) {

    if (argc < 1 || argc > 2)
        exit(-1);

    FILE* fd = stdin;

    if (argc == 2) {
        interactive = false;
        fd = fopen(argv[1], "r");
        if (fd == NULL)
            exit(-1);
        wsh_prompt = "";
    }

    init_shell_vars();
    print_vars();

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    promptf("");

    while ((read = getline(&line, &len, fd)) != -1) {

        strip(&line, &read);

        if (read == 0) { // ignore blank input lines
            promptf("");
            continue;
        }

        if (line[0] == '#') { // ignore comments
            promptf("");
            continue;
        }

        int n_tokens = 0;
        char** tokens = NULL;
        tokenize(line, &tokens, &n_tokens);
        char* command = tokens[0];

        printf("command: %s\n", command);
        printf("n_tokens: %d\n", n_tokens);
        printf("tokens: ");
        for (int i = 0; i < n_tokens; i++) {
            if (tokens[i] == NULL)
                printf("NULL");
            else
                printf("%s, ", tokens[i]);
        }
        printf("\n");

        if (!is_builtin(command)) { // 

        }

        if (strcmp(command,"cd") == 0)
            wsh_cd(tokens[1]);

        if (strcmp(command, "exit") == 0)
            wsh_exit();

        if (strcmp(command, "export") == 0) {
            wsh_export(tokens[1]);
            int pid = fork();
            if (pid < 0)
                printf("fork failed!!\n");
            else if (pid == 0) {
                printf("IN child\n");
                printf("get_shell_var(aa)=%s\n", get_shell_var("aa"));
                wsh_local("aa=100");
                printf("still in child get_shell_var(aa)=%s\n", get_shell_var("aa"));
                wsh_export("aa=200");
                printf("still in child get_env(aa)=%s\n", getenv("aa"));
                printf("getenv(dd)=%s\n", getenv("dd"));
                printf("Child of wsh, exec into ls\n");
                char* argv[] = {"ls", "-A", NULL};
                execv("/bin/ls", argv);
                printf("exec failed\n");
                exit(EXIT_FAILURE);
            } else {
                int status = 0;;
                wait(&status);
                printf("child complete, status=%d!\n", status);
                printf("WIFEXITED=%d\n", WIFEXITED(status));
                printf("WEXITSTATUS=%d\n", WEXITSTATUS(status));
                printf("WIFSIGNALED=%d\n", WIFSIGNALED(status));
                printf("WTERMSIG=%d\n", WTERMSIG(status));
                printf("WIFSTOPPED=%d\n", WIFSTOPPED(status));
                printf("WSTOPSIG=%d\n", WSTOPSIG(status));
                printf("WIFCONTINUED=%d\n", WIFCONTINUED(status));
                printf("in parent get_shell_var(aa)=%s\n", get_shell_var("aa"));
                printf("in parent get_env(aa)=%s\n", getenv("aa"));
            }
        }

        if (strcmp(command, "history") == 0)
            wsh_history();

        if (strcmp(command, "local") == 0)
            wsh_local(tokens[1]);

        // if (strcmp(command, "ls") == 0)
        //     wsh_ls();
        
        if (strcmp(command, "vars") == 0)
            wsh_vars();

        if (strcmp(command, "ls") == 0) {
            int pid = fork();
            if (pid < 0)
                printf("fork failed!!\n");
            else if (pid == 0) {
                printf("Child of wsh, exec into ls\n");
                char** argv = tokens;
                execv("/bin/ls", argv);
                printf("exec failed\n");
                exit(EXIT_FAILURE);
            } else {
                int status = 0;;
                wait(&status);
                printf("child complete, status=%d!\n", status);
                printf("WEXITSTATUS=%d\n", WEXITSTATUS(status));
            }
        }

        promptf("");
    }

    printf("\n");

    return 0;
}

/**
 * cd dir1 $dir2
 * export key1=val1
 * export key1=$var1
 * exit
 * 
 * 
 * 
 */