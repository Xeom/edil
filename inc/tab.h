#if !defined(TABLE_H)
# define TABLE_H
# include <stddef.h>

typedef struct tab_s tab;

/*
 * Key-value pairs are stored in the tab as an array of blocks:
 *
 * struct
 * {
 *     uint64_t hash;
 *     char key[keywidth];
 *     char value[valwidth];
 * }
 *
 */
struct tab_s
{
    size_t capacity; /* The number of blocks memory is allocated for */
    size_t usage;    /* The number of blocks used                    */
    size_t valwidth; /* The sizeof the values being stored           */
    size_t keywidth; /* The sizeof the keys being store              */
    size_t blkwidth; /* The total size of a block                    */
    char  *data;     /* The array of blocks                          */
};

/* Initialize a tab */
void tab_init(tab *t, size_t valwidth, size_t keywidth);

/* Kill a tab */
void tab_kill(tab *t);

/* Get the number of elements in a tab */
size_t tab_len(tab *t);

/* Associate a value with a key in the tab */
void *tab_set(tab *t, const void *k, const void *value);

/* Find the value associated with a key in the tab */
void *tab_get(tab *t, const void *k);

/* Remove a key from the tab */
void tab_del(tab *t, const void *k);

#endif
