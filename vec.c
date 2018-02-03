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
    /* The bytes after the point of insertion, the offset into the data  *
     * of the point of insertion, and the number of bytes being inserted */
    size_t bytesafter, offset, bytesins;

    if (!v) return NULL;

    offset     = ind * v->width;
    bytesins   = n * v->width;
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
        memmove(v->data + offset, data, bytesins);
    else memset(v->data + offset, 0,    bytesins);

    return v->data + offset;
}

int vec_del(vec *v, size_t ind, size_t n)
{
    size_t bytesafter, bytesdead, offset;

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
