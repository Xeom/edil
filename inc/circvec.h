#if !defined(CIRCVEC_H)
# define CIRCVEC_H
# include "vec.h"
# include <unistd.h>

typedef struct circvec_s circvec;

struct circvec_s
{
    vec v;
    size_t ins, del;
    unsigned int empty : 1;
};

void circvec_init(circvec *cv, size_t width, size_t size);

void circvec_kill(circvec *cv);

int circvec_full(circvec *cv);

int circvec_empty(circvec *cv);

size_t circvec_get_free(circvec *cv);

size_t circvec_get_used(circvec *cv);

void *circvec_pop(circvec *cv);

void *circvec_peek(circvec *cv);

void *circvec_push(circvec *cv);

void *circvec_get(circvec *cv, ssize_t ind);

#endif
