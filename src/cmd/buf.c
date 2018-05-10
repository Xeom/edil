#include "file.h"
#include "buf.h"
#include "ring.h"

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

    w->b->prihint = w->pri;

    w->b = ring_next(prev);

    win_reset(w);

    chr_format(rtn, "switched buffer %d -> %d", ring_get_ind(prev), ring_get_ind(w->b));
}

void buf_cmd_prev(vec *rtn, vec *args, win *w)
{
    buf *prev;
    prev = w->b;

    w->b->prihint = w->pri;

    w->b = ring_prev(prev);

    win_reset(w);

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
        w->b = new;
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