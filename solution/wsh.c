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

bool interactive = true;
char* wsh_prompt = "wsh> ";
int n_builtins = 7;
char* builtins[] = {"cd", "exit", "export", "history", "local", "ls", "vars"};

typedef struct Entry {
    char* key;
    char* val;
} entry;

typedef struct Dict {
    int size;
    int max_size;
    entry** entries;

} dict;

// Clone a string
char* clone_str(char* str) {
    char* clone = (char*)malloc((strlen(str) + 1) * sizeof(char));
    strcpy(clone, str);
    return(clone);
}

// Return index of key in dictionary if present, else -1 
int get_idx(dict* shell_vars, char* key) {
    for (int i = 0; i < shell_vars->size; i++) {
        if (strcmp(shell_vars->entries[i]->key, key) == 0)
            return i;
    }
    return -1;
}

entry* make_entry(char* key, char* val) {
    entry* new_entry = (entry*)malloc(sizeof(entry));
    new_entry->key = clone_str(key);
    new_entry->val = clone_str(val);
    return new_entry;
}

void add_entry(dict* shell_vars, char* key, char* val) {
    int idx = get_idx(shell_vars, key);
    if (idx == -1) { // add new entry
        shell_vars->size++;
        if (shell_vars->size > shell_vars->max_size) {
            shell_vars->max_size *= 2;
            shell_vars->entries = (entry**)realloc(shell_vars->entries, shell_vars->max_size * sizeof(entry*));
        }
        shell_vars->entries[shell_vars->size - 1] = make_entry(key, val);
    } else // update existing entry
        shell_vars->entries[idx]->val = clone_str(val);
}

char* get_val(dict* shell_vars, char* key) {
    int idx = get_idx(shell_vars, key);
    if (idx == -1)
        return NULL;
    return shell_vars->entries[idx]->val;
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
    va_end(params);
    free(buff1);
    free(buff2);
}

/**
 * Return index at which word is present in list, else -1
 */
int is_word_in_list(char** list, int size, char* word) {
    for (int i = 0; i < size; i++) {
        if (strcmp(list[i], word) == 0) // found
            return i;
    }
    return -1;
}

bool is_builtin(char* command) {
    int idx = is_word_in_list(builtins, n_builtins, command);
    return (idx >=0 && idx < n_builtins);
}

/**
 * Tokenize the given input string using delimiter = ' '
 * Also appends a NULL to the output array for exec argv
 * e.g.
 * Input: "echo hello world"
 * Output: {"echo", "hello", "world", NULL}
 */
void tokenize(char* line, char*** ptokens, int* n_tokens) {
    
    int buff_size = 0;
    int max_buff_size = 1;
    char** tokens = (char**)malloc(buff_size * sizeof(char*));

    char* token = strtok(line, " ");
    while (token != NULL) {
        buff_size++;
        if (buff_size > max_buff_size) {
            max_buff_size *= 2;
            tokens = (char**)realloc(tokens, max_buff_size * sizeof(char*));
        }
        tokens[buff_size - 1] = clone_str(token);
        token = strtok(NULL, " ");
    }
    tokens = (char**)realloc(tokens, (buff_size + 1) * sizeof(char*));
    tokens[buff_size] = NULL;

    *ptokens = tokens; // list of all token strings (including NULL)
    *n_tokens = buff_size + 1; // size includes NULL
}

void print_dict(dict* shell_vars) {
    printf("size=%d, max_size=%d, dict=", shell_vars->size, shell_vars->max_size);
    printf("{");
    for (int i = 0; i < shell_vars->size; i++) {
        entry* dict_entry = shell_vars->entries[i];
        printf("%s:%s,", dict_entry->key, dict_entry->val);
    }
    printf("}\n");
}

dict* shell_vars;

void local(char* token) {
    char* key = strtok(token, "=");
    char* val = strtok(NULL, "=");
    add_entry(shell_vars, key, val);
    print_dict(shell_vars);
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

    dict shvars;
    shell_vars = &shvars;
    //  = (dict*)malloc(sizeof(dict));

    print_dict(shell_vars);

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

        if (strcmp(command, "local") == 0) {
            local(tokens[1]);
        }

        // if (strcmp(command, "ls") == 0) {
        //     int pid = fork();
        //     if (pid < 0)
        //         printf("fork failed!!\n");
        //     else if (pid == 0) {
        //         printf("Child of wsh, exec into ls\n");
        //         char** argv = tokens;
        //         execv("/bin/ls", argv);
        //         printf("exec failed\n");
        //         exit(EXIT_FAILURE);
        //     } else {
        //         int status = 0;;
        //         wait(&status);
        //         printf("child complete, status=%d!\n", status);
        //         printf("WEXITSTATUS=%d\n", WEXITSTATUS(status));
        //     }
        // }

        promptf("");
    }

    printf("exiting...\n");

    return 0;
}