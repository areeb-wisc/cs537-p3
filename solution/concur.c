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

void print100(char* message) {
    for (int i = 1; i <= 1000; i++)
        printf("%s | %d\n", message, i);
    fflush(stdout);
}

void trial1(char* file_name) {
    FILE* f = fopen(file_name, "w");
    fflush(stdout);
    dup2(fileno(f), STDOUT_FILENO);
    int pid = fork();
    if (pid < 0)
        printf("failed\n");
    else if (pid == 0)
        print100("trail1 | child");
    else
        print100("trial1 | parent");    
}

void trial2(char* file_name) {
    int pid = fork();
    if (pid < 0)
        printf("failed\n");
    else if (pid == 0) {
        FILE* f = fopen(file_name, "w");
        fflush(stdout);
        dup2(fileno(f), STDOUT_FILENO);
        print100("trial2 | child");
    }
    else {
        FILE* f = fopen(file_name, "w");
        fflush(stdout);
        dup2(fileno(f), STDOUT_FILENO);
        print100("trial2 | parent");
    }
}

int main() {

    int pid = fork();

    if (pid < 0)
        printf("error\n");
    else if(pid == 0) {
        printf("In CHILD\n");
        printf("My PID = %d\n", getpid());
        printf("Parent PID = %d\n", getppid());
        exit(0);
    } else {
        // sleep(10);
        wait(0);
        printf("In PARENT\n");
        printf("My PID = %d\n", getpid());
        printf("Child PID = %d\n", pid);
    }

    printf("Common message\n");

    // printf("hello\n");
    // trial1("output1.txt");
    // trial2("output2.txt");
    return 0;
}