#if !defined(_GNU_SOURCE)
# define _GNU_SOURCE
# include <fcntl.h>
# include <stdio.h>
# undef _GNU_SOURCE
#else
# include <fcntl.h>
# include <stdio.h>
#endif

#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>

#include "bind.h"
#include "ui.h"
#include "vec.h"
#include "out.h"

#include "inp.h"

static void *inp_listen(void *arg);
static char *inp_key_basename(inp_key key);

static inp_key inp_translate_key(inp_key key);
static void    inp_load_keytranslate(void);
static void    inp_load_keycodes(void);

static void inp_listen_init(void);
static void inp_listen_kill(void);

void inp_nonblockify(int fd);

static int inp_fd_in;
static int inp_fd_out;

pthread_t inp_listen_thread = 0;

vec inp_keycodes;
table inp_keytranslate;

inp_keycode inp_keycodes_static[] =
{
    {inp_key_tab,  "", "tab"},
    {inp_key_back, "", "backspace"},
    {inp_key_shiftback, "", "Shift+backspace"},
    {inp_key_enter, "", "enter"},
    {' ',           "", "space"},

    {inp_key_up,    "[A", "up"},
    {inp_key_down,  "[B", "down"},
    {inp_key_right, "[C", "right"},
    {inp_key_left,  "[D", "left"},

    {inp_key_home,     "[1~", "home"},
    {inp_key_del,      "[3~", "del"},
    {inp_key_end,      "[4~", "end"},
    {inp_key_pgup,     "[5~", "pgup"},
    {inp_key_pgdn,     "[6~", "pgdn"},
    {inp_key_insert,   "[4h", "insert"},
    {inp_key_shifttab, "[Z",  "Shift+tab"},
    {inp_key_shiftdel, "[2K", "Shift+del"},

    {inp_key_f1,  "OP",    "f1"},
    {inp_key_f2,  "OQ",    "f2"},
    {inp_key_f3,  "OR",    "f3"},
    {inp_key_f4,  "OS",    "f4"},
    {inp_key_f5,  "[15~",  "f5"},
    {inp_key_f6,  "[17~",  "f6"},
    {inp_key_f7,  "[18~",  "f7"},
    {inp_key_f8,  "[19~",  "f8"},
    {inp_key_f9,  "[20~",  "f9"},
    {inp_key_f10, "[21~",  "f10"},
    {inp_key_f11, "[23~",  "f11"},
    {inp_key_f12, "[24~",  "f12"},
};

inp_key inp_keytranslate_static[][2] =
{
    {inp_key_ctrl | 'J', inp_key_enter},
    {inp_key_ctrl | 'H', inp_key_shiftback},
    {inp_key_ctrl | 'I', inp_key_tab},
    {'\177',             inp_key_back},
    {inp_key_esc | inp_key_ctrl | 'J', inp_key_esc | inp_key_enter},
    {inp_key_esc | inp_key_ctrl | 'H', inp_key_esc | inp_key_shiftback},
    {inp_key_esc | inp_key_ctrl | 'I', inp_key_esc | inp_key_tab},
    {inp_key_esc | '\177',             inp_key_esc | inp_key_back}
};

static int inp_keycode_cmp(const void *a, const void *b)
{
    const inp_keycode *acode, *bcode;

    acode = a;
    bcode = b;

    return strcmp(acode->code, bcode->code);
}

static inp_key inp_get_escaped_key(unsigned char chr)
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
        else      cmp = 1;

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

inp_key inp_get_key(unsigned char c)
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

    rtn = inp_translate_key(rtn);

    return rtn;
}

static char *inp_key_basename(inp_key key)
{
    size_t len, ind;
    len = vec_len(&inp_keycodes);

    for (ind = 0; ind < len; ind++)
    {
        inp_keycode *code;
        code = vec_get(&inp_keycodes, ind);

        if (code && code->key == key)
            return code->name;
    }

    return NULL;
}

