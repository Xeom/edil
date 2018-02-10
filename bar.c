#include <unistd.h>
#include <stdio.h>

#include "text.h"
#include "cli.h"
#include "cmd.h"
#include "vec.h"

#include "bar.h"

vec bar_query_inp;
vec bar_prompt;

void (*bar_query_cb)(char *str);

size_t bar_opt_sel;

#define MODE_FMT "<%s> ", mode
#define CURL_FMT "%lu,%lu", ln1+1,cn1+1
#define CURR_FMT "%lu,%lu (%ldx%ld)", ln2+1,cn2+1, labs(ln1-ln2)+1, labs(cn1-cn2)+1

void bar_init(void)
{
    vec_init(&bar_query_inp, sizeof(char));
    vec_init(&bar_prompt,    sizeof(text_char));
}

void bar_str2vec(char *str, vec *v)
{
    size_t width = 1, utf8ind = 1;
    text_char *chr;

    if (!str) return;
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

void bar_query(char *str, void (*cb)(char *str))
{
    bar_query_cb = cb;
    vec_del(&bar_prompt, 0, vec_len(&bar_prompt));
    cmd_cur_mode = cmd_mode_bar;
    bar_str2vec(str, &bar_prompt);
    bar_update();
}

void bar_query_run(void)
{
    bar_query_cb(vec_get(&bar_query_inp, 0));
    cmd_cur_mode = cmd_mode_buf;
    bar_update();
}

void bar_cmd_ins(char *str, size_t n)
{
    size_t len;
    len = vec_len(&bar_query_inp);

    if (len == 0)
    {
        vec_ins(&bar_query_inp, 0, 1, NULL);
        len = 1;
    }

    vec_ins(&bar_query_inp, len - 1, n, str);
    bar_update();   
}

void bar_cmd_del(void)
{
    size_t len;
    len = vec_len(&bar_query_inp);

    vec_del(&bar_query_inp, len - 2, 1);
    bar_update();
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
    size_t ln1, cn1;

    cur = &(text_cur_buf->cur);

    ln1 = cur->ln1;cn1 = cur->cn1;
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

void bar_name(vec *v)
{
    vec name;
    text_buf_getname(text_cur_buf, &name);

    vec_ins(v, vec_len(v), vec_len(&name), vec_get(&name, 0));

    vec_kill(&name);
}

void bar_update(void)
{
    vec left, right;
 
    vec_init(&left,  sizeof(text_char));
    vec_init(&right, sizeof(text_char));

    if (cmd_cur_mode == cmd_mode_buf)
    {
        bar_mode(&left);
        bar_curl(&left);
        bar_curr(&right);
    }
    else if (cmd_cur_mode == cmd_mode_bar)
    {
        vec_ins(&left, 0, vec_len(&bar_prompt), vec_get(&bar_prompt, 0)); 
        bar_str2vec(vec_get(&bar_query_inp, 0), &left);
    }
 
    cli_goto(0, cli_h - 1);
    cli_chrs_here(&left, cli_w);

    cli_goto(cli_w - vec_len(&right) - 1, cli_h - 1);
    cli_chrs_here(&right, vec_len(&right));

    vec_kill(&left);
    vec_kill(&right);
}


