#include "cur.h"

cur_mode_type cur_mode;

cur cur_enter(cur c, buf *b)
{
    size_t len;
    len = buf_line_len(b, c);
    cur rtn = { .ln = c.ln + 1, .cn = 0 };

    buf_ins_line(b, rtn);

    if ((ssize_t)len > c.cn)
    {
        vec *line;
        size_t num;

        line = vec_get(&(b->lines), c.ln);
        if (!line) return c;

        num = len - b->pri.cn;

        buf_ins(b, rtn, vec_get(line, c.cn), num);
	buf_del(b, c,   num);
    }

    return rtn;
}

cur cur_check_bounds(cur c, buf *b)
{
    size_t len;

    len = buf_len(b);

    if (c.ln <  0) c.ln = 0;
    else if (c.ln >= (ssize_t)len) c.ln = len - 1;

    len = buf_line_len(b, c);

    if (c.cn < 0) c.cn = 0;
    else if (c.cn > (ssize_t)len) c.cn = len;

    return c;
}

cur cur_move(cur c, buf *b, cur dir)
{
    c.ln += dir.ln;
    c.cn += dir.cn;

    c = cur_check_bounds(c, b);

    return c;
}

cur cur_del(cur c, buf *b)
{
    size_t len;

    len = buf_line_len(b, c);

    if ((ssize_t)len == c.cn)
    {
        vec *line;
        size_t num;
        cur delcur = { .ln = c.ln + 1, .cn = 0 };
        
        line = vec_get(&(b->lines), c.ln + 1);
        if (!line) return c;

        num = vec_len(line);

        buf_ins(b, c, vec_get(line, 0), num);
        buf_del_line(b, delcur);
    }
    else
    {
        buf_del(b, c, 1);
    }

    return c;
}

cur cur_ins(cur c, buf *b, vec *text)
{
    size_t num;
    num = vec_len(text);

    buf_ins(b, c, vec_get(text, 0), num);
    c.cn += num;

    return c;
}
