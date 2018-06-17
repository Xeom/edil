#include <pthread.h>
#include <stdlib.h>

#include "container/circvec.h"
#include "container/table.h"
#include "container/vec.h"
#include "text/cur.h"

#include "updater.h"

static int updater_process(updater *u);
static vec *updater_get_send_vec(void);
static void *updater_listener_thread(void *arg);

static pthread_key_t updater_send_to_key;

pthread_mutex_t updater_all_mtx;
vec updater_all;

static void updater_destruct_vec(void *v)
{
    vec_kill(v);
    free(v);
}

void updater_init(void)
{
    pthread_mutex_init(&updater_all_mtx, NULL);
    vec_init(&updater_all, sizeof(updater *));

    pthread_key_create(&updater_send_to_key, updater_destruct_vec);
}

void updater_start(updater *u)
{
    table_init(&(u->after), sizeof(cur), sizeof(buf *));
    circvec_init(&(u->curs), sizeof(updater_line_loc), 16);

    pthread_mutex_init(&(u->lock), NULL);
    pthread_cond_init(&(u->ready), NULL);

    pthread_mutex_lock(&updater_all_mtx);
    vec_app(&updater_all, &u);
    pthread_create(&(u->thread), NULL, updater_listener_thread, u);
    pthread_mutex_unlock(&updater_all_mtx);
}

static vec *updater_get_send_vec(void)
{
    vec *rtn;

    rtn = pthread_getspecific(updater_send_to_key);

    if (!rtn)
    {
        rtn = malloc(sizeof(vec));
        vec_init(rtn, sizeof(updater_line_loc *));

        pthread_setspecific(updater_send_to_key, rtn);
    }

    return rtn;
}

void updater_del_buf(buf *b)
{
    pthread_mutex_lock(&updater_all_mtx);

    VEC_FOREACH(&updater_all, updater **, u,
        vec tmp;
        table *after;
        circvec *curs;

        after = &(*u)->after;
        curs  = &(*u)->curs;

        if (table_get(after, b))
            table_delete(after, b);

        vec_init(&tmp, sizeof(updater_line_loc));
        while (!circvec_empty(curs))
        {
            updater_line_loc *loc;
            loc = circvec_pop(curs);

            if (loc->b == b) continue;

            vec_app(&tmp, circvec_pop(curs));
        }

        VEC_FOREACH(&tmp, updater_line_loc *, loc,
            circvec_pipe_push(curs, loc);
        );
    )

    pthread_mutex_unlock(&updater_all_mtx);
}

void updater_send_to(updater *u)
{
    vec *v;

    v = updater_get_send_vec();
    vec_app(v, &u);
}

void updater_after(buf *b, cur c)
{
    vec *sendto;

    sendto = updater_get_send_vec();
    VEC_FOREACH(sendto, updater **, uptr,
        updater *u = *uptr;
        cur *after;
        pthread_mutex_lock(&(u->lock));

        after = table_get(&(u->after), &b);

        if (!after)
            table_set(&(u->after), &b, &c);
        else
            *after = CUR_END(*after, c);

        pthread_cond_signal(&(u->ready));
        pthread_mutex_unlock(&(u->lock));
    )
}

void updater_line(buf *b, cur c)
{
    vec *sendto;

    sendto = updater_get_send_vec();
    VEC_FOREACH(sendto, updater **, uptr,
        updater *u;
        updater_line_loc lc;
        cur *cmp;
        u = *uptr;

        pthread_mutex_lock(&(u->lock));

        cmp = table_get(&(u->after), &b);

        if (!(cmp && CUR_GREATER(c, cmp)))
        {
            lc.c = c;
            lc.b = b;

            circvec_pipe_push(&(u->curs), &lc);
        }

        pthread_cond_broadcast(&(u->ready));
        pthread_mutex_unlock(&(u->lock));
    )
}

static void *updater_listener_thread(void *arg)
{
    updater *u;
    u = arg;

    pthread_mutex_lock(&(u->lock));

    while (1)
    {
        if (updater_process(u) == 1)
        {
            pthread_mutex_unlock(&(u->lock));
            pthread_mutex_lock(&(u->lock));
        }
        else
        {
            pthread_cond_wait(&(u->ready), &(u->lock));
        }
    }

    return NULL;
}

/* We assume we have already got the lock! */
static int updater_process(updater *u)
{
    buf **b;
    cur *c;
    updater_line_loc *loc;

    c = table_next(&(u->after), NULL, (void **)&b);

    if (c)
    {
        if ((u->fptr_after)(*b, c) == 1)
            *c = (cur){ .ln = c->ln + 1 };
        else
        {
            table_delete(&(u->after), b);
        }

        return 1;
    }

    if (circvec_empty(&(u->curs)))
        return 0;

    loc = circvec_pop(&(u->curs));

    (u->fptr_line)(loc->b, &(loc->c));

    return 1;
}
