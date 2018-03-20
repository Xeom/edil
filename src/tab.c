#include "tab.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define TAB_MIN_CAP 16

#define TAB_HASH_RANDOM_SEED 0xfc64c9bd39ea2141

#define TAB_MAX_USAGE(cap) ((cap >> 2) + (cap >> 3))
#define TAB_MIN_USAGE(cap) ((cap >> 3) + (cap >> 6))

static inline uint64_t *tab_blk_hash       (tab *t, size_t ind);
static inline char     *tab_blk_key        (tab *t, size_t ind);
static inline char     *tab_blk_val        (tab *t, size_t ind);
static inline void      tab_blk_setnull    (tab *t, size_t ind);
static inline int       tab_blk_isnull     (tab *t, size_t ind);
static        void      tab_relocate       (tab *t, size_t ind);
static        void      tab_realloc        (tab *t, size_t newcap);
static        void      tab_resize_bigger  (tab *t);
static        void      tab_resize_smaller (tab *t);
static        size_t    tab_find_blk       (tab *t, const void *k, int *new);
static        uint64_t  tab_hash           (const char *mem, size_t n);

void tab_init(tab *t, size_t valwidth, size_t keywidth)
{
    t->valwidth = valwidth;
    t->keywidth = keywidth;
    t->blkwidth = valwidth + keywidth + sizeof(uint64_t);

    t->usage    = 0;
    t->capacity = 0;

    t->data     = NULL;

    tab_realloc(t, TAB_MIN_CAP);
}

void tab_kill(tab *t)
{
    free(t->data);
}

size_t tab_len(tab *t)
{
    return t->usage;
}

static inline uint64_t *tab_blk_hash(tab *t, size_t ind)
{
    return (uint64_t *)(t->data + t->blkwidth * ind);
}

static inline char *tab_blk_key(tab *t, size_t ind)
{
    return t->data + t->blkwidth * ind + sizeof(uint64_t);
}

static inline char *tab_blk_val(tab *t, size_t ind)
{
    return t->data + t->blkwidth * ind + t->keywidth + sizeof(uint64_t);
}

static inline void tab_blk_setnull(tab *t, size_t ind)
{
    *tab_blk_hash(t, ind) = 0;
}

static inline int  tab_blk_isnull(tab *t, size_t ind)
{
    return *tab_blk_hash(t, ind) == 0;
}

static void tab_relocate(tab *t, size_t ind)
{
    size_t newind;

    if (tab_blk_isnull(t, ind)) return;

    newind = tab_find_blk(t, tab_blk_key(t, ind), NULL);

    if (newind == ind) return;

    memcpy(tab_blk_hash(t, newind), tab_blk_hash(t, ind), t->blkwidth);
    tab_blk_setnull(t, ind);
}

static void tab_realloc(tab *t, size_t newcap)
{
    size_t prevcap, ind;

    prevcap     = t->capacity;
    t->capacity = newcap;

    if (newcap > prevcap)
    {
        t->data = realloc(t->data, t->blkwidth * newcap);

        for (ind = prevcap; ind < newcap; ++ind)
            tab_blk_setnull(t, ind);
    }

    for (ind = 0; ind < prevcap; ++ind)
        tab_relocate(t, ind);
    for (ind = 0; ind < newcap; ++ind)
        tab_relocate(t, ind);

    if (prevcap > newcap)
        t->data = realloc(t->data, t->blkwidth * newcap);
}

static void tab_resize_bigger(tab *t)
{
    if (t->usage <= TAB_MAX_USAGE(t->capacity))
        return;

    tab_realloc(t, t->capacity << 1);
}

static void tab_resize_smaller(tab *t)
{
    if (t->usage > TAB_MIN_USAGE(t->capacity) ||
        t->capacity >> 1 < TAB_MIN_CAP)
        return;

    tab_realloc(t, t->capacity >> 1);
}

static size_t tab_find_blk(tab *t, const void *k, int *new)
{
    size_t ind;
    uint64_t hash;

    hash = tab_hash(k, t->keywidth);
    ind = hash % t->capacity;

    if (new) *new = 0;

    for (;;)
    {
        if (tab_blk_isnull(t, ind))
            break;

        if (*tab_blk_hash(t, ind) == hash &&
            memcmp(tab_blk_key(t, ind), k, t->keywidth) == 0)
            return ind;

        ind++;
        if (ind >= t->capacity)
            ind = 0;
    }

    if (new) *new = 1;

    return ind;
}

void *tab_set(tab *t, const void *k, const void *value)
{
    int    new;
    size_t ind;

    ind = tab_find_blk(t, k, &new);

    if (new)
        memcpy(tab_blk_key(t, ind), k,     t->keywidth);

    if (value)
        memcpy(tab_blk_val(t, ind), value, t->valwidth);
    else
        memset(tab_blk_val(t, ind), 0,     t->valwidth);

    *tab_blk_hash(t, ind) = tab_hash(k, t->keywidth);

    if (new) ++(t->usage);

    tab_resize_bigger(t);

    return tab_blk_val(t, ind);
}

void *tab_get(tab *t, const void *k)
{
    int    new;
    size_t ind;

    ind = tab_find_blk(t, k, &new);

    if (new) return NULL;

    return tab_blk_val(t, ind);
}

void tab_del(tab *t, const void *k)
{
    int    new;
    size_t ind;

    ind = tab_find_blk(t, k, &new);

    if (new) return;

    tab_blk_setnull(t, ind);

    /* This is very inefficient, but whatever */
    for (;;)
    {
        ind++;
        if (ind >= t->capacity)
            ind = 0;

        if (tab_blk_isnull(t, ind))
            break;

        tab_relocate(t, ind);
    }

    (t->usage)--;
    tab_resize_smaller(t);
}

static uint64_t tab_hash(const char *mem, size_t n)
{
    int      loop;
    uint64_t hash;

    hash  = TAB_HASH_RANDOM_SEED;
    loop = 1;

    while (loop)
    {
        uint32_t i;
        double   recp, den;
        void    *fptr;

        if (n >= 4)
        {
            i    = *(uint32_t *)mem;

            mem += 4;
            n   -= 4;
        }
        else
        {
            while (n)
            {
                 i <<= 8;
                 i &= mem[n--];
            }
            loop = 0;
        }

        den  = ((double)i * 2.0) - 1;

        recp = 1 / (double)den;
        fptr = &recp;

        hash ^= ((1l << 52) - 1) & *(long long *)fptr;
        hash ^= (hash << 26);
        hash ^= i;
    }

    if (hash == 0) return TAB_HASH_RANDOM_SEED;

    return hash;
}
