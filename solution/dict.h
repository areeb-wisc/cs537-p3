#ifndef DICT_H
#define DICT_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct Entry {
    char* key;
    char* val;
} entry;
typedef struct Dict {
    int size;
    int max_size;
    entry** entries;

} dict;

char* clone_str(const char*);
int get_idx(dict*, const char*);
entry* make_entry(const char*, const char*);
void add_entry(dict*, const char*, const char*);
char* get_val(dict*, const char*);
void print_dict(dict*);

#endif