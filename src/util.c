#include "text.h"

#include "util.h"

void util_str2vec(char *str, vec *v)
{
    size_t width = 1, utf8ind = 1;
    text_char *chr;

    if (!str) return;
    while (*str)
    {
        if (utf8ind == width)
        {
            width = text_utf8_len(*str);
            chr = vec_ins(v, vec_len(v), 1, NULL);
            chr->fg = text_col_none;
            utf8ind = 0;
        }

        chr->utf8[utf8ind]  = *str;
        utf8ind            += 1;
        str++;
    }
}


void tqueue_init(tqueue *tq, size_t width)
{
    vec_init(&(tq->data_a), width);
    vec_init(&(tq->data_b), width);

    tq->data_in  = &(tq->data_a);
    tq->data_out = &(tq->data_b);

    pthread_mutex_init(&(tq->mtx_in),  NULL);
    pthread_mutex_init(&(tq->mtx_out), NULL);
    pthread_cond_init (&(tq->ready),   NULL);
}

void tqueue_kill(tqueue *tq)
{
    pthread_mutex_destroy(&(tq->mtx_in));
    pthread_mutex_destroy(&(tq->mtx_out));
    
    pthread_cond_destroy(&(tq->ready));
}

static void tqueue_swap(tqueue *tq)
{
    vec *tmp;

    tmp          = tq->data_in;
    tq->data_in  = tq->data_out;
    tq->data_out = tmp;

    tq->out_ind  = 0;

    vec_del(tq->data_in, 0, vec_len(tq->data_in));
}

void tqueue_in(tqueue *tq, void *item)
{
    size_t len;

    USING_MTX(tq->mtx_in,
        len = vec_len(tq->data_in);
    
        vec_ins(tq->data_in, len, 1, item);
    );
    
    pthread_cond_broadcast(&(tq->ready));
}

void *tqueue_out(tqueue *tq)
{
    void *rtn;

    USING_MTX(tq->mtx_out,
        size_t len;
        len = vec_len(tq->data_out);

        if (len == tq->out_ind) 
        {
            USING_MTX(tq->mtx_in,
                if (!vec_len(tq->data_in))
                    pthread_cond_wait(&(tq->ready), &(tq->mtx_in));
            
                tqueue_swap(tq);
            );
        }

        rtn = vec_get(tq->data_out, tq->out_ind);
        tq->out_ind += 1;
    );

    return rtn;
}

int tqueue_empty(tqueue *tq)
{
    int rtn;

    USING_MTX(tq->mtx_out, 
        USING_MTX(tq->mtx_in,
            rtn = (vec_len(tq->data_in) == 0 && vec_len(tq->data_out) == tq->out_ind);
        );
    );

    return rtn;
}
