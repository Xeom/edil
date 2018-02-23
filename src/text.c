#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "cli.h"
#include "vec.h"
#include "util.h"
#include "bar.h"

#include "text.h"

text_buf *text_cur_buf;

void text_buf_setfg(text_buf *b, size_t cn, size_t ln, size_t maxcn, text_col col);
void text_buf_ins(text_buf *b, size_t cn, size_t ln, vec *chrs);
void text_buf_del(text_buf *b, size_t cn, size_t ln, size_t n);
void text_buf_del_line(text_buf *b, size_t ln);
void text_buf_ins_line(text_buf *b, size_t ln);

int text_utf8_len(char c)
{
    unsigned char utf8;

    utf8 = *(unsigned char *)(&c);

    if (utf8 <= 0x7f) return 1;
    if (utf8 <= 0xbf) return 1; /* This is an error :( */
    if (utf8 <= 0xdf) return 2;
    if (utf8 <= 0xef) return 3;
    if (utf8 <= 0xf7) return 4;
    if (utf8 <= 0xfb) return 5;
    if (utf8 <= 0xfd) return 6;
    return 1;
}

void text_buf_setflag(text_buf *b, text_flag f)
{
    USING_MTX(b->mtx,
        b->flags |= f;
    );
}

void text_buf_delflag(text_buf *b, text_flag f)
{
    USING_MTX(b->mtx,
        b->flags &= ~f;
    );
}

text_flag text_buf_getflag(text_buf *b)
{
    text_flag rtn;

    USING_MTX(b->mtx,
        rtn = b->flags;
    );

    return rtn;
}

void text_buf_cmd(text_buf *b, text_cmd *cmd)
{
    switch (cmd->type)
    {
    case text_cmd_setfg: text_buf_setfg(b,
        cmd->args.setfg.cn, cmd->args.setfg.ln, 
        cmd->args.setfg.maxcn, cmd->args.setfg.col);
        break;

    case text_cmd_ins: text_buf_ins(b,
        cmd->args.ins.cn, cmd->args.ins.ln, 
        &(cmd->args.ins.chrs));
        break;

    case text_cmd_del: text_buf_del(b,
        cmd->args.del.cn, cmd->args.del.ln,
        cmd->args.del.n);
        break;

    case text_cmd_ins_line: text_buf_ins_line(b,
        cmd->args.ins_line.ln);
        break;

    case text_cmd_del_line: text_buf_del_line(b,
        cmd->args.del_line.ln);
        break;
    }
}

void text_buf_getname(text_buf *b, vec *v)
{
    vec_init(v, sizeof(text_char));
    
    USING_MTX(b->mtx,
        vec_ins(v, 0, vec_len(&(b->name)), vec_get(&(b->name), 0));
    );
}

void text_buf_setname(text_buf *b, vec *v)
{
    USING_MTX(b->mtx,
        vec_del(&(b->name), 0, vec_len(&(b->name)));
        vec_ins(&(b->name), 0, vec_len(v), vec_get(v, 0));
    );

    bar_update();
}

void text_buf_getcur(text_buf *b, text_cur *cur)
{
    USING_MTX(
        b->mtx,
        memcpy(cur, &(b->cur), sizeof(text_cur));
    );
}

void text_buf_setcur(text_buf *b, text_cur *cur)
{
    USING_MTX(
        b->mtx,
        memcpy(&(b->cur), cur, sizeof(text_cur));
    );
}

void text_cur_cmd_pair(size_t *cn, size_t *ln, text_cmd *cmd)
{
    switch (cmd->type)
    {
    case text_cmd_ins:
        if (*ln == cmd->args.ins.ln 
                && cmd->args.ins.cn < *cn)
            *cn += vec_len(&(cmd->args.ins.chrs));
        break;

    case text_cmd_del:
        if (*ln == cmd->args.del.ln && *cn > cmd->args.del.cn)
        {
            if (*cn > cmd->args.del.cn + cmd->args.del.n)
                *cn -= cmd->args.del.n;
            else
                *cn  = cmd->args.del.cn;
        }
        break;

    case text_cmd_ins_line:
        if (*ln >= cmd->args.ins_line.ln)
            *ln += 1;
        break;

    case text_cmd_del_line:
        if (*ln >= cmd->args.del_line.ln)
            *ln -= 1;
        if (*ln == cmd->args.del_line.ln)
            *cn  = 0;
        break;
    }
}

