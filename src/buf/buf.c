struct buf_s
{
    text t;
    file finfo;
    vec  name;
    cur  prihint;
    buf_flags flags;
};

void buf_init(buf *b)
{
    text_init(&(b->t));
    vec_init(&(b->name),  sizeof(char));
    file_init(&(b->finfo));

    b->flags = 0;

    b->prihint = (cur){0, 0};

    buf_ins_line(b, (cur){0, 0});
}

void buf_kill(buf *b)
{
    size_t ln;

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

    len = text_len(&(b->t));
    text_del_lines(&(b->t), (cur){0, 0}, len);
}

void buf_ins_nl(buf *b, cur loc)
{
    size_t num;
    cur    newloc;
    line  *line1, *line2;

    newloc = (cur){ .ln = loc.ln + 1 };

    line1 = text_get_line(&(b->t), loc);
    if (!line1) return;
    line2 = text_new_line(&(b->t), newloc);
    if (!line2) return;

    num = line_len(line1) - loc.cn;

    if (num > 0)
    {
        chr *mem;

        mem = line_chr(line1, loc);
        line_ins_mem(line2, newloc, num, mem);
        line_del(line1, loc, num);
    }

    line_unlock(line1);
    line_unlock(line2);
}

void buf_del_nl(buf *b, cur loc)
{
    size_t num;
    cur    prevloc;
    line  *line1, *line2;

    prevloc = (cur){ .ln = loc.ln + 1 };

    line1 = text_get_line(&(b->t), loc);
    if (!line1) return;
    line2 = text_get_line(&(b->t), prevloc);
    if (!line2) return;

    num = line_len(line2);

    if (num > 0)
        line_cpy(line2, line_vec(line1));

    line_unlock(line1);
    line_unlock(line2);

    text_del_lines(&(b->t), prevloc, 1);
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

    return rtn;
}

void buf_ins_from(buf *b, cur c, buf *oth, cur loc, cur end)
{
    for (; loc.ln < end.ln; loc = (cur){ .ln = loc.ln + 1 })
    {
        ssize_t len, num;
        line *l;
        l   = text_get_line(&(oth->t), loc);
        len = line_len(line);
        num = len - loc.cn;

        if (num > 0)
            buf_ins_mem(b, c, num, line_chr(l, loc));

        c = (cur){ .cn = len };
        buf_ins_nl(b, c);
        c = (cur){ .ln = c->ln + 1 };

        line_unlock(l);
    }

    if (loc.ln == end.ln)
    {
        ssize_t num;
        line *l;
        text_get_line(&(oth->t), loc);

        num = end.cn - loc.cn;

        if (num > 0)
            buf_ins_mem(b, c, num, line_chr(l, loc));

        loc->cn += num;

        line_unlock(l);
    }
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
