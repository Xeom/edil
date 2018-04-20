#include <string.h>

#include "chr.h"
#include "indent.h"

#include "cmd/indent.h"

#define INDENT_MAX_TABWIDTH 64
#define INDENT_MAX_SENSIBLE_DEPTH    256

static void indent_mode_arg(char *str, vec *rtn);

void indent_cmd_lvlwidth(vec *rtn, vec *args, win *w)
{
    int val;

    if (vec_len(args) == 2)
    {
        if (chr_scan(vec_get(args, 1), "%d", &val) != 1)
            chr_from_str(rtn, "err: Not a valid width, ");

        else if (val > INDENT_MAX_TABWIDTH || val < 0)
            chr_from_str(rtn, "err: Width out of range, ");

        else
            indent_lvl_width = val;
    }

    chr_format(rtn, "lvlwidth: %d", indent_lvl_width);
}

void indent_cmd_tabwidth(vec *rtn, vec *args, win *w)
{
    int val;

    if (vec_len(args) == 2)
    {
        if (chr_scan(vec_get(args, 1), "%d", &val) != 1)
            chr_from_str(rtn, "err: Not a valid width, ");

        else if (val > INDENT_MAX_TABWIDTH || val < 0)
            chr_from_str(rtn, "err: Width out of range, ");

        else
            indent_set_tab_width(val);
    }

    chr_format(rtn, "tabwidth: %d", indent_tab_width);
}

void indent_cmd_indentmode(vec *rtn, vec *args, win *w)
{
    size_t ind, len;

    len = vec_len(args);
    for (ind = 1; ind < len; ind++)
    {
        vec *arg, str;

        if (ind != 1) chr_from_str(rtn, ", ");

        arg = vec_get(args, ind);

        vec_init(&str, sizeof(char));
        chr_to_str(arg, &str);
        vec_ins(&str, vec_len(&str), 1, NULL);

        indent_mode_arg(vec_get(&str, 0), rtn);

        vec_kill(&str);
    }
}

static void indent_mode_arg(char *str, vec *rtn)
{
    int enable;
    indent_flag flag;

    if (str[0] == '!')
    {
        enable = 0;
        str   += 1;
    }
    else
    {
        enable = 1;
    }

    if (strcmp(str, "spacify") == 0)
        flag = indent_spacify;
    else if (strcmp(str, "auto") == 0)
        flag = indent_auto;
    else if (strcmp(str, "skipblank") == 0)
        flag = indent_skipblank;
    else if (strcmp(str, "trim") == 0)
        flag = indent_trim;
    else
    {
        chr_format(rtn, "Unknown mode '%s'", str);
        return;
    }

    if (enable)
    {
        chr_format(rtn, "Enabled '%s' mode", str);
        indent_mode |= flag;
    }
    else
    {
        chr_format(rtn, "Disabled '%s' mode", str);
        indent_mode &= ~flag;
    }
}

void indent_cmd_incrindent(vec *rtn, vec *args, win *w)
{
    ssize_t depth;

    w->pri = indent_incr_depth(w->b, w->pri);
    win_out_line(w, (cur){ .ln = w->pri.ln });

    depth = indent_get_depth(w->b, w->pri);
    chr_format(rtn, "depth: %ld", depth);
}

void indent_cmd_decrindent(vec *rtn, vec *args, win *w)
{
    ssize_t depth;

    w->pri = indent_decr_depth(w->b, w->pri);
    win_out_line(w, (cur){ .ln = w->pri.ln });

    depth = indent_get_depth(w->b, w->pri);
    chr_format(rtn, "depth: %ld", depth);
}

void indent_cmd_autoindent(vec *rtn, vec *args, win *w)
{
    ssize_t depth;

    w->pri = indent_auto_depth(w->b, w->pri);
    win_out_line(w, (cur){ .ln = w->pri.ln });

    depth = indent_get_depth(w->b, w->pri);
    chr_format(rtn, "depth: %ld", depth);
}

void indent_cmd_indent(vec *rtn, vec *args, win *w)
{
    ssize_t depth;

    if (vec_len(args) == 2)
    {
        if (chr_scan(vec_get(args, 1), "%ld", &depth) != 1)
            chr_from_str(rtn, "err: Not a valid depth, ");

        else if (depth > INDENT_MAX_SENSIBLE_DEPTH || depth < 0)
            chr_from_str(rtn, "err: Depth out of range, ");

        else
        {
            indent_set_depth(w->b, w->pri, depth);
            w->pri.cn = 0;

            if (w->sec.ln == w->pri.ln)
                w->sec.cn = 0;

            win_out_line(w, (cur){ .ln = w->pri.ln });
        }
    }

    depth = indent_get_depth(w->b, w->pri);
    chr_format(rtn, "depth: %ld", depth);
}
