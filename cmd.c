#include <string.h>

#include "cli.h"
#include "vec.h"

#include "cmd.h"

typedef enum
{
    cmd_mode_bar,
    cmd_mode_buf,
    cmd_mode_opt
} cmd_mode;

cmd_mode cmd_cur_mode = cmd_mode_buf;

void cmd_move(text_buf *b, size_t *cn, size_t *ln, ssize_t cols, ssize_t lines)
{
    if (lines < -(ssize_t)*ln) 
        lines = -(ssize_t)*ln;

    if (lines > (ssize_t)text_buf_len(b) - 1 - (ssize_t)*ln)
        lines = (ssize_t)text_buf_len(b) - 1 - (ssize_t)*ln;

    *ln += lines;

    if (cols < -(ssize_t)*cn) 
        cols = -(ssize_t)*cn;

    if (cols > (ssize_t)text_buf_linelen(b, *ln) - (ssize_t)*cn)
        cols = (ssize_t)text_buf_linelen(b, *ln) - (ssize_t)*cn;

    *cn += cols;
}

void cmd_arrow(cli_key key)
{
    text_cur orig, cur;
    text_buf_getcur(text_cur_buf, &cur);
    memcpy(&orig, &cur, sizeof(text_cur));

    if (cmd_cur_mode == cmd_mode_buf)
    {
        switch (key)
        {
        case cli_key_up:    cmd_move(text_cur_buf, &cur.cn1, &cur.ln1,  0, -1); break;
        case cli_key_down:  cmd_move(text_cur_buf, &cur.cn1, &cur.ln1,  0,  1); break;
        case cli_key_left:  cmd_move(text_cur_buf, &cur.cn1, &cur.ln1, -1,  0); break;
        case cli_key_right: cmd_move(text_cur_buf, &cur.cn1, &cur.ln1,  1,  0); break;
        }
    }

    text_buf_setcur(text_cur_buf, &cur);
    text_buf_update_cur(text_cur_buf, &orig);
}

static vec cmd_ins_buf;

void cmd_init(void)
{
    vec_init(&cmd_ins_buf, sizeof(text_char));
}

void cmd_ins(cli_key key)
{
    static text_char chr;
    static int utf8ind = 0;
    static int width   = 0;

    chr.utf8[utf8ind] = key & 0xff;

    if (utf8ind == 0) width = text_utf8_len(chr.utf8[0]);
    utf8ind++;

    if (utf8ind == width)
    {
        if (utf8ind < 6) chr.utf8[utf8ind] = '\0';
        utf8ind = 0;
        chr.fg = text_col_none;
        vec_ins(&cmd_ins_buf, vec_len(&cmd_ins_buf), width, &chr);
    }
}

void cmd_ins_flush(void)
{
    text_cur cur;
    size_t   len;
    size_t   cn, ln;

    text_buf_getcur(text_cur_buf, &cur);

    len = vec_len(&cmd_ins_buf);
    if (len == 0) return;

    if (cmd_cur_mode == cmd_mode_buf)
    {
        text_cmd cmd;

        cn = cur.cn1;
        ln = cur.ln1;

        cmd.type = text_cmd_ins;
        cmd.args.ins.cn = cn;
        cmd.args.ins.ln = ln;
        memcpy(&(cmd.args.ins.chrs), &cmd_ins_buf, sizeof(vec));
 
        text_buf_cmd(text_cur_buf, &cmd);
        text_cur_cmd(text_cur_buf, &cmd);

        text_buf_getcur(text_cur_buf, &cur);
        cur.cn1 += vec_len(&cmd_ins_buf);
        text_buf_setcur(text_cur_buf, &cur);

        cli_line(text_cur_buf, cn, ln);
   }

   vec_del(&cmd_ins_buf, 0, len);
} 

void cmd_del_line(void)
{
    vec *v; text_cmd cmd;
    text_cur cur, orig; 
    size_t len, ln;

    text_buf_getcur(text_cur_buf, &cur);
    memcpy(&orig, &cur, sizeof(text_cur));
 
    ln = cur.ln1;
  
    if (ln == 0) return;
    
    v = &(cmd.args.ins.chrs);
    len = text_buf_linelen(text_cur_buf, ln - 1);

    cmd.type = text_cmd_ins;
    cmd.args.ins.ln = ln - 1;
    cmd.args.ins.cn = len;
    text_buf_get(text_cur_buf, ln, v);

    text_buf_cmd(text_cur_buf, &cmd);
    text_cur_cmd(text_cur_buf, &cmd);
   
    cmd.type = text_cmd_del_line;
    cmd.args.del_line.ln = ln;

    text_buf_cmd(text_cur_buf, &cmd);
    text_cur_cmd(text_cur_buf, &cmd);

    text_buf_getcur(text_cur_buf, &cur);
    cur.ln1 = ln - 1;
    cur.cn1 = len;
    text_buf_setcur(text_cur_buf, &cur);

    cli_line(text_cur_buf, len, ln - 1);
    cli_lines_after(text_cur_buf, ln);

    text_buf_update_cur(text_cur_buf, &orig);
}

