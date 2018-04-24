#include <stdio.h>
#include <string.h>

#include "vec.h"

/* Call after operations that might shorten a vec */
static void vec_resize_shorter(vec *v)
{
    /* Escape if no changes need making */
    if (v->usage >= (v->capacity>> 2)) return;

    /* Decrease the capacity until it's okay */
    do
    {
        v->capacity >>= 1;
    } while (v->usage < (v->capacity >> 2));

    v->data = realloc(v->data, v->capacity);
}

/* Call after operations that might lengthen a vec */
static void vec_resize_longer(vec *v)
{
    /* Escape if no changes need making */
    if (v->usage <= v->capacity) return;

    if (v->capacity == 0) v->capacity = 1;
    /* Increase the capacity until the data fits */
    do
    {
 //  printf("%lu %lu\n", v->usage, v->capacity);
        v->capacity <<= 1;
    } while (v->usage > v->capacity);

    v->data = realloc(v->data, v->capacity);
}

/* Initialises a vector */
void vec_init(vec *v, size_t width)
{
    v->width    = width;
    v->usage    = 0;
    v->capacity = 0;
    v->data     = NULL;
}

/* Kill a vector */
void vec_kill(vec *v)
{
    if (v->data) free(v->data);
    v->data     = NULL;
    v->usage    = 0;
    v->capacity = 0;
}

/* Use qsort on a vector */
void vec_sort(vec *v, int (*cmpfunc)(const void *a, const void *b))
{
    if (!v || !cmpfunc) return;
    if (v->usage == 0)  return;

    qsort(v->data, vec_len(v), v->width, cmpfunc);
}

/* Reverse a vector */
void vec_rev(vec *v)
{
    size_t lind, len, mid;

    len = vec_len(v);
    mid = len / 2;

    for (lind = 0; lind < mid; lind++)
    {
        char swap[v->width];
        size_t rind;
        rind = len - lind - 1;

        memcpy(swap,             vec_get(v, lind), v->width);
        memcpy(vec_get(v, lind), vec_get(v, rind), v->width);
        memcpy(vec_get(v, rind), swap,             v->width);
    }
}

/* Repeatedly insert something in a vector */
void *vec_rep(vec *v, size_t ind, size_t n, const void *data, size_t reps)
{
    /* The bytes after the point of insertion, the offset into the data   *
     * of the point of insertion, and the number of bytes being inserted, *
     * and the number of bytes per single insertion.                      */
    size_t bytesafter, offset, bytesins, bytessing;

    if (!v) return NULL;
    if (n == 0) return NULL;

    offset     = ind * v->width;
    bytessing  = n   * v->width;
    bytesins   = bytessing * reps;
    bytesafter = v->usage - offset;

    /* Don't accept inserts beyond the end of the data */
    if (offset > v->usage) return NULL;

    /* Increment and resize before putting in the data */
    v->usage  += bytesins;
    vec_resize_longer(v);

    /* Shift the bytes after the point of insertion forward */
    if (bytesafter)
        memmove(v->data + offset + bytesins, v->data + offset, bytesafter);

    /* Fill the gap, either with data or to zero */
    if (data)
    {
        while (reps--)
        {
            memmove(v->data + offset, data, bytessing);
            offset += bytessing;
        }
    }
    else memset(v->data + offset, 0, bytesins);

    return v->data + offset;
}

/* BISECTION */
size_t vec_bst(
    vec *v,
    const void *item,
    int (*cmpfunc)(const void *a, const void *b)
)
{
    size_t ltind, gtind;
    int cmp;
    puts((char *)item);

    if (!v || !cmpfunc) return 0;
    /* An empty vector returns 0 always */
    if (v->usage == 0) return 0;

    /* ltind and gtind will always be less than and greater than item, *
     * respectively. They are set to the ends of the vec here          */
    ltind = 0;
    gtind = vec_len(v) - 1;

    /* Check that ltind and gtind are less than and greater than item, *
     * otherwise return a limit.                                       */
    if (cmpfunc(vec_get(v, ltind), item) > 0) return 0;
    if (cmpfunc(vec_get(v, gtind), item) < 0) return vec_len(v);

    /* We're done when we've narrowed ltind and gtind down to one apart. *
     * Our new bisection index is between them.                          */
    while (ltind + 1 < gtind)
    {
        size_t midind;

        midind = (gtind + ltind) / 2;
        cmp = cmpfunc(vec_get(v, midind), item);

        if (cmp  > 0) gtind = midind;
        if (cmp  < 0) ltind = midind;
        if (cmp == 0) return  midind;
    }

    cmp = cmpfunc(vec_get(v, ltind), item);

    if (cmp == 0) return ltind;
    else          return gtind;
}

/* Get the number of items in a vector */
size_t vec_len(vec *v)
{
    if (!v) return 0;
    return v->usage / v->width;
}

/* Insert data into a vector */
void *vec_ins(vec *v, size_t ind, size_t n, const void *data)
{
    return vec_rep(v, ind, n, data, 1);
}

void *vec_app(vec *v, const void *data)
{
    size_t ind;
    ind = vec_len(v);

    return vec_rep(v, ind, 1, data, 1);
}

void *vec_str(vec *v, const char *str)
{
    size_t num, ind;
    num = strlen(str);
    ind = vec_len(v);

    return vec_rep(v, ind, num, str, 1);
}

void *vec_cpy(vec *v, vec *other)
{
    size_t num, ind;

    num = vec_len(other);
    ind = vec_len(v);

    if (num)
        return vec_rep(v, ind, num, other->data, 1);
    else
        return NULL;
}

int vec_clr(vec *v)
{
    return vec_del(v, 0, vec_len(v));
}

int vec_del(vec *v, size_t ind, size_t n)
{
    size_t bytesafter, bytesdead, offset;

    if (n == 0) return 0;

    offset     = ind * v->width;
    bytesdead  = n   * v->width;
    bytesafter = v->usage - offset - bytesdead;

    if (offset + bytesdead > v->usage) return -1;

    if (bytesafter)
        memmove(v->data + offset, v->data + offset + bytesdead, bytesafter);

    v->usage -= bytesdead;
    vec_resize_shorter(v);

    return 0;
}

void *vec_get(vec *v, size_t ind)
{
    size_t offset;

    if (!v) return NULL;

    offset = ind * v->width;

    if (offset >= v->usage) return NULL;

    return v->data + offset;
}
