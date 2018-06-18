#if !defined(_POSIX_C_SOURCE)
# define _POSIX_C_SOURCE 199309L
# include <sys/types.h>
# include <signal.h>
# undef _POSIC_C_SOURCE
#else
# include <sys/types.h>
# include <signal.h>
#endif

#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>

#include "container/vec.h"
#include "text/chr.h"
#include "text/col.h"
#include "updater.h"
#include "indent.h"
#include "print.h"
#include "win.h"

#include "out.h"

#define GOTO(cn, ln) "\033[" #ln ";" #cn "H"
#define CLR_LINE   "\033[K"
#define CLR_SCREEN "\033[2J"
#define HIDE_CUR   "\033[?25l"
#define SHOW_CUR   "\033[?25h"
#define RESET_COL  "\033[0m"

static void out_init_thread(void);
static int out_updater_line(buf *b, cur *c);
static int out_updater_after(buf *b, cur *c);

static void out_handle_winch(int sign);

ssize_t out_cols, out_rows;
int out_to_resize;

static updater out_thread;

/* The colour and text to be printed when a line that doesn't exist *
 * is displayed (not just empty, a line beyond the end of the buf)  */
char *out_blank_line_text = "\xc2\xbb";
col out_blank_line_col = {
    .fg = col_black | col_bright, .bg = col_none, .attr = 0
};

/* Col description for characters in the log bar */
col_desc out_log_col_desc  = {
    .fg = col_null, .bg = col_null
};

/* Colour of trailing spaces in the log bar */
col out_log_space_col = {
    .fg = col_black | col_bright, .bg = col_none, .attr = col_under
};

col out_trailing_space_col = {
    .fg = col_red, .bg = col_none, .attr = 0
};

chr out_trailing_space_chr = {
    .utf8 = ";",
    .fnt = {
        .fg = col_red, .bg = col_none, .attr = 0
    }
};

static struct termios out_tattr_orig;

void out_goto(int cn, int ln, FILE *f)
{
    print_fmt(GOTO(%d, %d), ln, cn);
}

void out_log(vec *chrs, FILE *f)
{
    /* A vector for storing the characters modified with out_log_col_desc */
    vec colchrs;
    ssize_t ind, len;

    len = (ssize_t)vec_len(chrs);

    vec_init(&colchrs, sizeof(chr));
    vec_ins(&colchrs, 0, len, vec_first(chrs));

    for (ind = 0; ind < len; ind++)
    {
        chr *c;

        c = vec_get(&colchrs, ind);
        chr_set_cols(c, out_log_col_desc);
    }

    if (len < out_cols)
    {
        chr space = { .utf8 = " ", .fnt = out_log_space_col };

        vec_ins(&colchrs, len, out_cols - len, NULL);

        for (ind = len; (ssize_t)ind < out_cols; ind++)
            memcpy(vec_get(&colchrs, ind), &space, sizeof(chr));
    }

    out_goto(0, out_rows, f);

    out_chrs(vec_first(&colchrs), out_cols, 0, f);

    vec_kill(&colchrs);
}

void out_clr_line(FILE *f)
{
    print_str(CLR_LINE);
}

void out_blank_line(FILE *f)
{
    print_str(CLR_LINE);

    col_print(out_blank_line_col, f);
    print_str(out_blank_line_text);
}

void out_chrs(chr *chrs, size_t n, size_t off, FILE *f)
{
    col prevcol = col_default;
    col currcol;
    size_t ind;

    print_str(RESET_COL);

    for (ind = 0; ind < n; ind++)
    {
        chr *c;

        c = &chrs[ind];

        if (*(c->utf8) == '\t')
        {
            indent_print_tab(ind + off, f, c->fnt);
            prevcol = c->fnt;
        }
        else if (!chr_is_blank(c))
        {
            currcol = c->fnt;
            if (memcmp(&currcol, &prevcol, sizeof(col)) != 0)
                col_print(currcol, f);

            chr_print(c, f);
            prevcol = currcol;
        }
    }

    col_print(col_default, f);
}

static void out_handle_winch(int sign)
{
    struct winsize w;

    ioctl(fileno(stdin), TIOCGWINSZ, &w);

    out_cols = w.ws_col;
    out_rows = w.ws_row;

    out_to_resize = 1;
}

void out_init(FILE *f)
{
    struct termios tattr;
    struct sigaction act;
    vec    empty;

    /* Set terminal attributes */
    tcgetattr(fileno(f), &tattr);

    memcpy(&out_tattr_orig, &tattr, sizeof(struct termios));

    tattr.c_lflag &= ~(ICANON | ECHO | ISIG);
    tattr.c_iflag &= ~(IXON | IXOFF);
    tattr.c_iflag |= IXANY;

    tattr.c_cc[VMIN]  = 1;
    tattr.c_cc[VTIME] = 0;

    tcsetattr(fileno(f), TCSANOW, &tattr);

    /* Get window size */
    out_handle_winch(0);

    /* Clear sreen and hide cursor */
    print_str(CLR_SCREEN HIDE_CUR);

    /* Mount window size handler */
    act.sa_handler = out_handle_winch;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    sigaction(SIGWINCH, &act, NULL);

    vec_init(&empty, sizeof(chr));
    out_log(&empty, stdout);

    vec_kill(&empty);

    out_init_thread();
}

void out_kill(FILE *f)
{
    updater_end(&out_thread);

    tcsetattr(fileno(f), TCSANOW, &out_tattr_orig);

    print_str(CLR_SCREEN SHOW_CUR RESET_COL);
    print_flush();
}

static void out_init_thread(void)
{
    out_thread.fptr_line  = out_updater_line;
    out_thread.fptr_after = out_updater_after;

    updater_send_to(&out_thread);

    updater_start(&out_thread);
}

static int out_updater_line(buf *b, cur *c)
{
    if (b == win_cur->b)
    {
        win_out_line(win_cur, *c);

        print_flush();
    }

    return 1;
}

static int out_updater_after(buf *b, cur *c)
{
    if (b == win_cur->b)
    {
        if (c->ln < win_min_ln(win_cur))
            *c = (cur){ .ln = c->ln + 1 };

        if (c->ln > win_max_ln(win_cur))
            return 0;

        win_out_line(win_cur, *c);

        print_flush();

        return 1;
    }


    return 0;
}
