struct line_s
{
    pthread_mutex_t lock;
    vec chrs;
};

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
        pthread_mutex_lock(&(rtn->lock));
    }
    else
        rtn = NULL;

    pthread_mutex_unlock(&(t->lock));

    return rtn;
}

void  text_release_line(text *t, line *l);
{
    if (!l) return;

    pthread_mutex_unlock(&(l->lock));
}

chr text_get_chr(text *t, cur c)
{
}

ssize_t text_len(text *t);
ssize_t text_line_len(text *t, cur c);

void text_del_line(text *t, cur c);
void text_ins_line(text *t, cur c);
void text_ins_lines(text *t, cur c, ssize_t n);
void text_del_lines(text *t, cur c, ssize_t n);