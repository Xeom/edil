#if !defined(VEC_H)
# define VEC_H
# include "types.h"

#define VEC_FOREACH(_vec, _type, _i, _code) \
    do {                                    \
        size_t _ind, _len;                  \
        _len = vec_len(_vec);               \
        for (_ind = 0; _ind < _len; ++_ind) \
        {                                   \
            _type _i;                       \
            _i = vec_get(_vec, _ind);       \
            do { _code; } while (0);        \
        }                                   \
    } while (0);

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
