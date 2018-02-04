#include <string.h>

#include "cli.h"
#include "vec.h"

#include "cmd.h"

typedef enum
{
    cmd_mode_bar,
    cmd_mode_buf,
    cmd_mode_opt
} cmd_move;

cmd_move cmd_cur_mode = cmd_mode_buf;

void cmd_arrow(cli_key key)
{
    text_cur orig, *cur;
    cur = &(text_cur_buf->cur);

    memcpy(&orig, cur, sizeof(text_cur));

    if (cmd_cur_mode == cmd_mode_buf)
    {
        switch (key)
        {
        case cli_key_up:    cur->ln1 -= 1; break;
        case cli_key_down:  cur->ln1 += 1; break;
        case cli_key_left:  cur->cn1 -= 1; break;
        case cli_key_right: cur->cn1 += 1; break;
        }
    }

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
    text_cur *cur;
    vec      *v;
    size_t    len;
    size_t    cn;

    cur = &(text_cur_buf->cur);
    len = vec_len(v);
    if (len == 0) return;

    if (cmd_cur_mode == cmd_mode_buf)
    {
        text_cmd cmd;

        cn = cur->cn1;

        cmd.type = text_cmd_ins;
        cmd.args.ins.cn = cur->cn1;
        cmd.args.ins.ln = cur->ln1;
        memcpy(&(cmd.args.ins.chrs), &cmd_ins_buf, sizeof(vec));
 
        text_buf_cmd(text_cur_buf,         &cmd);
        text_cur_cmd(&(text_cur_buf->cur), &cmd);

        cli_line(text_cur_buf, cn, cur->ln1);
   }

   vec_del(v, 0, len);
} 

void cmd_del(cli_key key)
{
    text_cmd  cmd;
    text_cur *cur;
    cur = &(text_cur_buf->cur);
    
    if (cmd_cur_mode == cmd_mode_buf)
    {
        switch (key)
        {
        case cli_key_back:
            if (cur->cn1 == 0) return;
            cur->cn1 -= 1;
        case cli_key_del:   
            cmd.type = text_cmd_del;
            cmd.args.del.cn = cur->cn1;
            cmd.args.del.ln = cur->ln1;
            cmd.args.del.n  = 1;

            text_buf_cmd(text_cur_buf,         &cmd);
            text_cur_cmd(&(text_cur_buf->cur), &cmd);

            cli_line(text_cur_buf, cur->cn1, cur->ln1);
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
        size_t tmp;

        cur->cn1 = orig.cn2;
        cur->cn2 = orig.cn1;
        cur->ln1 = orig.ln2;
        cur->ln2 = orig.ln1;

        text_buf_update_cur(text_cur_buf, &orig);
    }
}

#include <stdio.h>
void cmd_enter(cli_key key)
{
    text_cur *cur;
    cur = &(text_cur_buf->cur);

    if (cmd_cur_mode == cmd_mode_buf)
    {
        text_cmd cmd;
        vec *v, *line;
        size_t len, cn, ln;

        cn = cur->cn1;
        ln = cur->ln1;

        v = &(cmd.args.ins.chrs);

        cmd.type = text_cmd_ins_line;
        cmd.args.ins_line.ln = ln + 1;
        text_buf_cmd(text_cur_buf, &cmd);
        text_cur_cmd(&(text_cur_buf->cur), &cmd);

        line = vec_get(&(text_cur_buf->lines), ln);
        len  = vec_len(line);

        if (len > cn)
        {
            cmd.type = text_cmd_ins;
            cmd.args.ins.ln = ln + 1;
            cmd.args.ins.cn = 0;
            vec_init(v, sizeof(text_char));

            vec_ins(v, 0, len - cn, vec_get(line, cn));
            text_buf_cmd(text_cur_buf, &cmd);
            text_cur_cmd(&(text_cur_buf->cur), &cmd);

            cmd.type = text_cmd_del;
            cmd.args.del.ln = ln;
            cmd.args.del.cn = cn;
            cmd.args.del.n  = len - cn;
            text_buf_cmd(text_cur_buf, &cmd);
            text_cur_cmd(&(text_cur_buf->cur), &cmd);
        }
 
        text_cur_buf->cur.cn1 = 0;
        text_cur_buf->cur.ln1 += 1;

        cli_line(text_cur_buf, cn, ln);
        cli_lines_after(text_cur_buf, ln + 1);
        
    }
}
void cmd_handle_key(cli_key key)
{
 //   fprintf(stderr, "%d\n", key);
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
        //cmd_ins_flush();    
   break;
    }
}
/*
   if (key == cli_key_up 
     || key == cli_key_down 
     || key == cli_key_left 
     || key == cli_key_right)
        cmd_arrow(key);

    else if (key == cli_key_del 
          || key == cli_key_back)
        cmd_del(key);

    else if (key == ('A' | cli_key_ctrl))
        cmd_swap(key);

    else if (key == ('X' | cli_key_ctrl))
        cli_alive = 0;

    else if (key == cli_key_enter)
        cmd_enter(key);

    else if (key < 0x100)
        cmd_ins(key);
}*/
