#include <string.h>

#include "buf.h"
#include "indent.h"

void buf_init(buf *b)
{
    vec_init(&(b->lines), sizeof(vec));
    vec_init(&(b->fname), sizeof(char));
    b->flags = 0;

    buf_ins_line(b, (cur){0, 0});
}

void buf_kill(buf *b)
{
    size_t ln;

    for (ln = 0; ln < buf_len(b); ln++)
        vec_kill(vec_get(&(b->lines), ln));

    vec_kill(&(b->lines));
    vec_kill(&(b->fname));
}

void buf_setcol(buf *b, cur loc, size_t n, col_desc col)
{
    size_t cn, len;
    vec *line;

    line = vec_get(&(b->lines), loc.ln);
    if (!line) return;

    len = vec_len(line);

    if (n + loc.cn > len) n = len - loc.cn;

    for (cn = loc.cn; cn < n + loc.cn; cn++)
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
    indent_add_blanks_line(line, loc.cn);
}

void buf_del(buf *b, cur loc, size_t n)
{
    size_t len;
    vec *line;
    chr *c;

    line = vec_get(&(b->lines), loc.ln);
    len  = vec_len(line);

    if (loc.cn     < 0)   loc.cn = 0;
    if (loc.cn + n > len) loc.cn = len - n;

    if (len == 0) return;

    if (n == 0) return;

    while (loc.cn > 0)
    {
        c = vec_get(line, loc.cn);
        if (!c) return;
        if (!chr_is_blank(c)) break;
        loc.cn -= 1;
        n      += 1;
    }

    while (loc.cn + n < len)
    {
        c = vec_get(line, loc.cn + n);
        if (!chr_is_blank(c)) break;
        n += 1;
    }

    if (!line) return;

    vec_del(line, loc.cn, n);
    indent_add_blanks_line(line, loc.cn);
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

