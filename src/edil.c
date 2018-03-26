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
#include "ui.h"
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
"         Welcome, to            \n"
"      _____   ___      __   __  \n"
"     / ___/  / _ \\    / /  / /  \n"
"    / /_    / / | |  / /  / /   \n"
"   / __/   / / / /  / /  / /    \n"
"  / /__   / /_/ /  / /  / /__   \n"
" /____/  /_____/  /_/  /____/   \n"
"                     v" STRIFY(VERSION) "            \n\n"

" My text editor,\n"
"     by Francis Wharf\n\n"
"    - Copyright 2017\n";

static char *help = "\n"
"Edil - My text editor. v" STRIFY(VERSION) "\n\n"

"More complete documentation can be found in the README.md \n"
"file, available in my repo at http://github.com/Xeom/edil.\n\n"

"Options:\n"
"    --help     -h    Display this help message.\n"
"    --version  -v    Show the current version of edil, and\n"
"                     its compilation time\n";

static char *version =
"Edil v" STRIFY(VERSION) ", -- Compiled (" STRIFY(COMPILETIME) ")\n";

static char *argerror =
"Unknown argument.\n"
"Try 'edil --help' for usage information.\n";

static void load_string(win *w, char *str);
static void init_all(void);
static void kill_all(void);
static void loop(void);
static void process_arg(int argc, char **argv, int *n);

#define argis(a) (strcmp(argv[*n], #a) == 0)

static void process_arg(int argc, char **argv, int *n)
{
    if (argis(--help) || argis(-h))
    {
        fputs(help, stdout);
        exit(0);
    }

    else if (argis(--version) || argis(-v))
    {
        fputs(version, stdout);
        exit(0);
    }

    else
    {
        fputs(argerror, stdout);
        exit(0);
    }

    *n += 1;
}

static void init_all(void)
{
    out_init(stdout);
    inp_init();
    ui_init();
    cmd_init();
}

static void kill_all(void)
{
    out_kill(stdout);
    inp_kill();
    ui_kill();
    cmd_kill();
}

static void load_string(win *w, char *str)
{
    int fds[2];

    if (pipe(fds) != 0)
        return;

    if (write(fds[1], str, strlen(str)) != (int)strlen(str))
        return;

    close(fds[1]);

    file_load_win(w, fdopen(fds[0], "r"));
}

static void loop(void)
{
    while (ui_alive)
    {
        fflush(stdout);
        inp_wait();
    }
}

static void colour_edil(buf *b)
{
    size_t ln;
    col_desc textfnt = { .fg = col_cyan, .bg = col_null };
    col_desc linefnt = { .fg = col_cyan | col_bright, .bg = col_null, .set = col_under };
    col_desc copyfnt = { .fg = col_black | col_bright, .bg = col_null };

    for (ln = 1; ln < 7; ln++)
    {
        cur loc = { .ln = ln };
        buf_setcol(b, loc, buf_line_len(b, loc), textfnt);
    }

    buf_setcol(b, (cur){ .cn = 7 }, 25, linefnt);
    buf_setcol(b, (cur){ .ln = 7 }, 27, linefnt);

    buf_setcol(b, (cur){ .ln = 12 }, 20, copyfnt);

    b->flags |= buf_readonly;
}

int main(int argc, char **argv)
{
    buf b;
    win w;
    int argind;

    argind = 1;

    while (argind < argc)
        process_arg(argc, argv, &argind);

    buf_init(&b);
    win_init(&w, &b);
    init_all();

    win_cur = &w;

    load_string(&w, welcome);
    colour_edil(&b);

    w.cols = out_cols;
    w.rows = out_rows - 1;

    w.pri = (cur){0, 13};
    w.sec = (cur){0, 13};

    win_out_after(&w, (cur){0, 0}, stdout);
    win_out_bar(&w, stdout);

    loop();

    win_kill(&w);
    buf_kill(&b);
    kill_all();
}
