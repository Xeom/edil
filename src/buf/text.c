typedef struct text_s text;

struct text_s
{
    pthread_mutex_t lock;
    vec lines;
};

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
    size_t ind, end, len;
    line **ptr, *l;

    pthread_mutex_lock(&(t->lock));

    len = vec_len(&(t->lines));

    if (n + c.ln > len) n = len - c.ln;

    if (len == n) return;

    for (ind = c.ln; ind < c.ln + n; ++ind)
    {
        ptr = vec_get(&(t->lines), c.ln);
        if (!ptr) continue;

        l = *ptr;
        line_lock(l);
        line_unlock(l);
        line_kill(l);
    }

    vec_del(&(t->lines), c.ln, n);

    pthread_mutex_unlock(&(t->lock));
}

void text_ins_lines(text *t, cur c, size_t n)
{
    size_t ind, end, len;
    line **ptr, *l;

    pthread_mutex_lock(&(t->lock));

    len = vec_len(&(t->lines));

    if (c.ln > len) c.ln = len;

    vec_ins(&(t->lines), c.ln, n);

    for (ind = c.ln; ind < end; ++ind)
    {
        ptr = vec_get(&(t->lines), c.ln);
        if (!ptr) continue;

        l = *ptr;
        line_init(l);
    }

    pthread_mutex_unlock(&(t->lock));
}

line *text_new_line(text *t, cur c)
{
    pthread_mutex_lock(&(t->lock));

    len = vec_len(&(t->lines));

    if (c.ln > len) c.ln = len;;

    vec_ins(&(t->lines), c.ln, 1);

    ptr = vec_get(&(t->lines), c.ln);
    if (!ptr) continue;

    l = *ptr;
    line_init(l);
    line_lock(l);

    return l;
}