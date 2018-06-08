#include "ring.h"
#include "cmd.h"

#include "cmd/region.h"

buf *region_clipboard = NULL;

CMD_FUNCT(copy,
    CMD_MAX_ARGS(0);

    cur loc, start, end;

    if (region_clipboard == NULL)
    {
        region_clipboard = ring_new();
        region_clipboard->flags |= buf_nofile;
        buf_set_name(region_clipboard, "'clipboard'");

        CMD_RTN_FMT(
            "Created clipboard buffer [%d], ",
            ring_get_ind(region_clipboard)
        );
    }

    else if (region_clipboard == w->b)
        CMD_ERR("Cannot copy clipboard");

    loc = (cur){ .ln = buf_len(region_clipboard) };

    while ((loc.ln)--)
        buf_del_line(region_clipboard, loc);

    start = CUR_START(w->pri, w->sec);
    end   = CUR_END(w->pri,   w->sec);

    if (end.cn < buf_line_len(w->b, end))
        end.cn += 1;

    buf_ins_buf(region_clipboard, &(cur){0, 0}, w->b, start, end);

    CMD_RTN_FMT(
        "Copied %lu lines to clipboard",
        buf_len(region_clipboard)
    );
)

CMD_FUNCT(paste,
    CMD_MAX_ARGS(1)

    buf *source;

    if (CMD_NARGS)
    {
        int ind, maxind;
        buf **b;

        maxind = vec_len(&ring_bufs) - 1;

        CMD_ARG_PARSE(1, "%d", &ind);

        if (ind < 0 || ind > maxind)
            CMD_ERR("Invalid index to paste");

        b = vec_get(&ring_bufs, ind);
        source = *b;
    }
    else
    {
        if (!region_clipboard) CMD_ERR("There is no clipboard");

        source = region_clipboard;
    }

    if (w->b == source) CMD_ERR("Cannot paste buffer into itself");

    cur_ins_buf(w, source);

    CMD_RTN_FMT("Pasted %lu lines.", buf_len(source));
)

CMD_FUNCT(cut,
    CMD_MAX_ARGS(0);

    cmd_funct_copy(rtn, args, w);

    if (region_clipboard != w->b)
    {
        cur_del_region(w);
        CMD_RTN(", Deleted region");
    }
)

void cmd_region_init(void)
{
    CMD_ADD(copy, Copy the current region, "");
    CMD_ADD(paste, Paste a buffer, "");
    CMD_ADD(cut, Cut the current region, "");
}