void text_cur_cmd(text_buf *b, text_cmd *cmd)
{
    text_cur cur;

    text_buf_getcur(b, &cur);

    text_cur_cmd_pair(&(cur.cn1), &(cur.ln1), cmd);
    text_cur_cmd_pair(&(cur.cn2), &(cur.ln2), cmd);

    text_buf_setcur(b, &cur);
}

void text_buf_update_cur(text_buf *b, text_cur *orig)
{
    text_cur *new;
    new = &(b->cur);

    if (new->ln1 != orig->ln1)
    {
        cli_line(b, orig->cn1, orig->ln1);
        cli_line(b, new->cn1,  new->ln1);
    }
    else
    {
        cli_line(b, (orig->cn1 < new->cn1) ? orig->cn1 : new->cn1, new->ln1);
    }

    if (new->ln2 != orig->ln2)
    {
        cli_line(b, orig->cn2, orig->ln2);
        cli_line(b, new->cn2,  new->ln2);
    }
    else
    {
        cli_line(b, (orig->cn2 < new->cn2) ? orig->cn2 : new->cn2, new->ln2);
    }
}

void text_buf_init(text_buf *b)
{
    vec      *line;
    pthread_mutex_init(&(b->mtx), NULL);

    vec_init(&(b->lines), sizeof(vec));
    line = vec_ins(&(b->lines), 0, 1, NULL);

    vec_init(line, sizeof(text_char));
 
    b->flags  = 0;

    b->cur.cn1 = 0;
    b->cur.ln1 = 0;
    b->cur.cn2 = 0;
    b->cur.ln2 = 0;

    b->x = 0;
    b->y = 0;
    b->w = 64;
    b->h = 16;

    b->scrollx = 0;
    b->scrolly = 0; 
}

void text_buf_kill(text_buf *b)
{
    size_t ln, len;

    len = vec_len(&(b->lines));
    for (ln = 0; ln < len; ln++)
        vec_kill(vec_get(&(b->lines), ln));

    vec_kill(&(b->lines));
}

void text_buf_setfg(text_buf *b, size_t cn, size_t ln, size_t maxcn, text_col col)
{
    size_t curcn;
    vec   *line;

    USING_MTX(b->mtx,
        line = vec_get(&(b->lines), ln);
  
        if (line)
        {
            for (curcn = cn; curcn <= maxcn; curcn++)
            {
                text_char *chr;
                chr = vec_get(line, cn);

                if (!chr) break;

                chr->fg = col;
            }
        }
    );
  
}

void text_buf_ins(text_buf *b, size_t cn, size_t ln, vec *chrs)
{
    vec       *line;
    size_t     len;

    USING_MTX(b->mtx,
        line = vec_get(&(b->lines), ln);
        if (line)
        {
            len = vec_len(chrs);
            vec_ins(line, cn, len, vec_get(chrs, 0));
        }
    );
}

void text_buf_del(text_buf *b, size_t cn, size_t ln, size_t n)
{
    vec *line;

    USING_MTX(b->mtx,
        line = vec_get(&(b->lines), ln);
        if (line)
            vec_del(line, cn, n);
    );
}

void text_buf_ins_line(text_buf *b, size_t ln)
{
    vec *line;

    USING_MTX(b->mtx,
        line = vec_ins(&(b->lines), ln, 1, NULL);
        if (line)
            vec_init(line, sizeof(text_char));
    );
}

void text_buf_del_line(text_buf *b, size_t ln)
{
    vec *line;
    USING_MTX(b->mtx,
        line = vec_get(&(b->lines), ln);

        if (line)
        {
            vec_kill(line);
            vec_del(&(b->lines), ln, 1);
        }

        if (vec_len(&(b->lines)) == 0)
        {
            vec *line;
            line = vec_ins(&(b->lines), 0, 1, NULL);
            if (line)
                vec_init(line, sizeof(text_char));
        }
    );
}

void text_buf_get(text_buf *b, size_t ln, vec *v)
{
    vec *line;

    vec_init(v, sizeof(text_char));

    USING_MTX(b->mtx,
        line = vec_get(&(b->lines), ln);
        if (line) 
            vec_ins(v, 0, vec_len(line), vec_get(line, 0));
    );
}

size_t text_buf_linelen(text_buf *b, size_t ln)
{
    vec *line;
    size_t rtn;

    USING_MTX(b->mtx,   
        line = vec_get(&(b->lines), ln);
        if (line) 
            rtn = vec_len(line);
    );
   
    if (line) return rtn;

    return 0;
}

size_t text_buf_len(text_buf *b)
{
    size_t rtn;

    USING_MTX(b->mtx,
        rtn = vec_len(&(b->lines));
    );

    return rtn;
}


