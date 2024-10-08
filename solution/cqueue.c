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

void init(cqueue* cq, int size) {
    printf("init\n");
    cq->n = size;
    cq->r = -1;
    cq->f = -1;
    cq->words = (char**)malloc(size * sizeof(char*));
}

void push(cqueue* cq, const char* word) {
    printf("push %s\n", word);
    if (cq->r == -1 || cq->r == (cq->f + 1) % cq->n)
        cq->r = (cq->r + 1) % cq->n;
    cq->f = (cq->f + 1) % cq->n;
    cq->words[cq->f] = clone_str(word);
}

void pop(cqueue* cq) {
    printf("pop\n");
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

void print(cqueue* cq, int no) {
    char* word = get(cq, no);
    if (word == NULL)
        printf("NULL\n");
    else
        printf("%s\n", word);
}

void print_strings(cqueue* cq, char* delim, char* message) {
    printf("%s", message);
    int i = cq->r, size = getsize(cq);
    while (size--) {
        printf("%s%s", cq->words[i], delim);
        i = (i + 1) % cq->n;
    }
    printf("\n");
}

void display(cqueue* cq) {
    printf("n = %d, r = %d, f = %d\n", cq->n, cq->r, cq->f);
    print_strings(cq, ",", "words = ");
}

void resize(cqueue** cq, int size) {

    printf("resize to %d\n", size);
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

int main() {

    cqueue* history = (cqueue*)malloc(sizeof(cqueue));
    init(history, 5);
    display(history);

    push(history, "1");
    push(history, "2");
    push(history, "3");
    push(history, "4");
    push(history, "5");
    push(history, "6");
    pop(history);
    pop(history);
    display(history);
    print(history, 1);
    print(history, 2);
    print(history, 3);
    print(history, 4);
    print(history, 5);
    print(history, 6);

    push(history, "7");
    display(history);
    print(history, 1);
    print(history, 2);
    print(history, 3);
    print(history, 4);
    print(history, 5);
    print(history, 6);

    push(history, "8");
    display(history);
    print(history, 1);
    print(history, 2);
    print(history, 3);
    print(history, 4);
    print(history, 5);
    print(history, 6);

    resize(&history, 3);
    display(history);

    push(history, "9");
    display(history);
    print(history, 1);
    print(history, 2);
    print(history, 3);
    print(history, 4);

    resize(&history, 5);
    display(history);

    push(history, "10");
    push(history, "11");
    push(history, "12");
    display(history);
    print(history, 1);
    print(history, 2);
    print(history, 3);
    print(history, 4);
    print(history, 5);
    print(history, 6);


    return 0;
}