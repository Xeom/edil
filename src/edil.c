#if !defined(_POSIX_C_SOURCE)
# define _POSIX_C_SOURCE 1
# include <stdio.h>
# undef _POSIX_C_SOURCE
#else
# include <stdio.h>
#endif

#include "win.h"
#include "out.h"
#include "inp.h"
#include "con.h"
#include "cmd.h"
#include "cmd/file.h"

#include <string.h>

#define STRIFY2(...) #__VA_ARGS__
#define STRIFY(...) STRIFY2(__VA_ARGS__)

#if !defined(VERSION)
# define VERSION 0.0.0
#endif

#if !defined(COMPILETIME)
# define COMPILETIME never
#endif

static char *welcome =
"      __ Welcome, to ___________\n"
"      _____   ___      __   __  \n"
"     / ___/  / _ \\    / /  / / \n"
"    / /_    / / | |  / /  / /   \n"
"   / __/   / / / /  / /  / /    \n"
"  / /__   / /_/ /  / /  / /__   \n"
" /____/  /_____/  /_/  /____/   \n"
"______________________ v" STRIFY(VERSION) "\n\n"

"My text editor,\n"
"    by Francis Wharf\n\n"
"    - Copyright 2017\n";

static char *help =
"Edil - My text editor. v" STRIFY(VERSION) "\n\n";

static char *version =
"Edil v" STRIFY(VERSION) " Compiled " STRIFY(COMPILETIME) "\n";

static void load_string(win *w, char *str);
static void load_string(win *w, char *str)
{
    int fds[2];
    pipe(fds);

    write(fds[1], str, strlen(str));
    close(fds[1]);

    file_load_win(w, fdopen(fds[0], "r"));
}


int main(void)
{
    buf b;
    win w;

    inp_init();
    out_init(stdout);
    buf_init(&b);
    win_init(&w, &b);
    con_init();
    cmd_init();

    win_cur = &w;

    load_string(&w, welcome);

    w.cols = out_cols;
    w.rows = out_rows - 1;

    w.pri = (cur){0, 13};
    w.sec = (cur){0, 13};

    win_out_after(&w, (cur){0, 0}, stdout);
    fflush(stdout);

    while (con_alive)
    {
        inp_wait();
        fflush(stdout);
    }

    out_kill(stdout);
    inp_kill();
    con_kill();
    cmd_kill();
    win_kill(&w);

    buf_kill(&b);
}
