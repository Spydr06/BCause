#ifndef BCAUSE_LIST_H
#define BCAUSE_LIST_H

#include <stddef.h>

struct list {
    size_t alloc;
    size_t size;
    void** data;
};

void list_push(struct list *list, void *item);
void list_free(struct list *list);

#endif /* BCAUSE_LIST_H */
