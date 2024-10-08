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

char* clone_str(const char* str) {
    char* clone = (char*)malloc((strlen(str) + 1) * sizeof(char));
    strcpy(clone, str);
    return clone;
}

typedef struct circular_queue {
    char** words;
    int n, r, f;
} cqueue;

cqueue* cq;

void init(int size) {
    printf("init\n");
    cq->n = size;
    cq->r = -1;
    cq->f = -1;
    cq->words = (char**)malloc(size * sizeof(char*));
}

void push(const char* word) {
    printf("push %s\n", word);
    if (cq->r == -1 || cq->r == (cq->f + 1) % cq->n)
        cq->r = (cq->r + 1) % cq->n;
    cq->f = (cq->f + 1) % cq->n;
    cq->words[cq->f] = clone_str(word);
}

void pop() {
    printf("pop\n");
    if (cq->f == cq->r) {
        cq->f = -1;
        cq->r = -1;
    } else
        cq->r = (cq->r + 1) % cq->n;
}

int getsize() {
    if (cq->f == -1)
        return 0;
    if (cq->f >= cq->r)
        return cq->f - cq->r + 1;
    return cq->f + 1 + cq->n - cq->r;
}

char* get(int no) {
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

void print(int no) {
    char* word = get(no);
    if (word == NULL)
        printf("NULL\n");
    else
        printf("%s\n", word);
}

void print_strings(char** array, int size, char* delim, char* message) {
    printf("%s", message);
    int i = cq->r;
    while (size--) {
        printf("%s%s", array[i], delim);
        i = (i + 1) % cq->n;
    }
    printf("\n");
}

void display() {
    printf("n = %d, r = %d, f = %d\n", cq->n, cq->r, cq->f);
    print_strings(cq->words, getsize(), ",", "words = ");
}

int main() {

    cq = (cqueue*)malloc(sizeof(cqueue));
    init(1);
    display();

    push("1");
    push("2");
    push("3");
    push("4");
    push("5");
    push("6");
    pop();
    pop();
    display();
    print(1);
    print(2);
    print(3);
    print(4);
    print(5);
    print(6);

    push("7");
    display();
    print(1);
    print(2);
    print(3);
    print(4);
    print(5);
    print(6);

    push("8");
    display();
    print(1);
    print(2);
    print(3);
    print(4);
    print(5);
    print(6);

    return 0;
}