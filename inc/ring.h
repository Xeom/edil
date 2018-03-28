#if !defined(RING_H)
# define RING_H

#include "vec.h"
#include "buf.h"

extern vec ring_bufs;

void ring_init(void);
void ring_kill(void);

int ring_get_ind(buf *b);

buf *ring_next(buf *b);
buf *ring_prev(buf *b);

buf *ring_new(void);

#endif
