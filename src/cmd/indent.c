#include "chr.h"
#include "indent.h"

#include "cmd/indent.h"

#define INDENT_MAX_TABWIDTH 64

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
