#include "ring.h"

vec ring_bufs;

void ring_init(void)
{
    vec_init(&ring_bufs, sizeof(buf));
}

void ring_kill(void)
{
    size_t ind, len;

    len = vec_len(&ring_bufs);
    for (ind = 0; ind < len; ind++)
    {
        buf_kill(vec_get(&ring_bufs, ind));
    }

    vec_kill(&ring_bufs);
}

int ring_get_ind(buf *b)
{
    int ind;
    buf *first;
    first = vec_get(&ring_bufs, 0);

    if (first == NULL) return -1;

    ind = ((char *)b - (char *)first)/sizeof(buf);

    if (ind < 0 || ind >= (int)vec_len(&ring_bufs))
        return -1;

    return ind;
}

buf *ring_next(buf *b)
{
    int ind;
    ind = ring_get_ind(b);

    if (ind == -1)
        ind  = 0;
    else
    {
        ind += 1;
        ind %= vec_len(&ring_bufs);
    }

    return vec_get(&ring_bufs, ind);
}

buf *ring_prev(buf *b)
{
    int ind;
    ind = ring_get_ind(b);

    if (ind == -1)
        ind  = 0;
    else
    {
        if (ind == 0)
            ind = vec_len(&ring_bufs);

        ind -= 1;
    }

    return vec_get(&ring_bufs, ind);
}

buf *ring_new(void)
{
    vec msg;
    buf *b;

    vec_init(&msg, sizeof(chr));
    chr_format(&msg, "This is buffer number %ld.", vec_len(&ring_bufs));

    b = vec_ins(&ring_bufs, vec_len(&ring_bufs), 1, NULL);
    buf_init(b);

    buf_ins(b, (cur){0, 0}, vec_get(&msg, 0), vec_len(&msg));
    vec_kill(&msg);

    return b;
}
