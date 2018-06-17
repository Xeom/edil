#include "text/buf.h"
#include "win.h"
#include "text/cur.h"
#include "updater.h"

#include "cmd.h"
#include "cmd/nav.h"

CMD_FUNCT(goto,
    CMD_MAX_ARGS(2);

    if (CMD_NARGS == 2)
    {
        CMD_ARG_PARSE(2, "%ld", &(w->pri.cn))
        w->pri.cn -= 1;
    }

    if (CMD_NARGS >= 1)
    {
        CMD_ARG_PARSE(1, "%ld", &(w->pri.ln))
        w->pri.ln -= 1;
    }

    cur_chk_bounds(&(w->pri), w->b);
    cur_chk_blank(&(w->pri), w->b, (cur){0, 0});

    updater_after(w->b, (cur){0, 0});
    win_show_cur(w, w->pri);

    CMD_RTN_FMT(
        "Cursor at line %ld, col %ld.",
        w->pri.ln + 1, w->pri.cn + 1
    );
)

CMD_FUNCT(swap,
    cur tmp;

    CMD_MAX_ARGS(0);

    tmp    = w->pri;
    w->pri = w->sec;
    w->sec = tmp;

    CMD_RTN("Cursors swapped");

    updater_line(w->b, w->pri);
    updater_line(w->b, w->sec);
)

CMD_FUNCT(snap,
    cur prev;

    CMD_MAX_ARGS(0);

    prev   = w->sec;
    w->sec = w->pri;

    CMD_RTN("Secondary cursor snapped");

    updater_line(w->b, w->pri);
    updater_line(w->b, prev);
)

CMD_FUNCT(lineify,
    cur prevsec;
    ssize_t len;

    CMD_MAX_ARGS(0);

    prevsec = w->sec;

    len = buf_line_len(w->b, w->pri);

    w->pri.cn = 0;
    w->sec.cn = len;
    w->sec.ln = w->pri.ln;

    updater_line(w->b, w->pri);

    CMD_RTN("Line selected");

    if (prevsec.ln != w->sec.ln)
        updater_line(w->b, prevsec);
)

void cmd_nav_init(void)
{
    CMD_ADD(goto,
        Go to a specific line and column,
        " * If given, the first argument specifies the line number to go to.\n"
        " * If given, the second argument specifies the column number to go\n"
        "   to.\n"
        "Once the command is run, cursor's column and line number are\n"
        "returned and printed. Both column and line numbers for this\n"
        "command are indexed starting at one.\n"
    )

    CMD_ADD(snap,
        Snap the secondary cursor,
        "Move the secondary cursor to the position of the primary cursor.\n"
    )

    CMD_ADD(swap,
        Swap the cursors,
        "Move the primary cursor to the secondary cursor, and vice versa.\n"
        "The primary and secondary cursors swap places.\n"
    )

    CMD_ADD(lineify,
        Select the current line,
        "Move the primary cursor to the beginning of the current line, and\n"
        "the secondary cursor to the end. This places the whole line in the\n"
        "region, and allows for it to easily be cut or copied etc.\n"
    )
}
