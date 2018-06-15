#include "buf/buf.h"
#include "ring.h"
#include "buf/cur.h"

#include "cmd.h"
#include "cmd/region.h"

buf *region_clipboard = NULL;

CMD_FUNCT(copy,
    CMD_MAX_ARGS(0);

    cur start, end;

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

    buf_clr(region_clipboard);

    start = CUR_START(w->pri, w->sec);
    end   = CUR_END(w->pri,   w->sec);

    if (end.cn < buf_line_len(w->b, end))
        end.cn += 1;

    buf_ins_from(region_clipboard, (cur){0, 0}, w->b, start, end);

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

    buf_ins_from(
        w->b,
        w->pri,
        region_clipboard,
        (cur){0, 0}, buf_last(region_clipboard)
    );

    CMD_RTN_FMT("Pasted %lu lines.", buf_len(source));
)

CMD_FUNCT(cut,
    CMD_MAX_ARGS(0);

    cmd_funct_copy(args, rtn, w);

    if (region_clipboard != w->b)
    {
        cur_del_region(w);
        CMD_RTN(", Deleted region");
    }
)

void cmd_region_init(void)
{
    CMD_ADD(copy,
        Copy the current region,
        "Copy the current region between the primary and secondary cursors\n"
        "the contents of the region is placed into the clipboard buffer,\n"
        "which is created if it does not exist already.\n"
    );

    CMD_ADD(paste,
        Paste a buffer,
        "Insert the contents of another buffer at the location of the cursor\n"
        "By default, the clipboard buffer is inserted, but if an argument is\n"
        "given, it can specify a different buffer index."
    );

    CMD_ADD(cut,
        Cut the current region,
        "This command runs the copy command, and then deletes the region.\n"
    );
}
