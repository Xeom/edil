#if !defined(TABLE_H)
# define TABLE_H
# include "types.h"

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

void *table_next(table *t, void *val, void **key);
#endif
