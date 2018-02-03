#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "cmd.h"
#include "text.h"

#include "cli.h"

#define CLI_COL_CUR1  text_col_rev
#define CLI_COL_CUR2  text_col_under

#define CLI_COL_BLANK text_col_black|text_col_bold
#define CLI_STR_BLANK "\xc2\xbb"

#define CLI_INP_TMOUT {.tv_sec = 0, .tv_usec = 50000}

int cli_h, cli_w;
int cli_alive = 0;
static struct termios cli_torig;
vec cli_keycodes;

cli_keycode cli_keycodes_static[] =
{
    {cli_key_up,     "[A"},
    {cli_key_down,   "[B"},
    {cli_key_right,  "[C"},
    {cli_key_left,   "[D"},
    {cli_key_home,   "[1~"},
    {cli_key_del,    "[3~"},
    {cli_key_end,    "[4~"},
    {cli_key_pgup,   "[5~"},
    {cli_key_pgdn,   "[6~"},
    {cli_key_insert, "[4h"},
    {cli_key_f1,     "OP"},
    {cli_key_f2,     "OQ"},
    {cli_key_f3,     "OR"},
    {cli_key_f4,     "OS"},
    {cli_key_f5,     "[15~"},
    {cli_key_f6,     "[17~"},
    {cli_key_f7,     "[18~"},
    {cli_key_f8,     "[19~"},
    {cli_key_f9,     "[20~"},
    {cli_key_f10,    "[21~"},
    {cli_key_f11,    "[23~"},
    {cli_key_f12,    "[24~"}
};

static int cli_keycode_cmp(const void *a, const void *b)
{
    return strcmp(((const cli_keycode *)a)->code, ((const cli_keycode *)b)->code);
}

void cli_init(void)
{
    size_t numkeys;
    struct termios tinfo;
    int    flags;
    
    tcgetattr(STDIN_FILENO, &tinfo);

    memcpy(&cli_torig, &tinfo, sizeof(struct termios));

    tinfo.c_lflag -= tinfo.c_lflag & ICANON;
    tinfo.c_lflag -= tinfo.c_lflag & ECHO;

    tinfo.c_cc[VMIN]  = 1;
    tinfo.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &tinfo);

//   cli_resize();

    fputs("\033[2J\033[?25l", stdout);

    numkeys = sizeof(cli_keycodes_static) / sizeof(cli_keycode);
 
    qsort(&cli_keycodes_static, numkeys, sizeof(cli_keycode), cli_keycode_cmp);
 
    vec_init(&cli_keycodes, sizeof(cli_keycode));
    vec_ins(&cli_keycodes, 0, numkeys, &cli_keycodes_static);
    flags = fcntl(STDIN_FILENO, F_GETFD);
    fcntl(STDIN_FILENO, F_SETFD, flags | O_NONBLOCK);


    cli_alive = 1;
}

void cli_kill(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &cli_torig);

    fputs("\033[2J\033[?25h", stdout);

    vec_kill(&cli_keycodes);
}

static cli_key cli_get_escaped_key(char chr)
{
    static char code[16];
    static int  code_ind   = 0;
    static int  search_ind = 0;
    static int  rtn;

    code[code_ind] = chr;
    code[code_ind + 1] = '\0';
    code_ind += 1;

    while (code_ind <= 14)
    {
        int cmp;
        cli_keycode *curr;

        curr = vec_get(&cli_keycodes, search_ind);
        if (curr) cmp = strcmp(code, curr->code);

        if (cmp == 0) {rtn = curr->key;             break;}
        if (!curr)    {rtn = cli_key_esc | code[0]; break;}
        if (cmp >  0) search_ind++;
        if (cmp <  0)
        {
            if (memcmp(code, curr->code, code_ind) == 0)
                return 0;
            
            rtn = cli_key_esc | code[0];
            break;
        }
    }

    code_ind   = 0;
    search_ind = 0;

    return rtn;
}

cli_key cli_get_key(char chr)
{
    static int escaped = 0;
    cli_key    rtn;

    if (escaped)
    {    
        rtn = cli_get_escaped_key(chr);
        if (rtn == 0) return cli_key_none;
    }
    else if (chr == '\033')
    {
        escaped = 1;
        return cli_key_none;
    }
    else
        rtn = chr;

    escaped = 0;

    if ((rtn & ~cli_key_esc) < 32)
    {
        rtn += 0x40;
        rtn |= cli_key_ctrl;
    }

    return rtn;
}

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

    fputs("m", stdout);
}

