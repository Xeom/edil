#if !defined(UTIL_H)
# define UTIL_H
# include <pthread.h>
# include "vec.h"

# define USING_MTX(mtx, code)          \
         do {                          \
         pthread_mutex_lock(&(mtx));   \
         do { code } while (0);        \
         pthread_mutex_unlock(&(mtx)); \
         } while (0);                   

void util_str2vec(char *str, vec *v);

typedef struct tqueue_s tqueue;

struct tqueue_s
{
    pthread_mutex_t mtx_in, mtx_out;
    pthread_cond_t  ready;
    size_t out_ind;
    vec *data_in;
    vec *data_out;
    vec  data_a;
    vec  data_b;
};

void tqueue_init(tqueue *tq, size_t width);

void tqueue_kill(tqueue *tq);

void tqueue_in(tqueue *tq, void *item);

void *tqueue_out(tqueue *tq);

int tqueue_empty(tqueue *tq);

#endif /* UTIL_H */
