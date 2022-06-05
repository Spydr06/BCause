#include <list.h>

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#ifndef LIST_INIT_SIZE
    #define LIST_INIT_SIZE 32
#endif

#ifndef LIST_MULTIPLIER
    #define LIST_MULTIPLIER 2
#endif

void init_list_sized(List_T* list, size_t size)
{
    memset(list, 0, sizeof(List_T));
    list->size = 0;
    list->allocated = size;
    list->items = calloc(list->allocated, sizeof(void*));
}

void init_list(List_T* list)
{
    return init_list_sized(list, LIST_INIT_SIZE);
}

void list_push(List_T* list, void* item)
{
    list->size++;
    if(!list_has_space(list))
        list_grow(list, list->allocated ? list->allocated * LIST_MULTIPLIER : LIST_INIT_SIZE);
    list->items[list->size - 1] = item;
}

bool list_has_space(List_T* list)
{
    return list->size < list->allocated;
}

void list_grow(List_T* list, size_t to)
{
    if(to <= list->allocated)
        return;
    
    list->items = realloc(list->items, (
        list->allocated = to
    ) * sizeof(void*));
}

void* list_pop(List_T* list)
{
    return list->size ? list->items[--list->size] : NULL;
}

void free_list(List_T* list)
{
    free(list->items);
    free(list);
}

void list_clear(List_T* list)
{
    for(size_t i = 0; i < list->size; i++)
        list->items[i] = NULL;
    list->size = 0;
}

size_t list_contains(List_T* list, void* item)
{
    for(size_t i = 0; i < list->size; i++)
        if(list->items[i] == item)
            return i + 1;
    return 0;
}

void list_foreach(List_T* list, void (*func)(void*))
{
    for(size_t i = 0; i < list->size; i++)
        func(list->items[i]);
}

void list_foreach_s(List_T* list, void (*func)(size_t, void*))
{
    for(size_t i = 0; i < list->size; i++)
        func(i, list->items[i]);
}