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
#include "text/col.h"
#include "text/cur.h"
#include "ui.h"
#include "cmd.h"
#include "updater.h"
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

static char *welcome[] =
{
    "",
    "%1    ___________________________",
    "%1   / %11_______ %10_______   %12___ %13___ %1\\",
    "%1  / %11/\\   ___\\%10\\   _  ',%12/\\  \\%13\\  \\ %1\\",
    "%1  \\ %11\\ \\  \\____%10\\  \\ ',  \\%12\\  \\%13\\  \\ %1\\",
    "%1   \\ %11\\ \\   ___\\%10\\  \\  \\  \\%12\\  \\%13\\  \\ %1\\____",
    "%1    \\ %11\\ \\  \\____%10\\  '_'  |%12 \\  \\%13\\  \\____ %1\\",
    "%1     \\ %11\\ \\______\\%10\\_____/%12 \\ \\__\\%13\\______\\ %1\\",
    "%1      \\ %11\\/______/%10/____/%1 _%12 \\/__/%13/______/ %1/",
    "%1       \\_______________/ \\_____________/",
    "%8        v" STRIFY(VERSION),
    "          %6.--------------------.",
    "          %6\\%16 My Editor,          %6\\",
    "           %6\\%16    By %15Francis Wharf %6\\",
    "            %6'--------------------'",
    "             %8- Copyright 2018",
    "",
    " For help, please consult the %15README.md%16 file",
    " included in the repository. The %15docs/%16 directory",
    " in the repository includes various files that",
    " document %3E%2d%4i%5l%15's%16 features.",
    "",
    " The file %15docs/editing.md%16 in particular provides",
    " a quick guide to starting using %3E%2d%4i%5l%16 as an editor.",
    "",
    " Press %15Ctrl%16+%15Esc%16+%15K%16 and then %15enter%16 to exit, or",
    " press %15Ctrl%16+%15X%16, type %11\"quit !\"%16 and press %15enter%16."
};

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
"    --binds    -b    Display edil's keybindings\n"
"    --filecmd  -f    Specify a filename, that commands are\n"
"                     then loaded from and run on startup.\n"
"    --keycode  -kc   Specify a hexadecimal key value, and\n"
"                     an appropriate pretty name is printed\n"
"    --cmds     -cl   List all edil commands and print them\n"
"                     out in a markdown format.\n\n";

static char *version =
"Edil v" STRIFY(VERSION) ", -- Compiled (" STRIFY(COMPILETIME) ")\n";

#define argerror(error) #error ".\n" \
    "Try 'edil --help' for usage information.\n"

static void load_lines(win *w, int n, char **str);
static void init_all(void);
static void kill_all(void);
static void loop(void);
static int  process_arg(int argc, char **argv, int *n);

static vec *new_startup_cmd(void);
static void run_startup_cmd(void);

#define argis(a) (strcmp(argv[*n], #a) == 0)

static int process_arg(int argc, char **argv, int *n)
{
    if (argis(--help) || argis(-h))
    {
        fputs(help, stdout);
        return 0;
    }
    else if (argis(--version) || argis(-v))
    {
        fputs(version, stdout);
        return 0;
    }
    else if (argis(--cmd) || argis(-c))
    {
        vec *cmd;
        *n += 1;
        if (*n >= argc)
        {
            fputs(argerror('--cmd' needs an argument), stderr);
            return 0;
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
        return 0;
    }
    else if (argis(--cmds) || argis(-cl))
    {
        cmd_init();
        cmd_print_all(stdout);
        cmd_kill();
        return 0;
    }
    else if (argis(--filecmd) || argis(-f))
    {
        vec *cmd;
        cmd = new_startup_cmd();

        *n += 1;
        if (*n >= argc)
        {
            fputs(argerror('--filecmd' needs an argument), stderr);
            return 0;
        }

        chr_format(cmd, "conffile \"%s\"", argv[*n]);
    }
    else if (argis(--keycode) || argis(-kc))
    {
        char name[64];
        unsigned int intkey;
        inp_key key;

        *n += 1;
        if (*n >= argc)
        {
            fputs(argerror('--keycode' needs an argument), stderr);
            return 0;
        }

        if (sscanf(argv[*n], "%x", &intkey) != 1)
        {
            fputs(argerror('--keycode' takes a hexadecimal number), stderr);
            return 0;
        }

        key = intkey;

        inp_init();
        inp_key_name(key, name, sizeof(name));
        inp_kill();

        puts(name);

        return 0;
    }
    else if (strncmp(argv[*n], "-", 1) == 0)
    {
        fputs(argerror(Unknown argument), stdout);
        return 0;
    }
    else
    {
        vec *cmd;
        cmd = new_startup_cmd();
        chr_format(cmd, "new \"%s\"", argv[*n]);
    }

    *n += 1;

    return 1;
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
    updater_init();
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

static void load_lines(win *w, int n, char **str)
{
    vec chrs;
    int ind;

    vec_init(&chrs, sizeof(chr));

    buf_clr(w->b);
    win_reset(w);

    w->pri = (cur){0, 0};

    for (ind = 0; ind < n; ++ind)
    {
        vec_clr(&chrs);

        col_parse_string(
            (col){ .fg = col_none, .bg = col_none },
            &chrs,
            str[ind]
        );

        cur_ins_win(w, &chrs);
        cur_enter_win(w);
    }

    vec_kill(&chrs);
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

int main(int argc, char **argv)
{
    buf *b;
    win w;
    int argind;

    argind = 1;

    vec_init(&startup_cmd, sizeof(vec));

    while (argind < argc)
        if (process_arg(argc, argv, &argind) == 0)
            return 0;

    init_all();

    b = ring_new();
    buf_set_name(b, "'welcome'");

    win_init(&w, b);

    win_cur = &w;

    w.cols = out_cols;
    w.rows = out_rows - 1;

    load_lines(&w, sizeof(welcome)/sizeof(char *), welcome);
    b->flags |= buf_readonly | buf_nofile | buf_nokill;
    b->flags &= ~buf_modified;

    run_startup_cmd();

    updater_after(w.b, (cur){0, 0});

    loop();

    win_kill(&w);
    kill_all();
}
