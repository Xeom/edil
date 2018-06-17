#include <string.h>

#include "text/chr.h"
#include "container/vec.h"
#include "file.h"
#include "text/line.h"

#include "text/buf.h"

void buf_init(buf *b)
{
    text_init(&(b->t));
    vec_init(&(b->name), sizeof(char));
    file_init(&(b->finfo));

    b->flags = 0;

    b->prihint = (cur){0, 0};

    text_ins_lines(&(b->t), (cur){0, 0}, 1);
}

void buf_kill(buf *b)
{
    file_kill(&(b->finfo));
    vec_kill(&(b->name));
    text_kill(&(b->t));
}

void buf_ins(buf *b, cur loc, vec *chrs)
{
    line *l;

    l = text_get_line(&(b->t), loc);
    if (!l) return;

    line_ins(l, loc, chrs);

    line_unlock(l);
}

void buf_ins_str(buf *b, cur loc, char *str)
{
    line *l;

    l = text_get_line(&(b->t), loc);
    if (!l) return;

    line_ins_str(l, loc, str);

    line_unlock(l);
}

void buf_ins_mem(buf *b, cur loc, size_t n, chr *mem)
{
    line *l;

    l = text_get_line(&(b->t), loc);
    if (!l) return;

    line_ins_mem(l, loc, n, mem);

    line_unlock(l);
}

void buf_del(buf *b, cur loc, size_t n)
{
    line *l;

    l = text_get_line(&(b->t), loc);
    if (!l) return;

    line_del(l, loc, n);

    line_unlock(l);
}

void buf_clr(buf *b)
{
    size_t len;

    b->prihint = (cur){0, 0};

    len = buf_len(b);
    buf_del_lines(b, (cur){0, 0}, len);
}

void buf_ins_nl(buf *b, cur loc)
{
    ssize_t num;
    vec    chrs;
    cur    newloc;

    if (loc.ln >= buf_len(b)) return;

    newloc = (cur){ .ln = loc.ln + 1 };

    vec_init(&chrs, sizeof(chr));
    buf_cpy_line(b, loc, &chrs);
    buf_ins_lines(b, newloc, 1);

    num = vec_len(&chrs) - loc.cn;

    if (num > 0)
    {
        if (vec_len(&chrs) > (size_t)num)
            vec_del(&chrs, 0, vec_len(&chrs) - num);

        buf_del(b, loc,    num);
        buf_ins(b, newloc, &chrs);
    }

    vec_kill(&chrs);
}

void buf_del_nl(buf *b, cur loc)
{
    cur    prevloc;
    vec    chrs;

    if (loc.ln >= buf_len(b) - 1) return;

    prevloc = (cur){ .ln = loc.ln + 1 };

    vec_init(&chrs, sizeof(chr));
    buf_cpy_line(b, prevloc, &chrs);
    buf_del_lines(b, prevloc, 1);

    loc.cn = buf_line_len(b, loc);

    if (vec_len(&chrs))
        buf_ins(b, loc, &chrs);

    vec_kill(&chrs);
}

ssize_t buf_line_len(buf *b, cur loc)
{
    ssize_t rtn;
    line *l;

    l = text_get_line(&(b->t), loc);
    if (!l) return 0;

    rtn = line_len(l);

    line_unlock(l);
    return rtn;
}

chr buf_chr(buf *b, cur loc)
{
    line *l;
    chr rtn, *ptr;

    chr_blankify(&rtn);

    l = text_get_line(&(b->t), loc);
    if (!l) return rtn;

    ptr = line_chr(l, loc);
    if (!ptr) return rtn;
    rtn = *ptr;
    line_unlock(l);

    return rtn;
}

void buf_ins_from(buf *b, cur c, buf *oth, cur loc, cur end)
{
    vec chrs;
    vec_init(&chrs, sizeof(chr));

    for (; loc.ln < end.ln; loc = (cur){ .ln = loc.ln + 1 })
    {
        ssize_t len, num;

        vec_clr(&chrs);
        text_cpy_line(&(oth->t), loc, &chrs);

        len = vec_len(&chrs);
        num = len - loc.cn;

        if (num > 0)
            buf_ins(b, c, &chrs);

        c.cn = len;
        buf_ins_nl(b, c);
        c = (cur){ .ln = c.ln + 1 };
    }

    if (loc.ln == end.ln)
    {
        ssize_t num;

        vec_clr(&chrs);
        text_cpy_line(&(oth->t), loc, &chrs);

        num = end.cn - loc.cn;

        if (num > 0)
            buf_ins_mem(b, c, num, vec_get(&chrs, loc.cn));

        loc.cn += num;
    }

    vec_kill(&chrs);
}

void buf_set_name(buf *b, char *name)
{
    vec_clr(&(b->name));
    vec_str(&(b->name), name);
    vec_app(&(b->name), "\0");
}

char *buf_get_name(buf *b)
{
    static char *noname = "???";
    char *fname;

    fname = file_base(&(b->finfo));

    if (strlen(fname))
        return fname;

    else if (vec_len(&(b->name)))
        return vec_first(&(b->name));

    else
        return noname;
}
