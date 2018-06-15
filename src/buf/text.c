#include <stdlib.h>

#include "container/vec.h"
#include "buf/line.h"

#include "buf/text.h"

void text_init(text *t)
{
    vec_init(&(t->lines), sizeof(line *));
    pthread_mutex_init(&(t->lock), NULL);
}

void text_kill(text *t)
{
    pthread_mutex_lock(&(t->lock));

    VEC_FOREACH(&(t->lines), line **, ptr,
        line_kill(*ptr);
        free(*ptr);
    );

    vec_kill(&(t->lines));

    pthread_mutex_unlock(&(t->lock));
    pthread_mutex_destroy(&(t->lock));
}

line *text_get_line(text *t, cur c)
{
    line **ptr, *rtn;

    if (!t) return NULL;

    pthread_mutex_lock(&(t->lock));

    ptr = vec_get(&(t->lines), c.ln);

    if (ptr)
    {
        rtn = *ptr;
        line_lock(rtn);
    }
    else
        rtn = NULL;

    pthread_mutex_unlock(&(t->lock));

    return rtn;
}

void text_del_lines(text *t, cur c, size_t n)
{
    ssize_t ind, len;
    line **ptr, *l;
    int candel = 1;

    pthread_mutex_lock(&(t->lock));

    len = vec_len(&(t->lines));
    if (c.ln >= len) candel = 0;
    if (c.ln + (ssize_t)n > len) n = len - c.ln;

    if (candel)
    {
        for (ind = c.ln; ind < c.ln + (ssize_t)n; ++ind)
        {
            ptr = vec_get(&(t->lines), c.ln);
            if (!ptr) continue;

            l = *ptr;
            line_lock(l);
            line_unlock(l);
            line_kill(l);
            free(l);
        }

        vec_del(&(t->lines), c.ln, n);
    }

    if (vec_len(&(t->lines)) == 0)
    {
        l = malloc(sizeof(line));
        line_init(l);
        vec_app(&(t->lines), &l);
    }

    pthread_mutex_unlock(&(t->lock));
}

void text_ins_lines(text *t, cur c, size_t n)
{
    ssize_t ind, len;
    line **ptr, *l;

    pthread_mutex_lock(&(t->lock));

    len = vec_len(&(t->lines));

    if (c.ln > len) c.ln = len;

    vec_ins(&(t->lines), c.ln, n, NULL);

    for (ind = c.ln; ind < c.ln + (ssize_t)n; ++ind)
    {
        ptr = vec_get(&(t->lines), c.ln);
        if (!ptr) continue;

        l = malloc(sizeof(line));
        *ptr = l;
        line_init(l);
    }

    pthread_mutex_unlock(&(t->lock));
}

line *text_new_line(text *t, cur c)
{
    ssize_t len;
    line **ptr, *l;

    pthread_mutex_lock(&(t->lock));

    len = vec_len(&(t->lines));

    if (c.ln > len) c.ln = len;

    ptr = vec_ins(&(t->lines), c.ln, 1, NULL);

    pthread_mutex_unlock(&(t->lock));

    if (!ptr) return NULL;

    l = malloc(sizeof(line));
    *ptr = l;

    line_init(l);
    line_lock(l);

    return l;
}

size_t text_len(text *t)
{
    size_t rtn;

    pthread_mutex_lock(&(t->lock));

    rtn = vec_len(&(t->lines));

    pthread_mutex_unlock(&(t->lock));

    return rtn;
}

cur text_last(text *t)
{
    cur rtn;
    line *l;

    pthread_mutex_lock(&(t->lock));

    rtn.ln = vec_len(&(t->lines)) - 1;

    pthread_mutex_unlock(&(t->lock));

    l = text_get_line(t, rtn);
    rtn.cn = line_len(l);

    line_unlock(l);

    return rtn;
}
