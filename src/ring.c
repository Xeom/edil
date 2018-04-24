#include "ring.h"

vec ring_bufs;

void ring_init(void)
{
    vec_init(&ring_bufs, sizeof(buf *));
}

void ring_kill(void)
{
    size_t ind, len;

    len = vec_len(&ring_bufs);
    for (ind = 0; ind < len; ind++)
    {
        buf **b;

        b = vec_get(&ring_bufs, ind);
        buf_kill(*b);
        free(*b);
    }

    vec_kill(&ring_bufs);
}

int ring_get_ind(buf *b)
{
    size_t ind, len;

    len = vec_len(&ring_bufs);
    for (ind = 0; ind < len; ind++)
    {
        buf **cmp;
        cmp = vec_get(&ring_bufs, ind);

        if (*cmp == b)
            return ind;
    }

    return -1;
}

buf *ring_next(buf *b)
{
    int ind;
    buf **rtn;

    ind = ring_get_ind(b);

    if (ind == -1)
        ind  = 0;
    else
    {
        ind += 1;
        ind %= vec_len(&ring_bufs);
    }

    rtn = vec_get(&ring_bufs, ind);
    return *rtn;
}

buf *ring_prev(buf *b)
{
    int ind;
    buf **rtn;

    ind = ring_get_ind(b);

    if (ind == -1)
        ind  = 0;
    else
    {
        if (ind == 0)
            ind = vec_len(&ring_bufs);

        ind -= 1;
    }

    rtn = vec_get(&ring_bufs, ind);
    return *rtn;
}

buf *ring_new(void)
{
    vec msg;
    buf *b;

    b = malloc(sizeof(buf));
    buf_init(b);

    vec_init(&msg, sizeof(chr));
    chr_format(&msg, "This is buffer number %ld.", vec_len(&ring_bufs));
    buf_ins(b, (cur){0, 0}, vec_first(&msg), vec_len(&msg));
    vec_kill(&msg);

    vec_app(&ring_bufs, &b);

    return b;
}