void cmd_del(cli_key key)
{
    text_cmd cmd;
    text_cur cur, orig;
    size_t cn, ln;
    
    text_buf_getcur(text_cur_buf, &cur);
    memcpy(&orig, &cur, sizeof(text_cur));

    cn = cur.cn1;
    ln = cur.ln1;
    
    if (cmd_cur_mode == cmd_mode_buf)
    {
        switch (key)
        {
        case cli_key_back:
            if (cn == 0)
            {
                cmd_del_line();
                text_buf_update_cur(text_cur_buf, &orig);
                return;
            }

            cur.cn1 -= 1; 
            text_buf_setcur(text_cur_buf, &cur);
            cn = cur.cn1;

        case cli_key_del:   
            cmd.type = text_cmd_del;
            cmd.args.del.cn = cn;
            cmd.args.del.ln = ln;
            cmd.args.del.n  = 1;

            text_buf_cmd(text_cur_buf, &cmd);
            text_cur_cmd(text_cur_buf, &cmd);

            cli_line(text_cur_buf, cn, ln);
           break;
        }
    }
} 

void cmd_swap(cli_key key)
{
    text_cur *cur, orig;
    cur = &(text_cur_buf->cur);
    memcpy(&orig, cur, sizeof(text_cur));

    if (cmd_cur_mode == cmd_mode_buf)
    {
        cur->cn1 = orig.cn2;
        cur->cn2 = orig.cn1;
        cur->ln1 = orig.ln2;
        cur->ln2 = orig.ln1;

        text_buf_update_cur(text_cur_buf, &orig);
    }
}

void cmd_enter(cli_key key)
{
    text_cur cur;
    text_buf_getcur(text_cur_buf, &cur);

    if (cmd_cur_mode == cmd_mode_buf)
    {
        text_cmd cmd;
        size_t len, cn, ln;

        cn = cur.cn1;
        ln = cur.ln1;

        cmd.type = text_cmd_ins_line;
        cmd.args.ins_line.ln = ln + 1;
        text_buf_cmd(text_cur_buf, &cmd);
        text_cur_cmd(text_cur_buf, &cmd);

        len  = text_buf_linelen(text_cur_buf, ln);

        if (len > cn)
        {
            vec *v;

            v = &(cmd.args.ins.chrs);

            cmd.type = text_cmd_ins;
            cmd.args.ins.ln = ln + 1;
            cmd.args.ins.cn = 0;
            text_buf_get(text_cur_buf, ln, v);
            vec_del(v, 0, cn);

            text_buf_cmd(text_cur_buf, &cmd);
            text_cur_cmd(text_cur_buf, &cmd);

            vec_kill(v);

            cmd.type = text_cmd_del;
            cmd.args.del.ln = ln;
            cmd.args.del.cn = cn;
            cmd.args.del.n  = len - cn;
            text_buf_cmd(text_cur_buf, &cmd);
            text_cur_cmd(text_cur_buf, &cmd);
        }
 
        text_buf_getcur(text_cur_buf, &cur);
        cur.cn1  = 0;
        cur.ln1 += 1;
        text_buf_setcur(text_cur_buf, &cur);

        cli_line(text_cur_buf, cn, ln);
        cli_lines_after(text_cur_buf, ln + 1);
    }
}

void cmd_handle_key(cli_key key)
{
    switch (key)
    {
    case cli_key_up:   case cli_key_down:
    case cli_key_left: case cli_key_right:
        cmd_ins_flush();
        cmd_arrow(key);
        break;
    
    case cli_key_del: case cli_key_back:
        cmd_ins_flush();
        cmd_del(key);
        break;

    case ('A' | cli_key_ctrl):
        cmd_ins_flush();
        cmd_swap(key);
        break;

    case ('X' | cli_key_ctrl):
        cli_alive = 0;
        break;

    case cli_key_enter:
        cmd_ins_flush();
        cmd_enter(key);
        break;

    default:
        if (key < 0x100) cmd_ins(key);
   
   break;
    }
}

