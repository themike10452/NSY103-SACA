#ifndef PAIR_H
#define PAIR_H

#include <malloc.h>

typedef struct pair_t pair_t;

struct pair_t
{
    void* first;
    void* second;
};

static inline pair_t* pair_create(void* first, void* second)
{
    pair_t* p = malloc(sizeof(pair_t));
    p->first = first;
    p->second = second;
    return p;
}

#endif
