#include "chr.h"
#include "indent.h"

#include "cmd/indent.h"

#define INDENT_MAX_TABWIDTH 64

static void indent_mode_arg(char *str, vec *rtn);

void indent_cmd_lvlwidth(vec *rtn, vec *args, win *w)
{
    int val;

    if (vec_len(args) == 2)
    {
        vec   arg;
        char *str;

        vec_init(&arg, sizeof(char));
        chr_to_str(vec_get(args, 1), &arg);

        str = vec_get(&arg,  0);

        if (sscanf(str, "%d", &val) != 1 ||
            val > INDENT_MAX_TABWIDTH ||
            val < 0)
        {
            chr_format(rtn, "err: '%s' is not a valid width, ", str);
        }
        else
        {
            indent_lvl_width = val;
        }

        vec_kill(&arg);
    }

    chr_format(rtn, "lvlwidth: %d", indent_tab_width);
}

void indent_cmd_tabwidth(vec *rtn, vec *args, win *w)
{
    int val;

    if (vec_len(args) == 2)
    {
        vec   arg;
        char *str;

        vec_init(&arg, sizeof(char));
        chr_to_str(vec_get(args, 1), &arg);

        str = vec_get(&arg,  0);

        if (sscanf(str, "%d", &val) != 1 ||
            val > INDENT_MAX_TABWIDTH ||
            val < 0)
        {
            chr_format(rtn, "err: '%s' is not a valid width, ", str);
        }
        else
        {
            indent_set_tab_width(val);
        }

        vec_kill(&arg);
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
