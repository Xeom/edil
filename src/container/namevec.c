#include <string.h>

#include "text/chr.h"
#include "container/vec.h"

#include "container/namevec.h"

static int namevec_cmp(const void *aptr, const void *bptr);

void namevec_init(vec *v, namevec_item *items, size_t bytes)
{
    size_t n;
    n = bytes / sizeof(namevec_item);

    vec_init(v, sizeof(namevec_item));
    vec_ins(v, 0, n, items);
    vec_sort(v, namevec_cmp);
}

void namevec_sort(vec *v)
{
    vec_sort(v, namevec_cmp);
}

static int namevec_cmp(const void *aptr, const void *bptr)
{
    const namevec_item *a, *b;
    a = aptr;
    b = bptr;

    return strcmp(a->name, b->name);
}

int namevec_startswith(namevec_item *item, char *str)
{
    size_t len;

    if (!item) return 0;

    len = strlen(str);

    if (strncmp(str, item->name, len) == 0)
        return 1;

    else
        return 0;
}

namevec_item *namevec_get_str(vec *v, char *str, size_t *n)
{
    namevec_item *rtn, *next;
    size_t ind;
    ind = vec_bst(v, &(namevec_item){ .name = str }, namevec_cmp);

    rtn = vec_get(v, ind);

    if (!rtn) return NULL;

    if (strcmp(str, rtn->name) == 0)
    {
        *n = 1;
        return rtn;
    }

    next = rtn;

    if (n) *n = 0;

    while (next && namevec_startswith(next, str))
    {
        if (n)
            *n += 1;
        else
            return rtn;

        next = vec_get(v, ++ind);
    }

    if (n)
        return rtn;
    else
        return NULL;
}

namevec_item *namevec_get_chrs(vec *v, vec *chrs, size_t *n)
{
    vec str;
    namevec_item *rtn;

    vec_init(&str, sizeof(char));
    chr_to_str(chrs, &str);
    vec_ins(&str, vec_len(&str), 1, NULL);

    rtn = namevec_get_str(v, vec_first(&str), n);

    vec_kill(&str);

    return rtn;
}
