#include <pthread.h>

#include "container/vec.h"
#include "buf/chr.h"
#include "indent.h"
#include "buf/line.h"

void line_init(line *l)
{
    if (!l) return;

    vec_init(&(l->chrs), sizeof(chr));
    pthread_mutex_init(&(l->lock), NULL);
}

void line_kill(line *l)
{
    if (!l) return;

    vec_kill(&(l->chrs));
    pthread_mutex_destroy(&(l->lock));
}

void line_lock(line *l)
{
    if (!l) return;

    pthread_mutex_lock(&(l->lock));
}

void line_unlock(line *l)
{
    if (!l) return;

    pthread_mutex_unlock(&(l->lock));
}

void line_cpy(line *l, vec *to)
{
    if (!l) return;

    vec_cpy(to, &(l->chrs));
}

chr *line_chr(line *l, cur c)
{
    if (!l) return NULL;

    return vec_get(&(l->chrs), c.cn);
}

vec *line_vec(line *l)
{
    if (!l) return NULL;

    return &(l->chrs);
}

ssize_t line_len(line *l)
{
    if (!l) return 0;

    return vec_len(&(l->chrs));
}

void line_ins(line *l, cur c, vec *chrs)
{
    line_ins_mem(l, c, vec_len(chrs), vec_first(chrs));
}

void line_ins_str(line *l, cur c, char *str)
{
    vec chrs;
    vec_init(&chrs, sizeof(char));

    chr_from_str(&chrs, str);
    line_ins(l, c, &chrs);

    vec_kill(&chrs);
}

void line_ins_mem(line *l, cur c, size_t n, chr *mem)
{
    ssize_t len;

    if (!l) return;

    len = vec_len(&(l->chrs));
    if (c.cn > len) c.cn = len;

    if (n)
        vec_ins(&(l->chrs), c.cn, n, mem);

    indent_add_blanks_line(&(l->chrs), 0);
}

void line_del(line *l, cur c, size_t n)
{
    if (c.cn + (ssize_t)n > line_len(l))
        return;

    while (c.cn && chr_is_blank(line_chr(l, c)))
    {
        c.cn -= 1;
        n    += 1;
    }

    vec_del(&(l->chrs), c.cn, n);
    indent_add_blanks_line(&(l->chrs), 0);
}
