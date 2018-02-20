#include "out.h"

#define GOTO(cn, ln) "\033[" #ln ";" #cn "H"
#define CLR_LINE   "\033[K"
#define CLR_SCREEN "\033[2J"
#define HIDE_CUR   "\033[?25l"
#define SHOW_CUR   "\033[?25h"

col out_blank_line_col = { .fg = col_yellow, .bg = col_none, .attr = none }
char *out_blank_line_text = "\xc2\xbb"

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

void out_chrs(vec *chrs, size_t len, FILE *f)
{
    text_col prevcol = { .fg = col_none, .bg = col_none};
    text_col col;
    size_t ind;

    fputs(CLR_LINE, f);

    if (len == 0) return;

    for (ind = 0; ind < vec_len(chrs); ind++)
    {
        text_char *chr;

        chr = vec_get(chrs, ind);
        if (len-- == 0) break;

        col = chr->fg;
        if (col != prevcol) col_print(col, f);

        chr_print(chr, f);

        prevcol = col;
    }

    cli_fg(text_col_none);
}

void out_line(buf *b, size_t len, cur c, FILE *f)
{
    chr space = { .utf8 = " ", .fnt = { .fg = col_none, .bg = col_none } };
    vec     *line, modline;
    size_t   linelen;

    if (ln > text_buf_len(b)) return;

    line = vec_get(&(b->lines), ln);
    if (!line) return;

    if (c.cn || b->pri.ln == c.ln || b->sec.ln == c.ln)
    {
        linelen = vec_len(&line);

        vec_init(&modline, sizeof(chr));
        vec_ins(&modline, 0, linelen, vec_get(line, 0));

        line = &modline;
    }
  
    if (b->pri.ln == c.ln)
    {
        chr *curchr;

        if (b->pri.cn == linelen) vec_ins(line, linelen, 1, &space);

        curchr = vec_get(&line, cur.cn1);
        if (curchr) chr_set_cols(curchr, out_cur1_col_desc);
    }

    if (b->sec.ln == c.ln && !(b->pri.cn == b->sec.cn && b->pri.ln == b->sec.ln))
    {
        chr *curchr;

        if (cur.cn2 == linelen) vec_ins(&line, linelen, 1, &space);

        curchr = vec_get(&line, cur.cn2);
        if (curchr) chr_set_cols(curchr, out_cur2_col_desc);
    }

    if (cn) vec_del(line, 0, cn);

    cli_chrs_here(line, len, f);

    if (line == &modline) vec_kill(&line);
}

void out_lines_after(buf *b, cur c, FILE *f)
{
    if (c.ln < b->scrolly)
    {
        c.ln = b->scrolly;
        c.cn = 0;
    }

    while (c.ln <= b->h + b->scrolly)
    {
        out_line(b, c, f);
        c.ln += 1;
        c.cn  = 0;
    }
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
//    cli_handle_winch(0);

    /* Clear sreen and hide cursor */
    fputs(CLR_SCREEN HIDE_CUR, f);
/*
    /* Mount window size handler 
    act.sa_handler = cli_handle_winch;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    sigaction(SIGWINCH, &act, NULL);*/}

void out_kill(FILE *f)
{
    tcsetattr(fileno(f), TCSANOW, &out_tattr_orig);

    fputs(CLR_SCREEN SHOW_CUR, f);
}

