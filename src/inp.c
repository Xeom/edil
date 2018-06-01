#if !defined(_GNU_SOURCE)
# define _GNU_SOURCE
# include <fcntl.h>
# include <stdio.h>
# undef _GNU_SOURCE
#else
# include <fcntl.h>
# include <stdio.h>
#endif

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

void inp_nonblockify(int fd);

pthread_t inp_listen_thread = 0;
static int inp_fd_in;
static int inp_fd_out;

vec inp_keycodes;

inp_keycode inp_keycodes_static[] =
{
    {inp_key_tab,    "",     "tab"},
    {inp_key_back,   "",     "backspace"},
    {inp_key_up,     "[A",   "up"},
    {inp_key_down,   "[B",   "down"},
    {inp_key_right,  "[C",   "right"},
    {inp_key_left,   "[D",   "left"},
    {inp_key_home,   "[1~",  "home"},
    {inp_key_del,    "[3~",  "del"},
    {inp_key_end,    "[4~",  "end"},
    {inp_key_pgup,   "[5~",  "pgup"},
    {inp_key_pgdn,   "[6~",  "pgdn"},
    {inp_key_insert, "[4h",  "insert"},
    {inp_key_f1,     "OP",   "f1"},
    {inp_key_f2,     "OQ",   "f2"},
    {inp_key_f3,     "OR",   "f3"},
    {inp_key_f4,     "OS",   "f4"},
    {inp_key_f5,     "[15~", "f5"},
    {inp_key_f6,     "[17~", "f6"},
    {inp_key_f7,     "[18~", "f7"},
    {inp_key_f8,     "[19~", "f8"},
    {inp_key_f9,     "[20~", "f9"},
    {inp_key_f10,    "[21~", "f10"},
    {inp_key_f11,    "[23~", "f11"},
    {inp_key_f12,    "[24~", "f12"},
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

    return rtn;
}

static char *inp_key_basename(inp_key key)
{
    size_t len, ind;
    len = vec_len(&inp_keycodes);

    key &= ~(inp_key_ctrl | inp_key_esc);

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

    prefix = "";
    name   = NULL;

    if ((key & inp_key_ctrl) && (key & inp_key_esc))
        prefix = "Ctrl+Esc+";
    else if (key & inp_key_ctrl)
        prefix = "Ctrl+";
    else if (key & inp_key_esc)
        prefix = "Esc+";

    name = inp_key_basename(key);

    if (!name && (key & 0xff) > 0x20 && (key & 0xff) < 0x7e)
    {
        chrname[0] = key & 0xff;
        chrname[1] = '\0';
        name       = chrname;
    }

    if (name)
        snprintf(str, len, "%03x %s%s ", key, prefix, name);
    else
        snprintf(str, len, "%03x %s%02x ", key, prefix, key & 0xff);
}

static void inp_listen_term(int sign)
{
    pthread_exit(NULL);
}

static void *inp_listen(void *arg)
{
    struct sigaction act;

    act.sa_handler = inp_listen_term;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    sigaction(SIGTERM, &act, NULL);

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

void inp_empty_pipe(void)
{
    inp_key key;

    while (read(inp_fd_out, &key, sizeof(inp_key)) != -1)
        bind_handle_key(key);

    bind_flush();
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

    /* Create the input listener thread */
    pthread_create(&inp_listen_thread, NULL, inp_listen, NULL);
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
    vec_kill(&inp_keycodes);
    pthread_kill(inp_listen_thread, SIGTERM);
    pthread_join(inp_listen_thread, NULL);
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