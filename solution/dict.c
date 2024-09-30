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

typedef struct Entry {
    char* key;
    char* val;
} entry;

typedef struct Dict {
    int size;
    int max_size;
    entry** entries;

} dict;

char* clone_str(char* str) {
    char* clone = (char*)malloc((strlen(str) + 1) * sizeof(char));
    strcpy(clone, str);
    return(clone);
}

int get_idx(dict* my_dict, char* key) {
    for (int i = 0; i < my_dict->size; i++) {
        if (strcmp(my_dict->entries[i]->key, key) == 0)
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

void add_entry(dict* my_dict, char* key, char* val) {

    int idx = get_idx(my_dict, key);

    if (idx == -1) {
        printf("adding %s=%s\n", key, val);
        my_dict->size++;
        if (my_dict->size > my_dict->max_size) {
            my_dict->max_size *= 2;
            my_dict->entries = (entry**)realloc(my_dict->entries, my_dict->max_size * sizeof(entry*));
        }
        my_dict->entries[my_dict->size - 1] = make_entry(key, val);
    } else {
        printf("updating %s=%s\n", key, val);
        my_dict->entries[idx]->val = clone_str(val);
    }
}

char* get_val(dict* my_dict, char* key) {
    int idx = get_idx(my_dict, key);
    if (idx == -1)
        return NULL;
    return my_dict->entries[idx]->val;
}

void print_dict(dict* my_dict) {
    printf("size=%d, max_size=%d, dict=", my_dict->size, my_dict->max_size);
    printf("{");
    for (int i = 0; i < my_dict->size; i++) {
        entry* dict_entry = my_dict->entries[i];
        printf("%s:%s,", dict_entry->key, dict_entry->val);
    }
    printf("}\n");
}

int main() {
    // printf("Hello World\n");
    // dict* my_dict = (dict*)malloc(sizeof(dict));
    dict *my_dict, base_dict;
    my_dict = &base_dict;
    // printf("debug1\n");
    my_dict->size = 0;
    my_dict->max_size = 1;
    // printf("debug2\n");
    my_dict->entries = (entry**)malloc(my_dict->max_size*sizeof(entry*));
    // printf("debug3\n");
    print_dict(my_dict);
    add_entry(my_dict, "name", "areeb");
    print_dict(my_dict);
    add_entry(my_dict, "age", "27");
    print_dict(my_dict);
    printf("name=%s\n", get_val(my_dict, "name"));
    printf("age=%s\n", get_val(my_dict, "age"));
    printf("xyz=%s\n", get_val(my_dict, "xyz"));
    add_entry(my_dict, "university", "UW-Madison");
    print_dict(my_dict);
    add_entry(my_dict, "age", "24");
    printf("age=%s\n", get_val(my_dict, "age"));
    printf("xyz=%s\n", get_val(my_dict, "xyz"));
    add_entry(my_dict, "home", "india");
    add_entry(my_dict, "current", "US");
    print_dict(my_dict);
    return 0;
}