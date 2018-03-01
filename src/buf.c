#include <string.h>

#include "buf.h"

void buf_init(buf *b)
{
    vec_init(&(b->lines), sizeof(vec));
    b->flags = 0;

    buf_ins_line(b, (cur){0, 0});
}

void buf_kill(buf *b)
{
    size_t ln;

    for (ln = 0; ln < buf_len(b); ln++)
        vec_kill(vec_get(&(b->lines), ln));

    vec_kill(&(b->lines));
}

void buf_setcol(buf *b, cur loc, size_t n, col_desc col)
{
    size_t cn, len;
    vec *line;

    line = vec_get(&(b->lines), loc.ln);
    if (!line) return;

    len = vec_len(line);

    for (cn = loc.cn; cn < len + loc.cn; cn++)
    {
        chr *c;
        c = vec_get(line, cn);
        if (!c) break;

        chr_set_cols(c, col);
    }
}

void buf_ins(buf *b, cur loc, chr *chrs, size_t n)
{
    vec *line;

    line = vec_get(&(b->lines), loc.ln);
    if (!line) return;

    vec_ins(line, loc.cn, n, chrs);
}

void buf_del(buf *b, cur loc, size_t n)
{
    vec *line;

    line = vec_get(&(b->lines), loc.ln);
    if (!line) return;
    
    vec_del(line, loc.cn, n);
}

void buf_ins_line(buf *b, cur loc)
{
    vec *line;

    line = vec_ins(&(b->lines), loc.ln, 1, NULL);
    if (!line) return;

    vec_init(line, sizeof(chr));
}

void buf_del_line(buf *b, cur loc)
{
    vec *line;

    line = vec_get(&(b->lines), loc.ln);
    if (!line) return;

    vec_kill(line);
    vec_del(&(b->lines), loc.ln, 1);

    if (vec_len(&(b->lines)) == 0)
    {
        line = vec_ins(&(b->lines), 0, 1, NULL);
        if (!line) return;

        vec_init(line, sizeof(chr));
    }
}

size_t buf_len(buf *b)
{
    return vec_len(&(b->lines));
}

size_t buf_line_len(buf *b, cur loc)
{
    vec *line;

    line = vec_get(&(b->lines), loc.ln);
    if (!line) return 0;

    return vec_len(line);
}

