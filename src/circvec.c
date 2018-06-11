#include "circvec.h"

void circvec_init(circvec *cv, size_t width, size_t size)
{
    vec_init(&(cv->v), width);
    vec_ins(&(cv->v), 0, size, NULL);

    cv->ins = 0;
    cv->del = 0;
    cv->empty = 1;
}

void circvec_kill(circvec *cv)
{
    vec_kill(&(cv->v));
}

int circvec_full(circvec *cv)
{
    return !(cv->empty) && cv->ins == cv->del;
}

int circvec_empty(circvec *cv)
{
    return cv->empty;
}

size_t circvec_get_free(circvec *cv)
{
    vec *v;
    v = &(cv->v);

    if (circvec_empty(cv))
        return vec_len(v);

    if (cv->ins <= cv->del)
        return cv->del - cv->ins;

    else
        return vec_len(v) - (cv->ins - cv->del);
}

size_t circvec_get_used(circvec *cv)
{
    return vec_len(&(cv->v)) - circvec_get_free(cv);
}

void *circvec_pop(circvec *cv)
{
    vec *v;
    void *rtn;
    v = &(cv->v);

    if (circvec_empty(cv))
        return NULL;

    rtn = vec_get(v, cv->del);

    cv->del += 1;

    if (cv->del == vec_len(v))
        cv->del = 0;

    if (cv->del == cv->ins)
        cv->empty = 1;

    return rtn;
}

void *circvec_peek(circvec *cv)
{
    vec *v;
    v = &(cv->v);

    if (circvec_empty(cv))
        return NULL;

    return vec_get(v, cv->del);
}

void *circvec_push(circvec *cv)
{
    vec *v;
    void *ptr;
    v = &(cv->v);

    if (circvec_full(cv))
        circvec_pop(cv);

    ptr = vec_get(v, cv->ins);

    cv->ins += 1;

    if (cv->ins == vec_len(v))
        cv->ins = 0;

    cv->empty = 0;

    return ptr;
}

void *circvec_get(circvec *cv, ssize_t ind)
{
    size_t len;
    vec *v;
    v = &(cv->v);

    if (ind >= 0)
        ind += cv->del;
    else
        ind += cv->ins;

    len = vec_len(v);

    if (ind > (ssize_t)len) ind -= len;
    if (ind < 0)            ind += len;

    return vec_get(v, ind);
}