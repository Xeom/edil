#include <stdlib.h>
#include <string.h>

#include "cli.h"
#include "vec.h"

#include "text.h"

text_buf *text_cur_buf;

int text_utf8_len(char utf8)
{
    if (utf8 <= 0x7f) return 1;
    if (utf8 <= 0xbf) return 1; /* This is an error :( */
    if (utf8 <= 0xdf) return 2;
    if (utf8 <= 0xef) return 3;
    if (utf8 <= 0xf7) return 4;
    if (utf8 <= 0xfb) return 5;
    if (utf8 <= 0xfd) return 6;
    return 1;
}

void text_buf_init(text_buf *b)
{
    vec      *line;

    vec_init(&(b->lines), sizeof(vec));
    line = vec_ins(&(b->lines), 0, 1, NULL);

    vec_init(line, sizeof(text_char));

    b->cur.cn1 = 0;
    b->cur.ln1 = 0;
    b->cur.cn2 = 0;
    b->cur.ln2 = 0;

    b->flags  = 0;

    b->x = 0;
    b->y = 0;
    b->w = 30;
    b->h = 10;

    b->scrollx = 0;
    b->scrolly = 0; 
}

void text_buf_kill(text_buf *b)
{
    vec_kill(&(b->lines));
}

void text_buf_setfg(text_buf *b, size_t cn, size_t ln, size_t maxcn, text_col col)
{
    size_t curcn;
    vec   *line;

    line = vec_get(&(b->lines), ln);
  
    if (!line) return;

    for (curcn = cn; curcn <= maxcn; curcn++)
    {
        text_char *chr;
        chr = vec_get(line, cn);

        if (!chr) break;

        chr->fg = col;
    }

    cli_line(b, cn, ln);    
}

void text_buf_ins(text_buf *b, size_t cn, size_t ln, char *utf8)
{
    vec       *line;
    size_t     len, ind, currcn;
    text_col   bg, fg;
    text_char *chr;
    text_char  chrtemplate;

    len = strlen(utf8);
    ind = 0;

    line = vec_get(&(b->lines), ln);

    if (!line) return;

    /* Build a template */
    memset(&(chrtemplate.utf8), 0, sizeof(chrtemplate.utf8));
    chr = vec_get(line, cn);

    if (!chr)
    {
        chrtemplate.bg = text_col_none;
        chrtemplate.fg = text_col_none;
    }
    else
    {
        chrtemplate.bg = chr->bg;
        chrtemplate.fg = chr->fg;
    }
    
    if (!line) return;

    currcn = cn;

    while (ind < len)
    {
        int        width;
        text_char *chr;
        
        width = text_utf8_len(utf8[ind]);
 
        if (width + ind > len) break;
 
        chr = vec_ins(line, currcn++, 1, &chrtemplate);
        if (!chr) break;

        memcpy(chr->utf8, utf8 + ind, width);
        ind += width;
    }

    cli_line(b, cn, ln);
}

void text_buf_del(text_buf *b, size_t cn, size_t ln, size_t n)
{
    vec *line;

    line = vec_get(&(b->lines), ln);

    vec_del(line, cn, n);

    cli_line(b, cn, ln);
}

void text_buf_ins_line(text_buf *b, size_t ln)
{
    vec *line;

    line = vec_ins(&(b->lines), ln, 1, NULL);
    if (!line) return;
    vec_init(line, sizeof(text_char));

    cli_lines_after(b, ln);
}

void text_buf_del_line(text_buf *b, size_t ln)
{
    if (vec_del(&(b->lines), ln, 1) == -1) return;

    cli_lines_after(b, ln);
}

void text_buf_move_cur(text_buf *b, int cn1, int ln1, int cn2, int ln2)
{
    text_cur orig;
    memcpy(&orig, &(b->cur), sizeof(text_cur));

    b->cur.cn1 += cn1;
    b->cur.ln1 += ln1;
    b->cur.cn2 += cn2;
    b->cur.ln2 += ln2;

    if (b->cur.ln1 != orig.ln1)
    {
        cli_line(b, orig.cn1,   orig.ln1);
        cli_line(b, b->cur.cn1, b->cur.ln1);
    }
    else
    {
        cli_line(b, (orig.cn1 < b->cur.cn1) ? orig.cn1 : b->cur.cn1, b->cur.ln1);
    }

    if (b->cur.ln2 != orig.ln2)
    {
        cli_line(b, orig.cn2,   orig.ln2);
        cli_line(b, b->cur.cn2, b->cur.ln2);
    }
    else
    {
        cli_line(b, (orig.cn2 < b->cur.cn2) ? orig.cn2 : b->cur.cn2, b->cur.ln2);
    }
}




#include <unistd.h>
#include <stdio.h>
int main2(void)
{
    text_buf b;

    text_buf_init(&b);
    b.flags |= text_flag_vis;
    text_cur_buf = &b;

    cli_init();
    cli_lines_after(&b, 0);
    sleep(1);text_buf_ins(&b, 0, 0, "Hello");fflush(stdout);
    sleep(1);text_buf_ins(&b, 0, 0, "To the world, ");fflush(stdout);
    sleep(1);text_buf_setfg(&b, 14, 0, 14, text_col_red|text_col_under);fflush(stdout);
    fflush(stdout);
sleep(1);
    cli_kill();

    return 0;
}
