#include "file.h"
#include "buf.h"
#include "ring.h"
#include "ui.h"

#include "cmd/buf.h"

void buf_cmd_info(vec *rtn, vec *args, win *w)
{
    buf  *b;
    file *f;

    b = w->b;
    f = &(b->finfo);

    if (file_associated(f))      chr_from_str(rtn, "assoc ");
    if (f->flags & file_cr)      chr_from_str(rtn, "cr ");
    if (f->flags & file_pipe)    chr_from_str(rtn, "pipe ");
    if (b->flags & buf_readonly) chr_from_str(rtn, "ro ");
    if (b->flags & buf_modified) chr_from_str(rtn, "mod ");
    if (b->flags & buf_nofile)   chr_from_str(rtn, "nofile ");
    if (b->flags & buf_nokill)   chr_from_str(rtn, "nokill ");

    chr_from_str(rtn, file_name(f));
}

void buf_cmd_next(vec *rtn, vec *args, win *w)
{
    buf *prev;
    prev = w->b;

    win_set_buf(w, ring_next(prev));

    chr_format(
        rtn,
        "switched buffer %d -> %d",
        ring_get_ind(prev), ring_get_ind(w->b)
    );
}

void buf_cmd_prev(vec *rtn, vec *args, win *w)
{
    buf *prev;
    prev = w->b;

    win_set_buf(w, ring_prev(prev));

    chr_format(
        rtn,
        "switched buffer %d -> %d",
        ring_get_ind(prev), ring_get_ind(w->b)
    );
}

void buf_cmd_kill(vec *rtn, vec *args, win *w)
{
    int delind;
    buf *todel, *new;

    if (vec_len(args) > 2)
    {
        chr_from_str(rtn, "err: Too many arguments");
        return;
    }

    else if (vec_len(args) == 2)
    {
        if (chr_scan(vec_get(args, 1), "%d", &delind) != 1)
        {
            chr_from_str(rtn, "err: Index not a valid number");
            return;
        }
        else if (delind < 0 || delind >= (int)vec_len(&ring_bufs))
        {
            chr_from_str(rtn, "err: Index out of range");
            return;
        }
    }
    else
    {
        delind = ring_get_ind(w->b);
    }

    todel = *(buf **)vec_get(&ring_bufs, delind);

    if (w->b->flags & buf_modified)
    {
        chr_format(rtn, "err: Buffer modified");
        return;
    }

    if (todel->flags & buf_nokill)
    {
        chr_from_str(rtn, "err: Buffer not killable");
        return;
    }

    new   = ring_del(todel);

    if (todel == w->b)
    {
        win_set_buf(w, new);
        win_reset(w);
    }

    if (new == todel)
        chr_from_str(rtn, "err: Could not kill buffer");

    else
        chr_format(
            rtn,
            "Deleted buffer %d, switched to %d",
            delind, ring_get_ind(new)
        );
}

void buf_cmd_quit(vec *rtn, vec *args, win *w)
{
    size_t ind, len;

    if (vec_len(args) != 1)
    {
        chr_from_str(rtn, "err: Command takes no arguments");
        return;
    }

    len = vec_len(&ring_bufs);
    for (ind = 0; ind < len; ++ind)
    {
        buf **b;
        b = vec_get(&ring_bufs, ind);
        if ((*b)->flags & buf_modified)
        {
            chr_format(rtn, "err: Buf %ld modified", ind);
            win_set_buf(w, *b);
            return;
        }
    }

    ui_alive = 0;
}
