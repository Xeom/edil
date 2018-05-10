#include "vec.h"
#include "win.h"
#include "cmd.h"

#include "cmd/nav.h"

cmd_info cmd_info_goto =
{
    .fptr = nav_cmd_goto,
    .name = "goto",
    .desc = "Go to a specific location in the buffer",
    .notes = "After the command is run, it prints out the"
             " cursor's new location, even if it is given"
             " no arguments. If the specified new location"
             " is not a valid location in the buffer, the"
             " nearest valid location is used instead.",
    .nargs = 2,
    .args =
    {
        {
            .optional = 1,
            .name = "Line number",
            .desc = "The number of the line to go to."
                    " (starting from 1)"
        },
        {
            .optional = 1,
            .name = "Column number",
            .desc = "The number of the column to go to."
                    " (starting from 1)"
        }
    }
};

void nav_cmd_goto(vec *rtn, vec *args, win *w)
{
    if (vec_len(args) > 3)
    {
        chr_format(rtn, "err: This command takes up to two arguments only");
        return;
    }

    if (vec_len(args) == 3)
    {
        if (chr_scan(vec_get(args, 2), "%ld", &(w->pri.cn)) != 1)
        {
            chr_from_str(rtn, "err: Could not parse column number");
            return;
        }
        w->pri.cn -= 1;
    }

    if (vec_len(args) >= 2)
    {
        if (chr_scan(vec_get(args, 1), "%ld", &(w->pri.ln)) != 1)
        {
            chr_from_str(rtn, "err: Could not parse line number");
            return;
        }
        w->pri.ln -= 1;
    }

    w->pri = cur_check_bounds(w->pri, w->b);
    w->pri = cur_check_blank(w->pri, w->b, (cur){0, 0});

    win_out_after(w, (cur){0, 0});
    win_show_cur(w, w->pri);
    
    chr_format(
        rtn,
        "Cursor at line %ld, col %ld.",
        w->pri.ln + 1, w->pri.cn + 1
    );
}

cmd_info cmd_info_swap =
{
    .fptr = nav_cmd_swap,
    .name = "swap",
    .desc = "Swap the positions of the primary and secondary cursors",
    .nargs = 0
};

void nav_cmd_swap(vec *rtn, vec *args, win *w)
{
    cur tmp;

    if (vec_len(args) != 1)
    {
        chr_from_str(rtn, "err: This command takes no arguments");
        return;
    }
    else
    {
        chr_from_str(rtn, "Cursors swapped");
    }

    tmp    = w->pri;
    w->pri = w->sec;
    w->sec = tmp;

    win_out_line(w, w->pri);
    win_out_line(w, w->sec);
}

cmd_info cmd_info_snap =
{
    .fptr = nav_cmd_snap,
    .name = "snap",
    .desc = "Move the secondary cursor to the primary cursors",
    .nargs = 0
};

void nav_cmd_snap(vec *rtn, vec *args, win *w)
{
    cur prev;

    if (vec_len(args) != 1)
    {
        chr_from_str(rtn, "err: This command takes no arguments");
        return;
    }
    else
    {
        chr_from_str(rtn, "Secondary cursor snapped");
    }

    prev   = w->sec;
    w->sec = w->pri;

    win_out_line(w, w->pri);
    win_out_line(w, prev);
}