void cli_line_blank(void)
{
    cli_fg(CLI_COL_BLANK);
    fputs(CLI_STR_BLANK, stdout);
    cli_fg(text_col_none);
}

void cli_line_here(text_buf *b, size_t cn, size_t ln, size_t len)
{
    text_col nextcol, col = -1;
    vec *line;

    if (!(b->flags & text_flag_vis)) return;

    line = vec_get(&(b->lines), ln);
    if (!line) cli_line_blank();
   
    for (;; cn++)
    {
        text_char *chr;

        chr = vec_get(line, cn);
        if (chr == NULL) break;

        if (--len == 0)  break;

        nextcol = chr->fg;
 
        if (cn == b->cur.cn1 && ln == b->cur.ln1)
            nextcol ^= CLI_COL_CUR1;

        else if (cn == b->cur.cn2 && ln == b->cur.ln2)
            nextcol ^= CLI_COL_CUR2;

        if (col != nextcol)
            cli_fg(nextcol);

        col = nextcol;
        fwrite(chr->utf8, 1, text_utf8_len(chr->utf8[0]), stdout);
    }
    
    cli_fg(text_col_none);
}

void cli_line(text_buf *b, size_t cn, size_t ln)
{
    if (ln    < b->scrolly || ln > b->scrolly + b->h) return;
    if                       (cn > b->scrollx + b->w) return;

    if (cn < b->scrollx) cn = b->scrollx;

    cli_goto(b->x + cn - b->scrollx, b->y + ln - b->scrolly);
    cli_line_here(b, cn, ln, b->w - cn + b->scrollx);
}   

void cli_lines_after(text_buf *b, size_t ln)
{
    if (ln < b->scrolly)
        ln = b->scrolly;

    while (ln <= b->h + b->scrolly)
        cli_line(b, 0, ln++);
}

pthread_t cli_input_thread;

pthread_mutex_t cli_input_mtx;
pthread_cond_t  cli_input_cond;
pthread_mutex_t cli_input_cond_mtx;

vec cli_input_vec;

void *cli_input_listen(void *arg)
{
    char   chr;
    vec   *v;
    size_t len;

    v = &cli_input_vec;

    while (1)
    {
        chr = getchar();

        pthread_mutex_lock(&cli_input_mtx);

        len = vec_len(v);
        vec_ins(v, len, 1, &chr);

        pthread_cond_signal(&cli_input_cond);
        pthread_mutex_unlock(&cli_input_mtx);
   }
}

void cli_input(void)
{   
    char   chr;
    vec   *v;
    size_t len;
    cli_key key;

    v = &cli_input_vec;
    
    pthread_mutex_lock(&cli_input_mtx);
    pthread_cond_wait(&cli_input_cond, &cli_input_mtx);
    pthread_mutex_unlock(&cli_input_mtx);

    do
    {
         pthread_mutex_lock(&cli_input_mtx);
        chr = *((char *)vec_get(v, 0));
        vec_del(v, 0, 1);
        len = vec_len(v);
        pthread_mutex_unlock(&cli_input_mtx);

        key = cli_get_key(chr);
        cmd_handle_key(key);
    } while (len);

    cmd_ins_flush();

    fflush(stdout);        
}
 
/*
void cli_input(void)
{
//    fflush(stdout);cmd_handle_key(cli_gdet_key(getchar()));return;

    fd_set fds;
    struct timeval tmout = CLI_INP_TMOUT;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    select(STDIN_FILENO + 1, &fds, NULL, NULL, &tmout);

    while (FD_ISSET(STDIN_FILENO, &fds))
    {
        int chr;
        cli_key key;

        chr = getchar();    
        if (chr == EOF) break;

        key = cli_get_key(chr);
        cmd_handle_key(key);
    }

    puts("LOVE");
}
*/
#include <unistd.h>
void main(void)
{
    text_buf b;


    text_buf_init(&b);
    b.flags |= text_flag_vis;
    text_cur_buf = &b;

    vec_init(&cli_input_vec, 1);

    cmd_init();
    cli_init();
    cli_lines_after(&b, 0);
    text_buf_ins(&b, 0, 0, "Hello");
    fflush(stdout);

    pthread_create(&cli_input_thread, NULL, cli_input_listen, NULL);

    while (cli_alive) cli_input();
    cli_kill();
}