void inp_key_name(inp_key key, char *str, size_t len)
{
    char *prefix, *name;
    char chrname[2];
    inp_key keymods;

    prefix = "";
    name   = NULL;

    keymods = inp_key_esc;
    if ((name = inp_key_basename(key & ~keymods)))
        keymods &= key;

    if (!name)
    {
        keymods = inp_key_esc | inp_key_ctrl;
        if ((name = inp_key_basename(key & ~keymods)))
            keymods &= key;
    }

    if ((key & inp_key_ctrl) && (key & inp_key_esc))
        prefix = "Ctrl+Esc+";
    else if (key & inp_key_ctrl)
        prefix = "Ctrl+";
    else if (key & inp_key_esc)
        prefix = "Esc+";

    if (!name && (key & 0xff) > 0x20 && (key & 0xff) < 0x7e)
    {
        chrname[0] = key & 0xff;
        chrname[1] = '\0';
        name       = chrname;
    }

    if (name)
        snprintf(str, len, "%03x %s%s", key, prefix, name);
    else
        snprintf(str, len, "%03x %s0x%02x", key, prefix, key & 0xff);
}

void inp_empty_pipe(void)
{
    inp_key key;

    while (read(inp_fd_out, &key, sizeof(inp_key)) != -1)
        bind_handle_key(key);

    bind_flush();
    /* Flush */
}

static void inp_load_keycodes(void)
{
    size_t num;

    /* Load the keycodes */
    num = sizeof(inp_keycodes_static) / sizeof(inp_keycode);

    vec_init(&inp_keycodes, sizeof(inp_keycode));
    vec_ins(&inp_keycodes, 0, num, &inp_keycodes_static);
    vec_sort(&inp_keycodes, inp_keycode_cmp);
}

static inp_key inp_translate_key(inp_key key)
{
    inp_key *ptr;

    ptr = table_get(&inp_keytranslate, &key);

    if (ptr)
        return *ptr;
    else
        return  key;
}

static void inp_load_keytranslate(void)
{
    size_t ind, num;

    num = sizeof(inp_keytranslate_static) / sizeof(inp_key[2]);

    table_init(&inp_keytranslate, sizeof(inp_key), sizeof(inp_key));

    for (ind = 0; ind < num; ++ind)
    {
        inp_key from, to;
        from = inp_keytranslate_static[ind][0];
        to   = inp_keytranslate_static[ind][1];
        table_set(&inp_keytranslate, &from, &to);
    }
}

void inp_init(void)
{
    int pipefds[2];

    inp_load_keycodes();
    inp_load_keytranslate();

    if (pipe(pipefds) == -1)
    {
        printf(
            "Could not open input pipes: [%d] '%s'\n",
            errno, strerror(errno)
        );

        exit(-1);
    }

    inp_fd_in  = pipefds[1];
    inp_fd_out = pipefds[0];

    inp_nonblockify(inp_fd_in);
    inp_nonblockify(inp_fd_out);

    inp_listen_init();
}

void inp_nonblockify(int fd)
{
    int rtn, flags;
    rtn = fcntl(fd, F_GETFL);

    if (rtn == -1)
    {
        printf(
            "Could not get inp pipe flags: [%d] '%s'\n",
            errno, strerror(errno)
        );

        exit(-1);
    }

    flags = rtn | O_NONBLOCK;
    rtn   = fcntl(fd, F_SETFL, flags);

    if (rtn == -1)
    {
        printf(
            "Could not set inp pipe flags: [%d] '%s'\n",
            errno, strerror(errno)
        );

        exit(-1);
    }
}

void inp_kill(void)
{
    inp_listen_kill();

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

int inp_key_cmp(const void *aptr, const void *bptr)
{
    inp_key a, b;
    int basea, baseb;

    a = *(inp_key *)aptr;
    b = *(inp_key *)bptr;

    basea = 0x1ff & a;
    baseb = 0x1ff & b;

    if (basea > baseb) return 1;
    if (basea < baseb) return -1;

    if (a > b) return 1;
    if (a < b) return -1;

    else return 0;
}

/*
 * INPUT THREAD
 */

static void *inp_listen(void *arg)
{
    while (1)
    {
        int chr;
        inp_key key;

        chr = getchar();
        key = inp_get_key(chr);

        if (key == inp_key_none) continue;

        if (write(inp_fd_in, &key, sizeof(inp_key)) != sizeof(inp_key))
        {
            printf(
                "Could not write input key to pipe: [%d] '%s'\n",
                errno, strerror(errno)
            );

            continue;
        }
    }

    return NULL;
}

static void inp_listen_kill(void)
{
    pthread_cancel(inp_listen_thread);
    pthread_join(inp_listen_thread, NULL);
}

static void inp_listen_init(void)
{
    /* Create the input listener thread */

    pthread_create(&inp_listen_thread, NULL, inp_listen, NULL);
}
