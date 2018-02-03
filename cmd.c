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
    if (cmd_cur_mode == cmd_mode_buf)
    {
        switch (key)
        {
        case cli_key_up:    text_buf_move_cur(text_cur_buf,  0, -1, 0, 0); break;
        case cli_key_down:  text_buf_move_cur(text_cur_buf,  0,  1, 0, 0); break;
        case cli_key_left:  text_buf_move_cur(text_cur_buf, -1,  0, 0, 0); break;
        case cli_key_right: text_buf_move_cur(text_cur_buf,  1,  0, 0, 0); break;
        }
    }      
}

vec cmd_ins_buf;

void cmd_init(void)
{
    vec_init(&cmd_ins_buf, 1);
}

void cmd_ins(cli_key key)
{
    static char utf8[8];
    static int  utf8ind = 0;
    static int  width   = 0;

    utf8[utf8ind] = key & 0xff;

    if (utf8ind == 0) width = text_utf8_len(utf8[0]);
    utf8ind++;

    if (utf8ind == width)
    {
        utf8ind = 0;
        vec_ins(&cmd_ins_buf, vec_len(&cmd_ins_buf), width, utf8);
    }
}

void cmd_ins_flush(void)
{
    text_cur *cur;
    vec      *v;
    size_t    len;

    cur = &(text_cur_buf->cur);
    v   = &cmd_ins_buf;
    len = vec_len(v);
    if (len == 0) return;
  
    vec_ins(v, len, 1, "\0");
    text_buf_ins(text_cur_buf, cur->cn1, cur->ln1, vec_get(v, 0));

    if (cur->ln1 == cur->ln2 && cur->cn2 >= cur->cn1)
        text_buf_move_cur(text_cur_buf, len, 0, len, 0);
    else
        text_buf_move_cur(text_cur_buf, len, 0, 0, 0); 

    vec_del(v, 0, len + 1);
}

void cmd_handle_key(cli_key key)
{
    if (key == cli_key_up 
     || key == cli_key_down 
     || key == cli_key_left 
     || key == cli_key_right)
        cmd_arrow(key);

    else if (key == ('X' | cli_key_ctrl))
        cli_alive = 0;

    else if (key < 0x100)
        cmd_ins(key);
}
