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

int main() {
    char* xyz = getenv("xyz");
    printf("getenv(xyz)=%s\n", xyz);
    
    char** start = environ;
    while (*start != NULL) {
        printf("%s\n", *start);
        start++;
    }

    putenv("xyz=areeb");
    xyz = getenv("xyz");
    printf("getenv(xyz)=%s\n", xyz);
    
    start = environ;
    while (*start != NULL) {
        printf("%s\n", *start);
        start++;
    }

    unsetenv("xyz");
    xyz = getenv("xyz");
    printf("getenv(xyz)=%s\n", xyz);
    
    start = environ;
    while (*start != NULL) {
        printf("%s\n", *start);
        start++;
    }

    return 0;
}