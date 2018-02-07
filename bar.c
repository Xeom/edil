#include <unistd.h>
#include <stdio.h>

#include "text.h"
#include "cli.h"
#include "cmd.h"

#include "bar.h"

vec bar_text;
#define MODE_FMT "<%s> ", mode
#define CURL_FMT "%lu,%lu", ln1+1,cn1+1
#define CURR_FMT "%lu,%lu (%ldx%ld)", ln2+1,cn2+1, labs(ln1-ln2)+1, labs(cn1-cn2)+1

void bar_str2vec(char *str, vec *v)
{
    size_t width = 1, utf8ind = 1;
    text_char *chr;

    while (*str)
    {
        if (utf8ind == width)
        {
            width = text_utf8_len(*str);
            chr = vec_ins(v, vec_len(v), 1, NULL);
            chr->fg = text_col_none;
            utf8ind = 0;
        }

        chr->utf8[utf8ind]  = *str;
        utf8ind            += 1;
        str++;
    }
}
            

void bar_mode(vec *v)
{
    char  str[32];
    char *mode;

    mode = cmd_mode_names[cmd_cur_mode];
    snprintf(str, 32, MODE_FMT);

    bar_str2vec(str, v);
}

void bar_curl(vec *v)
{
    char str[32];
    text_cur *cur;
    size_t ln1, cn1, ln2, cn2;

    cur = &(text_cur_buf->cur);

    ln1 = cur->ln1;cn1 = cur->cn1;
    ln2 = cur->ln2;cn2 = cur->cn2;

    snprintf(str, 32, CURL_FMT);

    bar_str2vec(str, v);
}

void bar_curr(vec *v)
{
    char str[32];
    text_cur *cur;
    size_t ln1, cn1, ln2, cn2;

    cur = &(text_cur_buf->cur);

    ln1 = cur->ln1;cn1 = cur->cn1;
    ln2 = cur->ln2;cn2 = cur->cn2;

    snprintf(str, 32, CURR_FMT);

    bar_str2vec(str, v);
}

void bar_update(void)
{
    vec left, right;
 
    vec_init(&left,  sizeof(text_char));
    vec_init(&right, sizeof(text_char));

    bar_mode(&left);

    bar_curl(&left);
    bar_curr(&right);

    cli_goto(0, cli_h - 1);
    cli_chrs_here(&left, cli_w);

    cli_goto(cli_w - vec_len(&right) - 1, cli_h - 1);
    cli_chrs_here(&right, vec_len(&right));
}


