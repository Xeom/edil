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

#include "out.h"

#define GOTO(cn, ln) "\033[" #ln ";" #cn "H"
#define CLR_LINE   "\033[K"
#define CLR_SCREEN "\033[2J"
#define HIDE_CUR   "\033[?25l"
#define SHOW_CUR   "\033[?25h"
#define RESET_COL  "\033[0m"

static void out_handle_winch(int sign);

ssize_t out_cols, out_rows;

col out_blank_line_col = { .fg = col_black | col_bright, .bg = col_none, .attr = 0 };
char *out_blank_line_text = "\xc2\xbb";

col_desc out_cur1_col_desc = { .inv = col_rev,   .fg = col_null, .bg = col_null };
col_desc out_cur2_col_desc = { .inv = col_under, .fg = col_null, .bg = col_null };

static struct termios out_tattr_orig;

void out_goto(int cn, int ln, FILE *f)
{
    fprintf(f, GOTO(%d, %d), ln, cn);
}

void out_blank_line(FILE *f)
{
    fputs(CLR_LINE, f);

    col_print(out_blank_line_col, f);
    fputs(out_blank_line_text,    f);
}

void out_chrs(chr *chrs, size_t n, FILE *f)
{
    col prevcol = { .fg = col_none, .bg = col_none, .attr = 0 };
    col currcol;
    size_t ind;

    fputs(CLR_LINE RESET_COL, f);

    for (ind = 0; ind < n; ind++)
    {
        chr *c;

        c = chrs + ind;

        currcol = c->fnt;
        if (memcmp(&currcol, &prevcol, sizeof(col))) col_print(currcol, f);

        chr_print(c, f);

        prevcol = currcol;
    }
}

static void out_handle_winch(int sign)
{
    struct winsize w;

    ioctl(fileno(stdin), TIOCGWINSZ, &w);

    out_cols = w.ws_col;
    out_rows = w.ws_row;

//    out_lines_after();

    fflush(stdout);
}

void out_init(FILE *f)
{
    struct termios tattr;
    struct sigaction act;

    /* Set terminal attributes */   
    tcgetattr(fileno(f), &tattr);

    memcpy(&out_tattr_orig, &tattr, sizeof(struct termios));

    tattr.c_lflag -= tattr.c_lflag & ICANON;
    tattr.c_lflag -= tattr.c_lflag & ECHO;

    tattr.c_cc[VMIN]  = 1;
    tattr.c_cc[VTIME] = 0;

    tcsetattr(fileno(f), TCSANOW, &tattr);
    
    /* Get window size */
    out_handle_winch(0);

    /* Clear sreen and hide cursor */
    fputs(CLR_SCREEN HIDE_CUR, f);

    /* Mount window size handler */
    act.sa_handler = out_handle_winch;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    sigaction(SIGWINCH, &act, NULL);
}

void out_kill(FILE *f)
{
    tcsetattr(fileno(f), TCSANOW, &out_tattr_orig);

    fputs(CLR_SCREEN SHOW_CUR RESET_COL, f);
}
