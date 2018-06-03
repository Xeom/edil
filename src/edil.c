#if !defined(_POSIX_C_SOURCE)
# define _POSIX_C_SOURCE 1
# include <stdio.h>
# undef _POSIX_C_SOURCE
#else
# include <stdio.h>
#endif

#include "bind.h"
#include "ring.h"
#include "win.h"
#include "out.h"
#include "inp.h"
#include "ui.h"
#include "cmd.h"
#include "cmd/conf.h"
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

static char *welcome = "\n"
"          Welcome, to           .\n"
"       _____   ___      __   __\n"
"      / ___/  / _ \\    / /  / /\n"
"     / /_    / / | |  / /  / /\n"
"    / __/   / / / /  / /  / /\n"
"   / /__   / /_/ /  / /  / /_\n"
"  /____/  /_____/  /_/  /____/\n"
"                      v" STRIFY(VERSION) "\n\n"

"     My text editor,\n"
"          by Francis Wharf\n\n"
"    - Copyright 2017\n";

static char *help = "\n"
"Edil - My text editor. v" STRIFY(VERSION) "\n\n"

"More complete documentation can be found in the README.md\n"
"file, and in the doc/ directory of my repository.\n"
"This should be availible at http://github.com/Xeom/edil-2.0.\n\n"

"Usage: edil [options ...] [files ...]\n\n"

"Options:\n"
"    --cmd      -c    Specify a command to run on startup.\n"
"                       e.g. edil --cmd \"goto 1 1\"\n"
"                     will move the cursor to the start of the\n"
"                     file when edil starts up.\n"
"    --help     -h    Display this help message.\n"
"    --version  -v    Show the current version of edil, and\n"
"                     its compilation time\n"
"    --binds    -b    Display edil's keybindings\n\n";

static char *version =
"Edil v" STRIFY(VERSION) ", -- Compiled (" STRIFY(COMPILETIME) ")\n";

#define argerror(error) #error ".\n" \
    "Try 'edil --help' for usage information.\n"

static void load_string(win *w, char *str);
static void init_all(void);
static void kill_all(void);
static void loop(void);
static void process_arg(int argc, char **argv, int *n);

static vec *new_startup_cmd(void);
static void run_startup_cmd(void);

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
    else if (argis(--cmd) || argis(-c))
    {
        vec *cmd;
        *n += 1;
        if (*n >= argc)
        {
            fputs(argerror('--cmd' needs an argument), stdout);
            exit(0);
        }

        cmd = new_startup_cmd();
        chr_from_str(cmd, argv[*n]);
    }
    else if (argis(--binds) || argis(-b))
    {
        inp_init();
        bind_init();
        bind_print(stdout);
        fflush(stdout);
        bind_kill();
        inp_kill();
        exit(0);
    }
    else if (argis(--filecmd) || argis(-f))
    {
        vec *cmd;
        cmd = new_startup_cmd();

        *n += 1;
        if (*n >= argc)
        {
            fputs(argerror('--filecmd' needs an argument), stdout);
            exit(0);
        }

        chr_format(cmd, "conffile \"%s\"", argv[*n]);
    }
    else if (strncmp(argv[*n], "-", 1) == 0)
    {
        fputs(argerror(Unknown argument), stdout);
        exit(0);
    }
    else
    {
        vec *cmd;
        cmd = new_startup_cmd();
        chr_format(cmd, "new \"%s\"", argv[*n]);
    }

    *n += 1;
}

vec startup_cmd;

void run_startup_cmd(void)
{
    size_t ind, len;

    conf_run_default_files(win_cur);

    len = vec_len(&startup_cmd);
    for (ind = 0; ind < len; ++ind)
    {
        vec *cmd;
        cmd = vec_get(&startup_cmd, ind);
        ui_cmd_cb(win_cur, cmd);

        vec_kill(cmd);
    }


    vec_kill(&startup_cmd);
}

vec *new_startup_cmd(void)
{
    vec *cmd;
    cmd = vec_app(&startup_cmd, NULL);
    vec_init(cmd, sizeof(chr));

    return cmd;
}

static void init_all(void)
{
    ring_init();
    cmd_init();
    out_init(stdout);
    inp_init();
    bind_init();
}

static void kill_all(void)
{
    bind_kill();
    inp_kill();
    out_kill(stdout);
    cmd_kill();
    ring_kill();
}

static void load_string(win *w, char *str)
{
    int fds[2];
    file f;

    file_clr_win(w);

    if (pipe(fds) != 0)
        return;

    if (write(fds[1], str, strlen(str)) != (int)strlen(str))
        return;

    close(fds[1]);

    file_init_pipe(&f, fdopen(fds[0], "r"));
    file_load(&f, w->b);
    file_close(&f);
}

static void loop(void)
{
    while (ui_alive)
    {
        fflush(stdout);
        inp_wait();

        /* Update the entire screen */
        if (out_to_resize)
        {
            out_to_resize = 0;
            win_cur->cols = out_cols;
            win_cur->rows = out_rows - 1;
            out_goto(0, out_rows, stdout);
            out_clr_line(stdout);
            win_out_all();
        }
    }
}

static void colour_edil(buf *b)
{
    size_t ln;

    col_desc textfnt = { .fg = col_red | col_bright, .bg = col_null };
    col_desc linefnt = { .fg = col_red, .bg = col_null, .set = col_under };
    col_desc copyfnt = { .fg = col_black | col_bright, .bg = col_null };


    for (ln = 2; ln < 8; ln++)
    {
        cur loc = { .ln = ln };
        buf_setcol(b, loc, buf_line_len(b, loc), textfnt);
    }

    buf_setcol(b, (cur){ .cn = 8, .ln = 1 }, 25, linefnt);
    buf_setcol(b, (cur){ .cn = 1, .ln = 8 }, 27, linefnt);

    buf_setcol(b, (cur){ .ln = 13 }, 20, copyfnt);
}

int main(int argc, char **argv)
{
    buf *b;
    win w;
    int argind;

    argind = 1;

    vec_init(&startup_cmd, sizeof(vec));

    while (argind < argc)
        process_arg(argc, argv, &argind);

    init_all();

    b = ring_new();
    buf_set_name(b, "'welcome'");
    b->flags |= buf_readonly | buf_nofile | buf_nokill;

    win_init(&w, b);

    win_cur = &w;

    load_string(&w, welcome);
    colour_edil(b);
    w.cols = out_cols;
    w.rows = out_rows - 1;

    w.pri = (cur){0, 14};
    w.sec = (cur){0, 14};

    run_startup_cmd();

    win_out_after(&w, (cur){0, 0});

    loop();

    win_kill(&w);
    kill_all();
}
