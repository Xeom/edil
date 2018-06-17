#if !defined(CIRCVEC_H)
# define CIRCVEC_H
# include <unistd.h>
# include "types.h"

void circvec_init(circvec *cv, size_t width, size_t size);

size_t circvec_size(circvec *cv);

void circvec_kill(circvec *cv);

int circvec_full(circvec *cv);

int circvec_empty(circvec *cv);

size_t circvec_get_free(circvec *cv);

size_t circvec_get_used(circvec *cv);

void *circvec_pop(circvec *cv);

void *circvec_peek(circvec *cv);

void *circvec_push(circvec *cv);

void circvec_pipe_push(circvec *cv, void *data);

void *circvec_get(circvec *cv, ssize_t ind);

void circvec_resize(circvec *cv, size_t sz);

#endif
