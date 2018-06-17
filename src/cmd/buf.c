#include <string.h>

#include "text/buf.h"
#include "file.h"
#include "ring.h"
#include "win.h"
#include "ui.h"

#include "cmd.h"
#include "cmd/buf.h"

static buf *cmd_buf_find(buf *orig, char *search);

CMD_FUNCT(buffer,
    CMD_MAX_ARGS(1);

    static buf *prev = NULL;
    buf *b;

    if (CMD_NARGS == 0)
    {
        if (!prev)
            CMD_ERR("No previous buffer to switch back to");

        b = prev;
    }

    else
    {
        prev = w->b;

        CMD_ARG_STR(1, search)
        b = cmd_buf_find(w->b, vec_first(search));

        if (b == NULL)
            CMD_ERR("Could not find matching buffer.");

    }

    prev = w->b;

    win_set_buf(w, b);

    CMD_RTN_FMT(
        "switched buffer %d -> %d",
        ring_get_ind(prev), ring_get_ind(b)
    );
)

static buf *cmd_buf_find(buf *orig, char *search)
{
    int id;
    buf *b;
    if (sscanf(search, "%d", &id) == 1)
    {
        return ring_get(id);
    }

    for (b = ring_next(orig); b != orig; b = ring_next(b))
    {
        if (strstr(buf_get_name(b), search))
            return b;
    }

    for (b = ring_next(orig); b != orig; b = ring_next(b))
    {
        if (strstr(file_name(&(b->finfo)), search))
            return b;
    }

    return NULL;
}

CMD_FUNCT(bufinfo,

    buf  *b;
    file *f;

    CMD_MAX_ARGS(0);

    b = w->b;
    f = &(b->finfo);

    if (file_associated(f))      CMD_RTN(" assoc");
    if (f->flags & file_cr)      CMD_RTN(" cr");
    if (f->flags & file_pipe)    CMD_RTN(" pipe");
    if (f->flags & file_eofnl)   CMD_RTN(" eofnl");
    if (b->flags & buf_readonly) CMD_RTN(" ro");
    if (b->flags & buf_modified) CMD_RTN(" mod");
    if (b->flags & buf_nofile)   CMD_RTN(" nofile");
    if (b->flags & buf_nokill)   CMD_RTN(" nokill");

    CMD_RTN_FMT(" %d [%s] [%s]", ring_get_ind(b), buf_get_name(b), file_name(f));

    if (vec_len(rtn)) vec_del(rtn, 0, 1);
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
    buf *new = NULL;
    force = 0;

    CMD_TMP_VEC(killbufs, buf *);

    for (argind = 1; argind <= CMD_NARGS; ++argind)
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

            if (!b) CMD_ERR("index out of range");

            if (!force && (b->flags & buf_modified))
                CMD_ERR_FMT("Buffer %d modified", ind);

            if (b->flags & buf_nokill)
                CMD_ERR_FMT("Buffer %d cannot be killed", ind);

            vec_app(killbufs, &b);
        }
    }

    if (vec_len(killbufs) == 0)
    {
        if (!force && (w->b->flags & buf_modified))
            CMD_ERR("Current buffer modified");

        if (w->b->flags & buf_nokill)
            CMD_ERR("Current buffer cannot be killed");

        vec_app(killbufs, &(w->b));
    }

    VEC_FOREACH(killbufs, buf **, bptr,
        buf *b;
        int ind;

        b   = *bptr;
        ind = ring_get_ind(b);
        new = ring_del(b);

        if (new == b)
        {
            win_set_buf(w, b);
            CMD_ERR("Could not delete buffer");
        }
        else
        {
            CMD_RTN_FMT("Deleted buffer %d, ", ind);
        }
    );

    win_set_buf_killed(w, new);

    CMD_RTN_FMT("switched to %d", ring_get_ind(new));

)

CMD_FUNCT(quit,
    int force;

    CMD_MAX_ARGS(1);

    force = 0;

    if (CMD_NARGS)
    {
        if (CMD_ARG_IS(1, "!"))
            force = 1;
        else
            CMD_ERR("quit can only take '!' as an argument");
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
    CMD_ADD(buffer,
        Switch to a specified buffer,
        "This command switches the current window to a specified buffer.\n"
        "It takes one argument, which is used to search for a matching\n"
        "buffer. If no argument is given, the it goes back to the\n"
        "buffer that was previously navigated away from using this\n"
        "command.\n\n"

        "If the command is given a number, then the buffer with that id is\n"
        "chosen.\n\n"

        "Then, the names of every buffer are checked. These are generally\n"
        "basenames of the files associated with those buffers.\n\n"

        "Finally, the full path of every buffer is checked.\n\n"

        "The buffer that is switched to will be the first one after the\n"
        "current buffer that matches.\n"
    );

    CMD_ADD(bufinfo,
        Display information about the current buffer,
        "If a file is associated with the buffer, the path of\n"
        "that file is also returned, along with the buffer's\n"
        "index number and the buffer's name.\n\n"

        "Prints flags associated with the buffer:\n"
        " * `assoc` - The buffer is associated with a pipe or file.\n"
        " * `cr` - The buffer uses '\r\n' style newlines.\n"
        " * `pipe` - The buffer is associated with a pipe.\n"
        " * `ro` - The buffer is read-only.\n"
        " * `mod` - The buffer has been modified.\n"
        " * `nofile` - No file can be associated with this buffer.\n"
        " * `nokill` - This buffer cannot be killed.\n"
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
        "If `!` is given as an argument, the buffers will be\n"
        "force-killed, even if they are modified.\n"
    );

    CMD_ADD(quit,
        Quit Edil,
        "Exit Edil, if no buffers are modified. If `!` is given as\n"
        "an argument, then Edil will exit even if buffers are\n"
        "modified\n"
    );
}
