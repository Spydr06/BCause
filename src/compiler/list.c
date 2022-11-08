#include "list.h"

#include <stdlib.h>

void list_push(struct list *list, void *item)
{
    if(!list->alloc)
        list->data = malloc((list->alloc = 32));
    else if(list->alloc - list->size < 1)
        list->data = realloc(list->data, list->alloc *= 2);
    list->data[list->size++] = item;
}

void list_free(struct list *list)
{
    if(list->alloc)
        free(list->data);
}
