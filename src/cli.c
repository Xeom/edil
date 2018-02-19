#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 1
#endif
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>

#include "files.h"
#include "inp.h"
#include "cmd.h"
#include "text.h"
#include "bar.h"

#include "cli.h"

#define CLI_COL_CUR1  text_col_rev|text_col_bold
#define CLI_COL_CUR2  text_col_under|text_col_bold

#define CLI_COL_BLANK text_col_black|text_col_bold
#define CLI_STR_BLANK "\xc2\xbb"

int cli_h = 16, cli_w = 64;
int cli_alive = 0;
static struct termios cli_torig;

void cli_goto(size_t cn, size_t ln)
{
    printf("\033[%lu;%luH", ln + 1, cn + 1);
}

void cli_fg(text_col col)
{
    fputs("\033[0", stdout);

    if (col & text_col_bold)
        fputs(";1", stdout);

    if (!(col & text_col_none))
        printf(";3%d", col & text_col_allcols);

    if (col & text_col_under)
        fputs(";4", stdout);

    if (col & text_col_rev)
        fputs(";7", stdout);

    if (col & text_col_blink)
        fputs(";5", stdout);

    fputs("m", stdout);
}

void cli_line_blank(void)
{
    fputs("\033[K", stdout);
    cli_fg(CLI_COL_BLANK);
    fputs(CLI_STR_BLANK, stdout);
    cli_fg(text_col_none);
}

void cli_chrs_here(vec *chrs, size_t len)
{
    text_col prevcol = -1;
    text_col col;
    size_t ind;

    if (len == 0) return;

    fputs("\033[K", stdout);

    for (ind = 0; ind < vec_len(chrs); ind++)
    {
        text_char *chr;

        chr = vec_get(chrs, ind);
        if (len-- == 0) break;

        col = chr->fg;
        if (col != prevcol) cli_fg(col);

        fwrite(chr->utf8, 1, text_utf8_len(chr->utf8[0]), stdout);

        prevcol = col;
    }

    cli_fg(text_col_none);
}  

void cli_line_here(text_buf *b, size_t cn, size_t ln, size_t len)
{
    text_char space = {.utf8 = " ", .fg = text_col_none};
    vec      line;
    text_cur cur;
    size_t   linelen;

    if (!(text_buf_getflag(b) & text_flag_vis)) return;
    if (ln > text_buf_len(b))                   return;

    text_buf_get(b, ln, &line);
    text_buf_getcur(b,  &cur);

    linelen = vec_len(&line);

    if (cur.ln1 == ln)
    {
        text_char *curchr;
        if (cur.cn1 == linelen) vec_ins(&line, linelen, 1, &space);

        curchr = vec_get(&line, cur.cn1);
        if (curchr) curchr->fg ^= CLI_COL_CUR1;
    }
    if (cur.ln2 == ln && !(cur.cn1 == cur.cn2 && cur.ln1 == cur.ln2))
    {
        text_char *curchr;
        if (cur.cn2 == linelen) vec_ins(&line, linelen, 1, &space);

        curchr = vec_get(&line, cur.cn2);
        if (curchr) curchr->fg ^= CLI_COL_CUR2;
    }

    if (cn) vec_del(&line, 0, cn);

    cli_chrs_here(&line, len);

    vec_kill(&line);
}

void cli_line(text_buf *b, size_t cn, size_t ln)
{
    if (ln < b->scrolly || ln >= b->scrolly + b->h) return;
    if                    (cn >= b->scrollx + b->w) return;

    if (cn < b->scrollx) cn = b->scrollx;

    if (ln >= text_buf_len(b))
    {
        cli_goto(b->x, b->y + ln - b->scrolly);
        cli_line_blank();
    }
    else
    {
        cli_goto(b->x + cn - b->scrollx, b->y + ln - b->scrolly);
        cli_line_here(b, cn, ln, b->w - cn + b->scrollx);
    }
}   

void cli_lines_after(text_buf *b, size_t ln)
{
    if (ln < b->scrolly)
        ln = b->scrolly;

    while (ln <= b->h + b->scrolly)
        cli_line(b, 0, ln++);
}

void cli_handle_winch(int sig)
{
    struct winsize w;

    ioctl(STDIN_FILENO, TIOCGWINSZ, &w);
    cli_h = w.ws_row;
    cli_w = w.ws_col;

    text_cur_buf->w = cli_w;
    text_cur_buf->h = cli_h - 1;

    cli_lines_after(text_cur_buf, 0);

    fflush(stdout);
}

void cli_init(void)
{
    struct termios tinfo;
    struct sigaction act;

    /* Set terminal attributes */   
    tcgetattr(STDIN_FILENO, &tinfo);

    memcpy(&cli_torig, &tinfo, sizeof(struct termios));

    tinfo.c_lflag -= tinfo.c_lflag & ICANON;
    tinfo.c_lflag -= tinfo.c_lflag & ECHO;

    tinfo.c_cc[VMIN]  = 1;
    tinfo.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &tinfo);
    
    /* Get window size */
    cli_handle_winch(0);

    /* Clear sreen and hide cursor */
    fputs("\033[2J\033[?25l", stdout);

    /* Mount window size handler */
    act.sa_handler = cli_handle_winch;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    sigaction(SIGWINCH, &act, NULL);

    /* It's alive! */
    cli_alive = 1;
}

void cli_kill(void)
{
    /* Reset terminal attributes */
    tcsetattr(STDIN_FILENO, TCSANOW, &cli_torig);

    /* Clear screen and show cursor */
    fputs("\033[2J\033[?25h", stdout);
}

int main(void)
{
    text_buf b;

    text_buf_init(&b);
    files_load(&b, fopen("test.txt", "r"));
    b.flags |= text_flag_vis;
    text_cur_buf = &b;

    cmd_init();
    cli_init(); 
    inp_init();
    bar_init();

    cli_lines_after(&b, 0);

    fflush(stdout);

    while (cli_alive) 
    {
       inp_loop();
    }
    cli_kill();
    inp_kill();
    cmd_kill();

    return 0;
}
