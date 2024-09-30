#define _GNU_SOURCE

#include<ctype.h>
#include<limits.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>

bool interactive = true;
char* wsh_prompt = "wsh> ";

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

void prompt(char* line) {
    printf("%s%s\n",wsh_prompt,line);
}

void skeleton(char* word) {
    for (int i = 0; word[i] != '\0'; i++) {
        if (word[i] == ' ')
            printf("W");
        else if (word[i] == '\n')
            printf("N");
        else
            printf("%c", word[i]);
    }
    printf("\n");
}

int main(int argc, char* argv[]) {

    if (argc < 1 || argc > 2)
        exit(-1);

    FILE* fd = stdin;

    if (argc == 2) {
        interactive = false;
        fd = fopen(argv[1], "r");
        wsh_prompt = "";
    }

    printf("interactive = %d\n", interactive);
    printf("wsh_prompt = %s\n", wsh_prompt);

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, fd)) != -1) {

        printf("skeleton=");
        skeleton(line); 
        printf("prompt=");
        prompt(line);

        strip(&line, &read);

        if (read == 0) // ignore blank input lines
            continue;
        
        if (line[0] == '#') // ignore comments
            continue;

        printf("skeleton after stripping=");
        skeleton(line);
        printf("prompt after stripping=");
        prompt(line);

        printf("tokens:\n");
        char* token = strtok(line, " ");
        while (token != NULL) {
            printf("%s, ", token);
            token = strtok(NULL, " ");
        }
        printf("\n");
    }
    printf("exiting...\n");

    return 0;
}