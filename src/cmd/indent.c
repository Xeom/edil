#include <string.h>

#include "win.h"
#include "text/cur.h"
#include "indent.h"
#include "updater.h"

#include "cmd.h"
#include "cmd/indent.h"

#define INDENT_MAX_TABWIDTH 64
#define INDENT_MAX_SENSIBLE_DEPTH    256

static void indent_mode_arg(char *str, vec *rtn);

CMD_FUNCT(lvlwidth,
    CMD_MAX_ARGS(1);

    if (CMD_NARGS)
    {
        int val;
        CMD_ARG_PARSE(1, "%d", &val);

        if (val > INDENT_MAX_TABWIDTH || val < 0)
            CMD_RTN("err: Width out of range, ");

        else
            indent_lvl_width = val;
    }

    CMD_RTN_FMT("lvlwidth: %d", indent_lvl_width);
)

CMD_FUNCT(tabwidth,
    CMD_MAX_ARGS(1);

    if (CMD_NARGS)
    {
        int val;
        CMD_ARG_PARSE(1, "%d", &val);

        if (val > INDENT_MAX_TABWIDTH || val < 0)
            CMD_RTN("err: Width out of range, ");

        else
            indent_tab_width = val;
    }

    CMD_RTN_FMT("tabwidth: %d", indent_tab_width);
)

CMD_FUNCT(indentmode,
    size_t argind;

    for (argind = 1; argind <= CMD_NARGS; ++argind)
    {
        if (argind > 1) CMD_RTN(", ");

        CMD_ARG_STR(argind, arg);

        indent_mode_arg(vec_first(arg), rtn);
    }
)

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

CMD_FUNCT(incrindent,
    CMD_MAX_ARGS(0);

    ssize_t depth;

    w->pri = indent_incr_depth(w->b, w->pri);
    updater_line(w->b, (cur){ .ln = w->pri.ln });

    depth = indent_get_depth(w->b, w->pri);
    CMD_RTN_FMT("depth: %ld", depth);
)

CMD_FUNCT(decrindent,
    CMD_MAX_ARGS(0);

    ssize_t depth;

    w->pri = indent_decr_depth(w->b, w->pri);
    updater_line(w->b, (cur){ .ln = w->pri.ln });

    depth = indent_get_depth(w->b, w->pri);
    CMD_RTN_FMT("depth: %ld", depth);
)

CMD_FUNCT(autoindent,
    CMD_MAX_ARGS(0);

    ssize_t depth;

    w->pri = indent_auto_depth(w->b, w->pri);
    updater_line(w->b, (cur){ .ln = w->pri.ln });

    depth = indent_get_depth(w->b, w->pri);
    CMD_RTN_FMT("depth: %ld", depth);
)

CMD_FUNCT(indent,
    CMD_MAX_ARGS(1);

    ssize_t depth;

    if (CMD_NARGS)
    {
        CMD_ARG_PARSE(1, "%ld", &depth);

        if (depth > INDENT_MAX_SENSIBLE_DEPTH || depth < 0)
            CMD_RTN("err: Depth out of range");

        else
        {
            ssize_t origdepth;
            cur pretend, rel[2];
            cur *affect[2] = { &(w->pri), &(w->sec) };

            origdepth = indent_get_depth(w->b, w->pri);
            pretend = (cur){ .ln = w->pri.ln, .cn = origdepth };

            cur_get_rel_pos(pretend, w->b, affect, 2, rel);
            indent_set_depth(w->b, w->pri, depth);

            pretend.cn = depth;
            cur_set_rel_pos(pretend, w->b, affect, 2, rel);

            updater_line(w->b, (cur){ .ln = w->pri.ln });
        }
    }

    depth = indent_get_depth(w->b, w->pri);
    CMD_RTN_FMT("depth: %ld", depth);
)

void cmd_indent_init(void)
{
    CMD_ADD(lvlwidth,
        Set the indent level width,
        "The level width is the depth of an indent level, i.e. how far a\n"
        "line is indented when the tab key is pressed. This is not\n"
        "always the width of one tab.\n\n"

        "The command takes a new width as a single argument, but even if\n"
        "this is not given, the command prints out the current value.\n"
    );

    CMD_ADD(tabwidth,
        Set the tab width,
        "The tab width is the width that tabs are displayed as.\n\n"

        "The command takes a new width as a single argument, but even if\n"
        "this is not given, the command prints out the current value.\n"
    );

    CMD_ADD(indentmode,
        Set indent modes,
        "Various indent modes can be set:\n"
        " * `spacify` - By default, indentation to a specific depth is\n"
        "    achieved using tabs as much as possible, and spaces as needed\n"
        "    after the tabs. If spacify mode is enabled, then only spaces\n"
        "    are used.\n"
        " * `auto` - If this mode is active, when the enter key is pressed,\n"
        "    the new line is automatically indented to the same indentation\n"
        "    level as the previous line.\n"
        " * `trim` - When the enter key is pressed, the trailing whitespace\n"
        "    on the current line is trimmed.\n"
        " * `skipblanks` - Blank lines are not considered when calculating\n"
        "    the automatic indent, and so the automatic level is the same as\n"
        "    the previous non-blank line. A blank line is one consisting only\n"
        "    of whitespace.\n\n"

        "To set a mode, give it as an argument, and to unset a mode, give it\n"
        "as an argument prefixed with an '!'.\n"
    );

    CMD_ADD(incrindent,
        Indent a line,
        "Increase the indentation of the current line to the next\n"
        "indentation level as specified by lvlwidth.\n"
    );

    CMD_ADD(decrindent,
        Un-indent a line,
        "Decrease the indentation of the current line to the previous\n"
        "indentation level as specified by lvlwidth.\n"
    );

    CMD_ADD(autoindent,
        Automatically indent a line,
        "Set the indent of the current line to the indentation of the\n"
        "previous line.\n"
    );

    CMD_ADD(indent,
        Set the indentation depth of a line,
        "If a number is given as an argument, then indent the current line\n"
        "such that it begins with that number of columns of whitespace.\n\n"

        "Even if an argument is not given, the command returns the current\n"
        "indentation depth of the current line.\n"
    );
}
