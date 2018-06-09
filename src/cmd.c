#include <ctype.h>
#include <string.h>

#include "vec.h"
#include "namevec.h"
#include "chr.h"
#include "ring.h"

#include "cmd/buf.h"
#include "cmd/file.h"
#include "cmd/nav.h"
#include "cmd/indent.h"
#include "cmd/region.h"
#include "cmd/conf.h"

#include "cmd.h"

vec cmd_items;

buf *cmd_log_buf = NULL;
col_desc cmd_log_cmd_col    = { .fg = col_yellow, .bg = col_null };
col_desc cmd_log_rtn_col    = { .fg = col_white | col_bright, .bg = col_null };
col_desc cmd_log_prefix_col = { .fg = col_yellow | col_bright, .bg = col_null };

/* Increment ind until whitespace isn't found, return this *
 * value.                                                  */
static size_t cmd_eat_whitespace(vec *chrs, size_t ind);

static size_t cmd_parse_str(vec *str, vec *chrs, size_t ind);
static size_t cmd_parse_word(vec *str, vec *chrs, size_t ind);

void cmd_init(void)
{
    vec_init(&cmd_items, sizeof(namevec_item));

    cmd_nav_init();
    cmd_buf_init();
    cmd_indent_init();
    cmd_file_init();
    cmd_region_init();
    cmd_conf_init();

    namevec_sort(&cmd_items);
}

void cmd_kill(void)
{
    vec_kill(&cmd_items);
}

void cmd_run(vec *args, vec *rtn, win *w)
{
    size_t number, len;
    namevec_item *item;
    vec *arg;

    len = vec_len(args);
    if (len == 0) return;

    arg  = vec_first(args);

    item = namevec_get_chrs(&cmd_items, arg, &number);

    if (!item || number == 0)
    {
        chr_from_str(rtn, "err: '");
        vec_cpy(rtn, arg);
        chr_format(rtn, "' is not a known command");
    }
    else if (number > 1)
    {
        chr_from_str(rtn, "err: '");
        vec_cpy(rtn, arg);
        chr_format(rtn, "' is ambiguous (");

        while (number--)
        {
            namevec_item *i;
            i = item + number;
            chr_format(rtn, "%s%s", i->name, (number) ? ", " : "");
        }

        chr_from_str(rtn, ")");
    }
    else
    {
        cmd_info *info = item->ptr;
        info->fptr(args, rtn,  w);
    }

}

void cmd_parse(vec *args, vec *chrs, size_t ind)
{
    size_t len;

    len = vec_len(chrs);

    for (; ind < len; ind++)
    {
        chr *c;
        vec *arg;
        ind = cmd_eat_whitespace(chrs, ind);

        c = vec_get(chrs, ind);
        if (!c) break;

        arg = vec_app(args, NULL);
        vec_init(arg, sizeof(chr));

        if (strcmp("\"", c->utf8) == 0)
            ind = cmd_parse_str(arg, chrs, ind + 1);

        else
            ind = cmd_parse_word(arg, chrs, ind);
    }
}

static size_t cmd_eat_whitespace(vec *chrs, size_t ind)
{
    size_t len;

    len = vec_len(chrs);

    for (; ind < len; ind++)
    {
        chr *c;
        c = vec_get(chrs, ind);

        if (strcmp(" ", c->utf8) != 0)
            break;
    }

    return ind;
}

static size_t cmd_parse_str(vec *str, vec *chrs, size_t ind)
{
    int    escaped = 0;
    size_t len;

    len = vec_len(chrs);

    for (; ind < len; ind++)
    {
        chr *c;
        c = vec_get(chrs, ind);

        if (escaped)
            vec_app(str, c);
        else if (strcmp("\\", c->utf8) == 0)
            escaped = 1;
        else if (strcmp("\"", c->utf8) == 0)
        {
            ind += 1;
            break;
        }
        else
            vec_app(str, c);
    }

    return ind;
}

