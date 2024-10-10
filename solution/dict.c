#include "dict.h"

char* clone_str(const char* str) {
    char* clone = (char*)malloc((strlen(str) + 1) * sizeof(char));
    strcpy(clone, str);
    return(clone);
}

int get_idx(dict* my_dict, const char* key) {
    for (int i = 0; i < my_dict->size; i++) {
        if (strcmp(my_dict->entries[i]->key, key) == 0)
            return i;
    }
    return -1;
}

entry* make_entry(const char* key, const char* val) {
    entry* new_entry = (entry*)malloc(sizeof(entry));
    // printf("malloced\n");
    new_entry->key = clone_str(key);
    // printf("cloned key\n");
    new_entry->val = clone_str(val);
    // printf("cloned value\n");
    return new_entry;
}

void add_entry(dict* my_dict, const char* key, const char* val) {

    int idx = get_idx(my_dict, key);

    if (idx == -1) {
        // printf("adding %s=%s\n", key, val);
        my_dict->size++;
        if (my_dict->size > my_dict->max_size) {
            // printf("expanding\n");
            my_dict->max_size *= 2;
            my_dict->entries = (entry**)realloc(my_dict->entries, my_dict->max_size * sizeof(entry*));
        }
        // printf("calling make_entry\n");
        // printf("mydict->size=%d, mydict->maxsize=%d\n", my_dict->size, my_dict->max_size);
        my_dict->entries[my_dict->size - 1] = make_entry(key, val);
        // printf("added finally\n");
    } else {
        // printf("updating %s=%s\n", key, val);
        my_dict->entries[idx]->val = clone_str(val);
    }
    // printf("added %s=%s\n", key, val);
}

char* get_val(dict* my_dict, const char* key) {
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