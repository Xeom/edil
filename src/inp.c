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

static void *inp_listen(void *arg);

static pthread_t inp_listen_thread;
static int inp_fd_in;
static int inp_fd_out;

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
    {inp_key_f12,    "[24~"},
};

char *inp_key_names[] =
{
    [inp_key_back]   = "back",
    [inp_key_tab]    = "tab",
    [inp_key_enter]  = "enter",
    [inp_key_esc]    = "esc",
    [inp_key_ctrl]   = "ctrl",
    [inp_key_none]   = "none",
    [inp_key_up]     = "up",
    [inp_key_down]   = "down",
    [inp_key_right]  = "right",
    [inp_key_left]   = "left",
    [inp_key_home]   = "home",
    [inp_key_del]    = "del",
    [inp_key_end]    = "end",
    [inp_key_pgup]   = "pgup",
    [inp_key_pgdn]   = "pgdn",
    [inp_key_insert] = "insert",
    [inp_key_f1]     = "f1", 
    [inp_key_f2]     = "f2",
    [inp_key_f3]     = "f3",
    [inp_key_f4]     = "f4",
    [inp_key_f5]     = "f5",
    [inp_key_f6]     = "f6",
    [inp_key_f7]     = "f7",
    [inp_key_f8]     = "f8",
    [inp_key_f9]     = "f9",
    [inp_key_f10]    = "f10",
    [inp_key_f11]    = "f11",
    [inp_key_f12]    = "f12"
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
    static int  double_esc = 0;

    if (chr == '\033' && code_ind == 0 && !double_esc)
    {
        double_esc = 1;
        return 0;
    }

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

    if (double_esc)
        rtn |= inp_key_esc;

    code_ind   = 0;
    search_ind = 0;
    double_esc = 0;

    return rtn;
}

inp_key inp_get_key(char c)
{
    static int escaped = 0;
    inp_key    rtn;

    if (escaped)
    {    
        rtn = inp_get_escaped_key(c);
        if (rtn == 0) return inp_key_none;
    }
    else if (c == '\033')
    {
        escaped = 1;
        return inp_key_none;
    }
    else
        rtn = c;

    escaped = 0;

    if ((rtn & ~inp_key_esc) < 32)
    {
        rtn += 0x40;
        rtn |= inp_key_ctrl;
    }

    return rtn;
}

void inp_key_name(inp_key key, char *str, size_t len)
{
    char *prefix, *name;
    char chrname[2];
    
    prefix = "";
    name   = NULL;
    size_t numnames;

    numnames = sizeof(inp_key_names)/sizeof(char *);

    if (inp_key_names[key] && key < numnames)
        name = inp_key_names[key];

    else
    {
        inp_key basekey;
        
        if ((key & inp_key_ctrl) && (key & inp_key_esc))
            prefix = "Ctrl+Esc+";
        else if (key & inp_key_ctrl)
            prefix = "Ctrl+";
        else if (key & inp_key_esc)
            prefix = "Esc+";

        basekey = key & ~(inp_key_ctrl | inp_key_esc);

        if (inp_key_names[basekey] && key < numnames)
            name = inp_key_names[basekey];
    }

    if (!name && (key & 0xff) > 0x20 && (key & 0xff) < 0x7e)
    {
        chrname[0] = key & 0xff;
        chrname[1] = '\0';
        name       = chrname;
    }

    if (name)
        snprintf(str, len, "(%03x %s%s) ", key, prefix, name);
    else
        snprintf(str, len, "(%03x %s%02x) ", key, prefix, key & 0xff);
}

static void *inp_listen(void *arg)
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

    return NULL;
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

void inp_wait(void)
{
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(inp_fd_out, &fds);

    select(inp_fd_out + 1, &fds, NULL, NULL, NULL);

    if (FD_ISSET(inp_fd_out, &fds))
        inp_empty_pipe();
}
