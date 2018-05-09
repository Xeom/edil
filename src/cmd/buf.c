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

    chr_from_str(rtn, file_name(f));
}

void buf_cmd_next(vec *rtn, vec *args, win *w)
{
    buf *prev;
    prev = w->b;

    w->b->prihint = w->pri;
    
    w->b = ring_next(prev);

    win_reset(w);

    win_out_after(w, (cur){0, 0});
    chr_format(rtn, "switched buffer %d -> %d", ring_get_ind(prev), ring_get_ind(w->b));
}

void buf_cmd_prev(vec *rtn, vec *args, win *w)
{
    buf *prev;
    prev = w->b;

    w->b->prihint = w->pri;
    
    w->b = ring_prev(prev);

    win_reset(w);
    
    win_out_after(w, (cur){0, 0});
    chr_format(
        rtn,
        "switched buffer %d -> %d",
        ring_get_ind(prev), ring_get_ind(w->b)
    );
}
