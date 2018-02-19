typedef enum
{
    cmd_mode_bar;
    cmd_mode_buf;
    cmd_mode_mnu;
    cmd_mode_sel;
    cmd_mode_rct;   
} cur_mode_type;

cur_mode_type cur_mode;

struct cur_s
{
    ssize_t cn, ln;
};

cur cur_enter(cur c, buf *b)
{
    size_t len;
    len = buf_linelen(b, b->pri.ln);
    cur rtn = { .ln = c.ln + 1, .cn = 0 };

    buf_insline(b, c);

    if (len > c.cn)
    {
        vec *line;
        size_t num;

        line = vec_get(&(b->lines), c.ln);
        if (!line) return;

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

    if      (c.ln <  0)   c.ln = 0;
    else if (c.ln >= len) c.ln = len - 1;

    len = buf_line_len(b, c.ln);

    if      (c.cn < 0)   c.cn = 0;
    else if (c.cn > len) c.cn = len;

    return c;
}

cur cur_move(cur c, buf *b, cur dir)
{
    c.ln += dir.ln;
    c.cn += dir.cn;

    c = cur_check_bounds(b, c);

    return c;
}

cur cur_del(cur c, buf *b)
{
    size_t len;

    len = buf_line_len(b, c.ln);

    if (len == c.cn)
    {
        vec *line;
        size_t num;
        
        line = vec_get(&(b->lines), c.ln + 1);
        if (!line) return;

        num = vec_len(line);

        buf_ins(b, c, vec_get(line, 0), num);
        buf_del_line(b, { .ln = c.ln + 1, .cn = 0 });
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
