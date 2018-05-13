#if !defined(TABLE_H)
# define TABLE_H
# include <stddef.h>

typedef struct table_s table;

/*
 * Key-value pairs are stored in the table as an array of blocks:
 *
 * struct
 * {
 *     uint64_t hash;
 *     char key[keywidth];
 *     char value[valwidth];
 * }
 *
 */
struct table_s
{
    size_t capacity; /* The number of blocks memory is allocated for */
    size_t usage;    /* The number of blocks used                    */
    size_t valwidth; /* The sizeof the values being stored           */
    size_t keywidth; /* The sizeof the keys being store              */
    size_t blkwidth; /* The total size of a block                    */
    char  *data;     /* The array of blocks                          */
};

/* Initialize a table */
void table_init(table *t, size_t valwidth, size_t keywidth);

/* Kill a table */
void table_kill(table *t);

/* Get the number of elements in a table */
size_t table_len(table *t);

/* Associate a value with a key in the table */
void *table_set(table *t, const void *k, const void *value);

/* Find the value associated with a key in the table */
void *table_get(table *t, const void *k);

/* Remove a key from the table */
void table_delete(table *t, const void *k);

#endif
