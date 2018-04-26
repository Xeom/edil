#include "ring.h"

#include "cmd/region.h"

buf *region_clipboard = NULL;

void region_cmd_copy(vec *rtn, vec *args, win *w)
{
    cur loc, start, end;

    if (region_clipboard == NULL)
    {
        region_clipboard = ring_new();
        region_clipboard->flags |= buf_nofile;
        buf_set_name(region_clipboard, "'clipboard'");

        chr_format(
            rtn,
            "Created clipboard buffer [%d], ",
            ring_get_ind(region_clipboard)
        );
    }

    else if (region_clipboard == w->b)
    {
        chr_from_str(rtn, "err: Cannot copy clipboard");
        return;
    }

    loc = (cur){ .ln = buf_len(region_clipboard) };

    while ((loc.ln)--)
        buf_del_line(region_clipboard, loc);

    start = *cur_region_start(w);
    end   = *cur_region_end(w);

    if (end.cn < buf_line_len(w->b, end))
        end.cn += 1;

    buf_ins_buf(region_clipboard, &(cur){0, 0}, w->b, start, end);

    chr_format(
        rtn,
        "Copied %lu lines to clipboard",
        buf_len(region_clipboard)
    );
}

void region_cmd_paste(vec *rtn, vec *args, win *w)
{
    buf *source;

    if (vec_len(args) == 2)
    {
        int ind, maxind;
        buf **b;

        maxind = vec_len(&ring_bufs) - 1;

        if (chr_scan(vec_get(args, 1), "%d", &ind) != 1 ||
            ind < 0 || ind > maxind)
        {
            chr_from_str(rtn, "err: Not a valid index");
            return;
        }

        b = vec_get(&ring_bufs, ind);
        source = *b;
    }
    else
    {
        if (!region_clipboard)
        {
            chr_from_str(rtn, "err: Nothing has been copied to the clipboard");
            return;
        }

        source = region_clipboard;
    }

    if (w->b == source)
    {
        chr_from_str(rtn, "err: Cannot paste buffer into itself");
        return;
    }

    cur_ins_buf(w, source, (cur){0, 0}, buf_last_cur(source));

    chr_format(
        rtn,
        "Pasted %lu lines.",
        buf_len(source)
    );
}

void region_cmd_cut(vec *rtn, vec *args, win *w)
{
    region_cmd_copy(rtn, args, w);

    if (region_clipboard != w->b)
    {
        cur_del_region(w);
        chr_from_str(rtn, ", Deleted region");
    }
}
