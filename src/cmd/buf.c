#include <string.h>

#include "file.h"
#include "buf.h"
#include "ring.h"
#include "ui.h"
#include "cmd.h"

#include "cmd/buf.h"

CMD_FUNCT(bufinfo,

    buf  *b;
    file *f;

    CMD_MAX_ARGS(0);

    b = w->b;
    f = &(b->finfo);

    CMD_RTN_FMT("%d [%s] ", ring_get_ind(b), file_name(f));

    if (file_associated(f))      CMD_RTN("assoc ");
    if (f->flags & file_cr)      CMD_RTN("cr ");
    if (f->flags & file_pipe)    CMD_RTN("pipe ");
    if (b->flags & buf_readonly) CMD_RTN("ro ");
    if (b->flags & buf_modified) CMD_RTN("mod ");
    if (b->flags & buf_nofile)   CMD_RTN("nofile ");
    if (b->flags & buf_nokill)   CMD_RTN("nokill ");

)

CMD_FUNCT(next,
    buf *prev;
    prev = w->b;

    CMD_MAX_ARGS(0);

    win_set_buf(w, ring_next(prev));

    CMD_RTN_FMT(
        "switched buffer %d -> %d",
        ring_get_ind(prev), ring_get_ind(w->b)
    );
)

CMD_FUNCT(prev,
    buf *prev;
    prev = w->b;

    CMD_MAX_ARGS(0);

    win_set_buf(w, ring_prev(prev));

    CMD_RTN_FMT(
        "switched buffer %d -> %d",
        ring_get_ind(prev), ring_get_ind(w->b)
    );
)

CMD_FUNCT(kill,
    int force, ind;
    size_t argind;
    buf *new;
    force = 0;

    CMD_TMP_VEC(killbufs, buf *);

    for (argind = 1; argind < CMD_NARGS; ++argind)
    {
        if (CMD_ARG_IS(argind, "!"))
        {
            force = 1;
        }
        else
        {
            buf *b;

            CMD_ARG_PARSE(argind, "%d", &ind);
            b = ring_get(ind);

            if (!b)
            {
                CMD_RTN("err: index out of range");
                return;
            }

            if (!force && (b->flags & buf_modified))
            {
                CMD_RTN_FMT("err: Buffer %d modified", ind);
                return;
            }

            if (b->flags & buf_nokill)
            {
                CMD_RTN_FMT("err: Buffer %d cannot be killed", ind);
                return;
            }

            vec_app(killbufs, &b);
        }
    }

    if (vec_len(killbufs) == 0)
    {
        if (!force && (w->b->flags & buf_modified))
        {
            CMD_RTN_FMT("err: Current buffer modified", ind);
            return;
        }

        if (w->b->flags & buf_nokill)
        {
            CMD_RTN_FMT("err: Current buffer cannot be killed", ind);
            return;
        }

        vec_app(killbufs, &(w->b));
    }

    VEC_FOREACH(killbufs, bptr,
        buf *b;
        b = *(buf **)bptr;

        new = ring_del(b);

        if (new == b)
        {
            CMD_RTN("err: Could not delete buffer");
            win_set_buf(w, b);
            return;
        }
        else
        {
            CMD_RTN_FMT("Deleted buffer %d, ", ring_get_ind(b));
        }
    );

    win_set_buf(w, new);

    CMD_RTN_FMT("switched to %d", ring_get_ind(new));

)

CMD_FUNCT(quit,
    int force = 0;

    CMD_MAX_ARGS(1);

    if (vec_len(args) > 2)
    {
        chr_from_str(rtn, "err: Command maximum one argument");
        return;
    }

    force = 0;

    if (vec_len(args) == 2)
    {
        if (chr_cmp_str(vec_get(args, 1), "!") == 0)
            force = 1;
        else
        {
            chr_from_str(rtn, "err: quit can only take '!' as an argument");
            return;
        }
    }

    if (!force)
    {
        RING_FOREACH(b,
            if (b->flags & buf_modified)
            {
                CMD_RTN_FMT("err: Buf %ld modified", ring_get_ind(b));
                win_set_buf(w, b);
                return;
            }
        )
    }

    ui_alive = 0;
)

void cmd_buf_init(void)
{
    CMD_ADD(bufinfo,
        Display information about the current buffer,
        "If a file is associated with the buffer, the path of\n"
        "that file is also returned, along with the buffer's\n"
        "index number.\n\n"

        "Prints flags associated with the buffer:\n"
        " * assoc - The buffer is associated with a pipe or file.\n"
        " * cr - The buffer uses '\r\n' style newlines.\n"
        " * pipe - The buffer is associated with a pipe.\n"
        " * ro - The buffer is read-only.\n"
        " * mod - The buffer has been modified.\n"
        " * nofile - No file can be associated with this buffer.\n"
        " * nokill - This buffer cannot be killed.\n"
    );

    CMD_ADD(next,
        Go to the next buffer,
        "Go to the buffer with an index one higher than the current\n"
        "buffer, or if there is no such buffer, loop around to the\n"
        "first buffer.\n"
    );

    CMD_ADD(prev,
        Go to the previous buffer,
        "Go to the buffer with an index one lower than the current\n"
        "buffer, or if there is no such buffer, loop around to the\n"
        "final buffer.\n"
    );

    CMD_ADD(kill,
        Kill the current buffer,
        "By default, this command kills the current buffer. If other\n"
        "buffers are given as arguments however, it will kill them\n"
        "instead. Buffers are not killed if they are modified.\n"
        "If '!' is given as an argument, the buffers will be\n"
        "force-killed, even if they are modified.\n"
    );

    CMD_ADD(quit,
        Quit edil,
        "Exit edil, if no buffers are modified. If '!' is given as\n"
        "an argument, then edil will exit even if buffers are\n"
        "modified\n"
    );
}