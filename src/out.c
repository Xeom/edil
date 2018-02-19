#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "cmd.h"
#include "text.h"

#include "cli.h"

#define OUT_COL_CUR1  text_col_rev|text_col_bold
#define OUT_COL_CUR2  text_col_under|text_col_bold

#define OUT_COL_BLANK text_col_black|text_col_bold
#define OUT_STR_BLANK "\xc2\xbb"

#define USING_MTX(mtx, code)          \
        pthread_mutex_lock(&(mtx));   \
        do { code } while (0);        \
        pthread_mutex_unlock(&(mtx)); \

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
        if (--len == 0) break;

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

void cli_resize(void)
{
    struct winsize w;

    ioctl(STDIN_FILENO, TIOCGWINSZ, &w);
    cli_h = w.ws_row;
    cli_w = w.ws_col;
}

pthread_t out_printing_thread;

pthread_mutex_t out_mtx;
pthread_cond_t  out_cond;

typedef struct out_loc_s out_loc;

struct out_loc
{
    size_t ln, cn;
}

vec     out_update_lines;
ssize_t out_update_after = -1;

void *out_update_listen(void *arg)
{
    ssize_t after;

    USING_MUTEX(&out_mtx,
        if (out_update_after == -1 && vec_len(out_update_lines) == 0)
            pthread_cond_wait(&out_cond, &out_mtx);
        after = out_update_after;
    );

    
}

int out_cmp_loc(const void *a, const void *b)
{
    out_loc aval, bval;
    aval = *a;
    bval = *b;

    if      (a->ln > b->ln) return  1;
    else if (a->ln < b->ln) return -1;
    else if (a->cn > b->cn) return  1;
    else if (c->cn < b->cn) return -1;
    else                    return  0;
}

void out_update_all(vec *v, size_t after);
{
    len  = vec_len(&to_update);

    vec_sort(v, out_cmd_loc);

    for (ind = 0; ind < len; ind++)
    {
        out_loc *loc, *prev;
        loc = vec_get(&to_update, ind);

        if (ind && loc->ln == prev->ln) continue;
        prev = loc;

        if (after != -1 && (ssize_t)loc->ln >= after) break;

        out_line(loc);
    }
    while (1)
    {
        size_t *ln;
        USING_MUTEX(&out_mtx,
            ln = vec_get(&out_update_lines, vec_len(&out_update_lines) - 1);
            
}
    

void *cli_input_listen(void *arg)
{
    char   chr;
    vec   *v;
    size_t len;

    v = &cli_input_vec;

    while (1)
    {
        chr = getchar();

        USING_MUTEX(cli_input_mtx,
            len = vec_len(v);
            vec_ins(v, len, 1, &chr);
        );

        pthread_cond_signal(&cli_input_cond);
    }
}

void cli_input(void)
{   
    char   chr;
    vec   *v;
    size_t len;
    cli_key key;

    v = &cli_input_vec;
    
    USING_MUTEX(cli_input_mtx,
        pthread_cond_wait(&cli_input_cond, &cli_input_mtx);
    );
 
    do
    {
        USING_MUTEX(cli_input_mtx,
            chr = *((char *)vec_get(v, 0));
            vec_del(v, 0, 1);
            len = vec_len(v);
        );

        key = cli_get_key(chr);
        cmd_handle_key(key);
    } while (len);

    cmd_ins_flush();

    fflush(stdout);        
}

void cli_init(void)
{
    size_t numkeys;
    struct termios tinfo;
    int    flags;

    /* Set terminal attributes */   
    tcgetattr(STDIN_FILENO, &tinfo);

    memcpy(&cli_torig, &tinfo, sizeof(struct termios));

    tinfo.c_lflag -= tinfo.c_lflag & ICANON;
    tinfo.c_lflag -= tinfo.c_lflag & ECHO;

    tinfo.c_cc[VMIN]  = 1;
    tinfo.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &tinfo);

//   cli_resize();


    /* Clear sreen and hide cursor */
    fputs("\033[2J\033[?25l", stdout);

    /* Load the keycodes */
    numkeys = sizeof(cli_keycodes_static) / sizeof(cli_keycode);
 
    qsort(&cli_keycodes_static, numkeys, sizeof(cli_keycode), cli_keycode_cmp);
 
    vec_init(&cli_keycodes, sizeof(cli_keycode));
    vec_ins(&cli_keycodes, 0, numkeys, &cli_keycodes_static);
    
    /* Create the input listener thread */
    pthread_mutex_init(&cli_input_mtx, NULL);
    pthread_create(&cli_input_thread, NULL, cli_input_listen, NULL);

   
//   flags = fcntl(STDIN_FILENO, F_GETFD);
//    fcntl(STDIN_FILENO, F_SETFD, flags | O_NONBLOCK);

    /* It's alive! */
    cli_alive = 1;


}

void cli_kill(void)
{
    /* Reset terminal attributes */
    tcsetattr(STDIN_FILENO, TCSANOW, &cli_torig);

    /* Clear screen and show cursor */
    fputs("\033[2J\033[?25h", stdout);

    pthread_mutex_destroy(&cli_input_mtx);

    vec_kill(&cli_keycodes);
}

int main(void)
{
    text_buf b;

    text_buf_init(&b);
    b.flags |= text_flag_vis;
    text_cur_buf = &b;

    vec_init(&cli_input_vec, 1);

    cmd_init();
    cli_init();
    cli_lines_after(&b, 0);

    fflush(stdout);

    while (cli_alive) cli_input();
    cli_kill();

    return 0;
}
