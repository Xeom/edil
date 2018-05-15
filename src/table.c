#include "table.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_MIN_CAP 16

#define TABLE_HASH_RANDOM_SEED 0xfc64c9bd39ea2141

#define TABLE_MAX_USAGE(cap) ((cap >> 2) + (cap >> 3))
#define TABLE_MIN_USAGE(cap) ((cap >> 3) + (cap >> 6))

static inline size_t    table_blk_ind        (table *t, void *blk);
static inline uint64_t *table_blk_hash       (table *t, size_t ind);
static inline char     *table_blk_key        (table *t, size_t ind);
static inline char     *table_blk_val        (table *t, size_t ind);
static inline void      table_blk_setnull    (table *t, size_t ind);
static inline int       table_blk_isnull     (table *t, size_t ind);
static        void      table_relocate       (table *t, size_t ind);
static        void      table_realloc        (table *t, size_t newcap);
static        void      table_resize_bigger  (table *t);
static        void      table_resize_smaller (table *t);
static        size_t    table_find_blk       (table *t, const void *k, int *new);
static        uint64_t  table_hash           (const char *mem, size_t n);

void table_init(table *t, size_t valwidth, size_t keywidth)
{
    t->valwidth = valwidth;
    t->keywidth = keywidth;
    t->blkwidth = valwidth + keywidth + sizeof(uint64_t);

    t->usage    = 0;
    t->capacity = 0;

    t->data     = NULL;

    table_realloc(t, TABLE_MIN_CAP);
}

void table_kill(table *t)
{
    free(t->data);
}

size_t table_len(table *t)
{
    return t->usage;
}

static inline size_t table_blk_ind(table *t, void *blk)
{
    size_t diff = (char *)blk - t->data;
    return diff / t->blkwidth;
}

static inline uint64_t *table_blk_hash(table *t, size_t ind)
{
    return (uint64_t *)(t->data + t->blkwidth * ind);
}

static inline char *table_blk_key(table *t, size_t ind)
{
    return t->data + t->blkwidth * ind + sizeof(uint64_t);
}

static inline char *table_blk_val(table *t, size_t ind)
{
    return t->data + t->blkwidth * ind + t->keywidth + sizeof(uint64_t);
}

static inline void table_blk_setnull(table *t, size_t ind)
{
    *table_blk_hash(t, ind) = 0;
}

static inline int  table_blk_isnull(table *t, size_t ind)
{
    return *table_blk_hash(t, ind) == 0;
}

static void table_relocate(table *t, size_t ind)
{
    size_t newind;

    if (table_blk_isnull(t, ind)) return;

    newind = table_find_blk(t, table_blk_key(t, ind), NULL);

    if (newind == ind) return;

    memcpy(table_blk_hash(t, newind), table_blk_hash(t, ind), t->blkwidth);
    table_blk_setnull(t, ind);
}

static void table_realloc(table *t, size_t newcap)
{
    size_t prevcap, ind;

    prevcap     = t->capacity;
    t->capacity = newcap;

    if (newcap > prevcap)
    {
        t->data = realloc(t->data, t->blkwidth * newcap);

        for (ind = prevcap; ind < newcap; ++ind)
            table_blk_setnull(t, ind);
    }

    for (ind = 0; ind < prevcap; ++ind)
        table_relocate(t, ind);
    for (ind = 0; ind < newcap; ++ind)
        table_relocate(t, ind);

    if (prevcap > newcap)
        t->data = realloc(t->data, t->blkwidth * newcap);
}

static void table_resize_bigger(table *t)
{
    if (t->usage <= TABLE_MAX_USAGE(t->capacity))
        return;

    table_realloc(t, t->capacity << 1);
}

static void table_resize_smaller(table *t)
{
    if (t->usage > TABLE_MIN_USAGE(t->capacity) ||
        t->capacity >> 1 < TABLE_MIN_CAP)
        return;

    table_realloc(t, t->capacity >> 1);
}

void *table_next(table *t, void *val, void **key)
{
    size_t ind;

    if (!val) ind = 0;
    else      ind = table_blk_ind(t, val) + 1;

    if (key) *key = NULL;

    for (;;)
    {

        if (!table_blk_isnull(t, ind))
        {
            if (key) *key = table_blk_key(t, ind);

            return table_blk_val(t, ind);
        }

        ind++;

        if (ind >= t->capacity)
            return NULL;
    }
}

static size_t table_find_blk(table *t, const void *k, int *new)
{
    size_t ind;
    uint64_t hash;

    hash = table_hash(k, t->keywidth);
    ind = hash % t->capacity;

    if (new) *new = 0;

    for (;;)
    {
        if (table_blk_isnull(t, ind))
            break;

        if (*table_blk_hash(t, ind) == hash
            && memcmp(table_blk_key(t, ind), k, t->keywidth) == 0)
            return ind;

        ind++;
        if (ind >= t->capacity)
            ind = 0;
    }

    if (new) *new = 1;

    return ind;
}

void *table_set(table *t, const void *k, const void *value)
{
    int    new;
    size_t ind;

    ind = table_find_blk(t, k, &new);

    if (new)   memcpy(table_blk_key(t, ind), k,     t->keywidth);

    if (value) memcpy(table_blk_val(t, ind), value, t->valwidth);
    else       memset(table_blk_val(t, ind), 0,     t->valwidth);

    *table_blk_hash(t, ind) = table_hash(k, t->keywidth);

    if (new) ++(t->usage);

    table_resize_bigger(t);

    return table_blk_val(t, ind);
}

void *table_get(table *t, const void *k)
{
    int    new;
    size_t ind;

    ind = table_find_blk(t, k, &new);

    if (new) return NULL;

    return table_blk_val(t, ind);
}

void table_delete(table *t, const void *k)
{
    int    new;
    size_t ind;

    ind = table_find_blk(t, k, &new);

    if (new) return;

    table_blk_setnull(t, ind);

    for (;;)
    {
        ind++;
        if (ind >= t->capacity)
            ind = 0;

        if (table_blk_isnull(t, ind))
            break;

        table_relocate(t, ind);
    }

    (t->usage)--;
    table_resize_smaller(t);
}

static uint64_t table_hash(const char *mem, size_t n)
{
    int      loop;
    uint64_t hash;

    hash  = TABLE_HASH_RANDOM_SEED;
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

    if (hash == 0) return TABLE_HASH_RANDOM_SEED;

    return hash;
}
