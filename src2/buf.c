#include "buf.h"

void buf_setcol(buf *b, cur loc, size_t n, col_desc *col)
{
    size_t cn;
    vec *line;

    line = vec_get(&(b->lines), loc.ln);
    if (!line) return;

    for (cn = loc.cn; cn < len + loc.cn; cn++)
    {
        chr *c;
        c = vec_get(line, cn);
        if (!c) break;

        chr_set_cols(c, col);
    }
}

void buf_ins(buf *b, cur loc, text_char *chrs, size_t n)
{
    size_t len;

    line = vec_get(&(b->lines), loc.ln);
    if (!line) return;

    vec_ins(line, cn, n, chrs);
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

    line = vec_ins(&(b->lines), loc.ln);
    if (!line) return;

    vec_init(line, sizeof(chr));
}

void buf_del_line(buf *b, cur loc)
{
    vec *line;

    line = vec_get(&(b->lines), loc.ln)
    if (!line) return;

    vec_kill(line);
    vec_del(&(b->lines), loc.ln);

    if (vec_len(&(b->lines) == 0))
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