static size_t cmd_parse_word(vec *str, vec *chrs, size_t ind)
{
    int    escaped = 0;
    size_t len;

    len = vec_len(chrs);

    for (; ind < len; ind++)
    {
        chr *c;
        c = vec_get(chrs, ind);

        if (escaped)
            vec_app(str, c);
        else if (strcmp("\\", c->utf8) == 0)
            escaped = 1;
        else if (strcmp(" ",  c->utf8) == 0)
            break;
        else if (strcmp(")",  c->utf8) == 0)
            break;
        else if (strcmp("\"", c->utf8) == 0)
            break;
        else
            vec_app(str, c);
    }

    return ind;
}

void cmd_log(vec *chrs, int iscmd)
{
    cur loc = {0, 0};

    if (cmd_log_buf == NULL)
    {
        cmd_log_buf = ring_new();
        cmd_log_buf->flags |= buf_readonly | buf_nofile | buf_nokill;
        buf_clr(cmd_log_buf);
        buf_set_name(cmd_log_buf, "'cmd log'");
    }

    loc.ln = buf_len(cmd_log_buf) - 1;
    buf_ins_line(cmd_log_buf, loc);

    buf_ins(cmd_log_buf, loc, vec_first(chrs), vec_len(chrs));

    if (iscmd)
    {
        vec prefix;
        int len;

        vec_init(&prefix, sizeof(chr));
        chr_from_str(&prefix, ">> ");

        len = vec_len(&prefix);

        buf_ins(cmd_log_buf, loc, vec_first(&prefix), len);
        buf_setcol(cmd_log_buf, loc, len, cmd_log_prefix_col);
        loc.cn += len;

        buf_setcol(cmd_log_buf, loc, vec_len(chrs), cmd_log_cmd_col);
        vec_kill(&prefix);
    }
    else
    {
        buf_setcol(cmd_log_buf, loc, vec_len(chrs), cmd_log_rtn_col);
    }

    if (buf_len(cmd_log_buf) > 256)
    {
        buf_del_line(cmd_log_buf, (cur){0, 0});
        buf_del_line(cmd_log_buf, (cur){0, 0});
    }
}

void cmd_print_all(FILE *stream)
{
    fputs("Commands\n",   stream);
    fputs("========\n\n", stream);

    fputs("To type a command in *Edil*, enter *command mode*, by pressing\n"
          "`Ctrl-X`, then you can simply type a command and press enter to\n"
          "run it.\n\n"

          "Commands take arguments delimited by spaces after their name,\n"
          "e.g. `load file1`, and for arguments with spaces inside them,\n"
          "quotation marks can be used. e.g. `load \"file 1\"`.\n\n"

          "Many commands are also bound to shortcut or run on certain key\n"
          "combinations. A shortcut is where, when you press the key, Edil\n"
          "enters bar mode and the command is inserted into the bar ready to\n"
          "be run. This allows you to specify arguments for the command, or\n"
          "prevents accidentally running the command when you did not mean\n"
          "to.\n\n"

         "The implementation for this system is in the `src/cmd.c` file, and\n"
         "files containing implementations of commands themselves are stored\n"
         "in the `src/cmd` directory.\n\n"

         "List of commands\n"
         "----------------\n", stream);

    VEC_FOREACH(&cmd_items, namevec_item *, item,
        cmd_print_pre(stream, item->ptr);
    );

    fputs("\n"
          "Full documentation\n"
          "------------------\n\n", stream);

    VEC_FOREACH(&cmd_items, namevec_item *, item,
        cmd_print_info(stream, item->ptr);
    );
}

void cmd_print_pre(FILE *stream, cmd_info *info)
{
    fprintf(stream, " * [__%s__](#%s-command) - _%s_\n", info->name, info->name, info->desc);
}

void cmd_print_info(FILE *stream, cmd_info *info)
{
    char upper;
    upper = toupper(info->name[0]);

    fprintf(stream, "#### %c%s command \n", upper, info->name + 1);
    fprintf(stream, " - %s\n\n", info->desc);
    fprintf(stream, "%s\n---\n", info->full);
}
