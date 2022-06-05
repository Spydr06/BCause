#pragma once 

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct LIST_STRUCT
{
    size_t size;
    size_t allocated;
    void** items;
} List_T;

void init_list(List_T* list);
void init_list_sized(List_T* list, size_t size);
void free_list(List_T* list);

void list_foreach(List_T* list, void (*func)(void*));
void list_foreach_s(List_T* list, void (*func)(size_t, void*));

bool list_has_space(List_T* list);
void list_grow(List_T* list, size_t to);
void list_push(List_T* list, void* item);
void* list_pop(List_T* list);
void list_clear(List_T* list);
size_t list_contains(List_T* list, void* item);