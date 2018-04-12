#include "vec.h"
#include "win.h"

#include "cmd/nav.h"

void nav_cmd_goto(vec *rtn, vec *args, win *w)
{
    vec strarg;

    if (vec_len(args) > 3)
    {
        chr_format(rtn, "err: This command takes up to two arguments only");
        return;
    }

    if (vec_len(args) == 3)
    {
        vec_init(&strarg, sizeof(char));

        chr_to_str(vec_get(args, 2), &strarg);
        vec_ins(&strarg, vec_len(&strarg), 1, NULL);

        if (sscanf(vec_get(&strarg, 0), "%ld", &(w->pri.cn)) != 1)
        {
            chr_format(
                rtn,
                "err: Could nor parse number '%s'",
                vec_get(&strarg, 0)
            );

            vec_kill(&strarg);
            return;
        }

        w->pri.cn -= 1;

        vec_kill(&strarg);
    }

    if (vec_len(args) >= 2)
    {
        vec_init(&strarg, sizeof(char));

        chr_to_str(vec_get(args, 1), &strarg);
        vec_ins(&strarg, vec_len(&strarg), 1, NULL);

        if (sscanf(vec_get(&strarg, 0), "%ld", &(w->pri.ln)) != 1)
        {
            chr_format(
                rtn,
                "err: Could nor parse number '%s'",
                vec_get(&strarg, 0)
            );

            vec_kill(&strarg);
            return;
        }

        w->pri.ln -= 1;

        vec_kill(&strarg);
    }

    w->pri = cur_check_bounds(w->pri, w->b);
    w->pri = cur_check_blank(w->pri, w->b, (cur){0, 0});

    chr_format(
        rtn,
        "Cursor at line %ld, col %ld.",
        w->pri.ln + 1, w->pri.cn + 1
    );
}

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
