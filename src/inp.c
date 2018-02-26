#if !defined(_GNU_SOURCE)
# define _GNU_SOURCE
# include <fcntl.h>
# include <stdio.h>
# undef _GNU_SOURCE
#else
# include <fcntl.h>
# include <stdio.h>
#endif

#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

#include "con.h"
#include "vec.h"
#include "out.h"

#include "inp.h"

vec inp_keycodes;

inp_keycode inp_keycodes_static[] =
{
    {inp_key_up,     "[A"},
    {inp_key_down,   "[B"},
    {inp_key_right,  "[C"},
    {inp_key_left,   "[D"},
    {inp_key_home,   "[1~"},
    {inp_key_del,    "[3~"},
    {inp_key_end,    "[4~"},
    {inp_key_pgup,   "[5~"},
    {inp_key_pgdn,   "[6~"},
    {inp_key_insert, "[4h"},
    {inp_key_f1,     "OP"},
    {inp_key_f2,     "OQ"},
    {inp_key_f3,     "OR"},
    {inp_key_f4,     "OS"},
    {inp_key_f5,     "[15~"},
    {inp_key_f6,     "[17~"},
    {inp_key_f7,     "[18~"},
    {inp_key_f8,     "[19~"},
    {inp_key_f9,     "[20~"},
    {inp_key_f10,    "[21~"},
    {inp_key_f11,    "[23~"},
    {inp_key_f12,    "[24~"}
};

static int inp_keycode_cmp(const void *a, const void *b)
{
    return strcmp(((const inp_keycode *)a)->code, ((const inp_keycode *)b)->code);
}

static inp_key inp_get_escaped_key(char chr)
{
    static char code[16];
    static int  code_ind   = 0;
    static int  search_ind = 0;
    static int  rtn;

    code[code_ind] = chr;
    code[code_ind + 1] = '\0';
    code_ind += 1;

    while (code_ind <= 14)
    {
        int cmp;
        inp_keycode *curr;

        curr = vec_get(&inp_keycodes, search_ind);
        if (curr) cmp = strcmp(code, curr->code);

        if (cmp == 0) {rtn = curr->key;             break;}
        if (!curr)    {rtn = inp_key_esc | code[0]; break;}
        if (cmp >  0) search_ind++;
        if (cmp <  0)
        {
            if (memcmp(code, curr->code, code_ind) == 0)
                return 0;
            
            rtn = inp_key_esc | code[0];
            break;
        }
    }

    code_ind   = 0;
    search_ind = 0;

    return rtn;
}

inp_key inp_get_key(char chr)
{
    static int escaped = 0;
    inp_key    rtn;

    if (escaped)
    {    
        rtn = inp_get_escaped_key(chr);
        if (rtn == 0) return inp_key_none;
    }
    else if (chr == '\033')
    {
        escaped = 1;
        return inp_key_none;
    }
    else
        rtn = chr;

    escaped = 0;

    if ((rtn & ~inp_key_esc) < 32)
    {
        rtn += 0x40;
        rtn |= inp_key_ctrl;
    }

    return rtn;
}

pthread_t inp_listen_thread;
int inp_fd_in;
int inp_fd_out;

void *inp_listen(void *arg)
{
    while (1)
    {
        int chr;
        inp_key key;
   
        chr = getchar();
        key = inp_get_key(chr);

        if (key == inp_key_none) continue;

        write(inp_fd_in, &key, sizeof(inp_key));
    }
}


void inp_empty_pipe(void)
{
    inp_key key;
    
    while (read(inp_fd_out, &key, sizeof(inp_key)) != -1)
        con_handle(key);

    con_flush();
    /* Flush */
}

void inp_init(void)
{
    size_t numkeys;
    int pipefds[2];

    /* Load the keycodes */
    numkeys = sizeof(inp_keycodes_static) / sizeof(inp_keycode);

    vec_init(&inp_keycodes, sizeof(inp_keycode));
    vec_ins(&inp_keycodes, 0, numkeys, &inp_keycodes_static);
    vec_sort(&inp_keycodes, inp_keycode_cmp);

    pipe(pipefds);
    inp_fd_in  = pipefds[1];
    inp_fd_out = pipefds[0];

    fcntl(inp_fd_in,  F_SETFL, O_NONBLOCK | fcntl(inp_fd_in,  F_GETFL));
    fcntl(inp_fd_out, F_SETFL, O_NONBLOCK | fcntl(inp_fd_out, F_GETFL));

    /* Create the input listener thread */
    pthread_create(&inp_listen_thread, NULL, inp_listen, NULL);
}

void inp_kill(void)
{
    vec_kill(&inp_keycodes);
}

#include "win.h"
#include "out.h"
int main(void)
{
    inp_init();
    out_init(stdout);

    buf b;
    win w;
    vec text, str;

    vec_init(&str,  sizeof(char));
    vec_init(&text, sizeof(chr));

    vec_ins(&str, 0, 12, "Hello World!");
    chr_from_str(&text, &str);

    buf_init(&b);
    win_init(&w, &b);
    con_init();

    win_cur = &w;

    w.pri = cur_ins(w.pri, &b, &text);
    w.pri = cur_move(w.pri, &b, (cur){.cn = -3});
    w.pri = cur_enter(w.pri, &b);
    w.pri = cur_move(w.pri, &b, (cur){.ln = -1});
    out_init(stdout);

    w.cols = out_cols;
    w.rows = out_rows;

    win_out_after(&w, (cur){0, 0}, stdout);

    while (1)
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(inp_fd_out, &fds);
        select(inp_fd_out + 1, &fds, NULL, NULL, NULL);
        if (FD_ISSET(inp_fd_out, &fds))
            inp_empty_pipe();

        fflush(stdout);
    }

    return 0;
    out_kill(stdout);

    buf_kill(&b);

    vec_kill(&str);
    vec_kill(&text);
}
