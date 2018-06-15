#if !defined(RING_H)
# define RING_H

#include "container/vec.h"
#include "buf/buf.h"

extern vec ring_bufs;

#define RING_FOREACH(_b, ...) \
    VEC_FOREACH(&ring_bufs, buf **, _ ## _b, \
        buf *_b = * _ ## _b;                 \
        do { __VA_ARGS__ } while (0);        \
    )

void ring_init(void);
void ring_kill(void);

int ring_get_ind(buf *b);
buf *ring_get(int ind);

buf *ring_next(buf *b);
buf *ring_prev(buf *b);

buf *ring_new(void);

buf *ring_del(buf *b);

#endif
