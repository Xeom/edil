#if !defined(VEC_H)
# define VEC_H

# include <stdlib.h>

typedef struct vec_s vec;

struct vec_s
{
    /* The stored data, can be NULL */
    char *data;
    /* Capacity usage and width in bytes */
    size_t capacity;
    size_t usage;
    size_t width;
};

void vec_init(vec *v, size_t width);

void vec_kill(vec *v);

void vec_sort(vec *v, int (*cmpfunc)(const void *a, const void *b));

void vec_rev(vec *v);

size_t vec_bst(
    vec *v,
    const void *item,
    int (*cmpfunc)(const void *a, const void *b)
);

size_t vec_len(vec *v);

void *vec_ins(vec *v, size_t ind, size_t n, const void *data);

void *vec_rep(vec *v, size_t ind, size_t n, const void *data, size_t reps);

int vec_del(vec *v, size_t ind, size_t n);

void *vec_get(vec *v, size_t ind);

#define vec_first(v) ((void *)((v)->data))

/* Append one item to the end of a vector */
void *vec_app(vec *v, const void *data);

/* Append the contents of another vector onto this one */
void *vec_cpy(vec *v, vec *other);

/* Delete everything in a vector */
int vec_clr(vec *v);

/* Append a string */
void *vec_str(vec *v, const char *str);
#endif /* VEC_H */
