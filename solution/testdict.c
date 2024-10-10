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

#include "dict.h"

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