#if !defined _XOPEN_SOURCE
# define _XOPEN_SOURCE 500
# include <stdlib.h>
# include <limits.h>
# undef _XOPEN_SOURCE
#else
# include <stdlib.h>
# include <limits.h>
#endif

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>

#include "cmd.h"
#include "chr.h"
#include "ring.h"
#include "indent.h"
#include "file.h"

#include "cmd/file.h"


buf *file_clipboard = NULL;

void file_cmd_new(vec *rtn, vec *args, win *w)
{
    buf *b;

    b = ring_new();
    w->b = b;

    chr_format(rtn, "New buffer [%d]", ring_get_ind(b));

    if (vec_len(args) != 1)
    {
        chr_from_str(rtn, ", ");
        file_cmd_load(rtn, args, w);
    }

    w->pri = (cur){0, 0};
    w->sec = (cur){0, 0};

    win_out_after(w, (cur){0, 0});
}

void file_cmd_next(vec *rtn, vec *args, win *w)
{
    buf *prev;
    prev = w->b;

    w->b = ring_next(prev);

    w->pri = (cur){0, 0};
    w->sec = (cur){0, 0};

    win_out_after(w, (cur){0, 0});
    chr_format(rtn, "switched buffer %d -> %d", ring_get_ind(prev), ring_get_ind(w->b));
}

void file_cmd_prev(vec *rtn, vec *args, win *w)
{
    buf *prev;
    prev = w->b;

    w->b = ring_prev(prev);

    w->pri = (cur){0, 0};
    w->sec = (cur){0, 0};

    win_out_after(w, (cur){0, 0});
    chr_format(
        rtn,
        "switched buffer %d -> %d",
        ring_get_ind(prev), ring_get_ind(w->b)
    );
}

void file_cmd_copy(vec *rtn, vec *args, win *w)
{
    cur loc, start, end;

    if (file_clipboard == NULL)
    {
        file_clipboard = ring_new();
        file_clipboard->flags |= buf_nofile;

        chr_format(
            rtn,
            "Created clipboard buffer [%d], ",
            ring_get_ind(file_clipboard)
        );
    }

    else if (file_clipboard == w->b)
    {
        chr_from_str(rtn, "err: Cannot copy clipboard");
        return;
    }

    loc = (cur){ .ln = buf_len(file_clipboard) };

    while ((loc.ln)--)
        buf_del_line(file_clipboard, loc);

    start = *cur_region_start(w);
    end   = *cur_region_end(w);

    if (end.cn < buf_line_len(w->b, end))
        end.cn += 1;

    buf_ins_buf(file_clipboard, &(cur){0, 0}, w->b, start, end);

    chr_format(
        rtn,
        "Copied %lu lines to clipboard",
        buf_len(file_clipboard)
    );
}

void file_cmd_paste(vec *rtn, vec *args, win *w)
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
        if (!file_clipboard)
        {
            chr_from_str(rtn, "err: Nothing has been copied to the clipboard");
            return;
        }

        source = file_clipboard;
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

void file_cmd_discard(vec *rtn, vec *args, win *w)
{
    if (vec_len(args) != 1)
    {
        chr_format(rtn, "err: Command takes no arguments");
        return;
    }

    if (w->b->flags & buf_readonly)
    {
        chr_format(rtn, "err: Buffer read-only");
    }
    else
    {
        file_clr_win(w);
        w->b->flags &= ~buf_modified;
        chr_format(rtn, "Contents of buffer discarded");
    }

    win_out_after(w, (cur){0, 0});
}

void file_cmd_load(vec *rtn, vec *args, win *w)
{
    buf  *b;
    file *f;

    b = w->b;
    f = &(b->finfo);

    if (w->b->flags & buf_modified)
    {
        chr_format(rtn, "err: Buffer modified");
        return;
    }

    if (w->b->flags & buf_readonly)
    {
        chr_format(rtn, "err: Buffer read-only");
        return;
    }

    if (w->b->flags & buf_nofile)
    {
        chr_format(rtn, "err: Buffer cannot be a file");
        return;
    }


    if (vec_len(args) == 2)
    {
        vec *path;
        path = vec_get(args, 1);

        if (file_assoc(f, path) == -1)
        {
            chr_format(
                rtn,
                "err Parsing path: [%d] %s",
                errno, strerror(errno)
            );
            return;
        }
    }

    if (file_associated(f))
    {
       if (file_load(f, b) == -1)
       {
            if (errno == ENOENT)
            {
                chr_format(rtn, "New file");
                file_clr_win(w);
            }
            else
                chr_format(
                    rtn,
                    "err Loading '%s': [%d] %s",
                    file_name(f), errno, strerror(errno)
                );
        }
        else
        {
            chr_format(rtn, "Loaded '%s'", file_name(f));
        }

        win_out_after(w, (cur){0, 0});
    }
    else
        chr_from_str(rtn, "err: No associated file");
}

void file_cmd_assoc(vec *rtn, vec *args, win *w)
{
    file *f;
    f = &(w->b->finfo);

    if (w->b->flags & buf_nofile)
    {
        chr_format(rtn, "err: Buffer cannot be a file");
        return;
    }

    if (vec_len(args) == 2)
    {
        vec *path;
        path = vec_get(args, 1);

        if (file_assoc(f, path) == -1)
        {
            chr_format(
                rtn,
                "err Parsing path: [%d] %s",
                errno, strerror(errno)
            );
            return;
        }
    }

    if (file_associated(f))
        chr_format(rtn, "file: '%s'", file_name(f));
    else
        chr_format(rtn, "err: No associated file");
}

void file_cmd_save(vec *rtn, vec *args, win *w)
{
    file *f;
    buf  *b;

    b = w->b;
    f = &(b->finfo);

    if (!file_associated(f))
    {
        chr_format(rtn, "err: No associated file");
        return;
    }

    if (file_save(f, b) == -1)
        chr_format(
            rtn,
            "err Writing '%s': [%d] %s",
            file_name(f), errno, strerror(errno)
        );

    else
        chr_format(rtn, "Wrote '%s'", file_name(f));

    w->b->flags &= ~buf_modified;
}

void file_cmd_chdir(vec *rtn, vec *args, win *w)
{
    char *cwd;
    cwd = malloc(PATH_MAX);

    if (vec_len(args) == 2)
    {
        file f;
        file_init(&f);

        if (file_assoc(&f, vec_get(args, 1)) == -1)
            chr_format(rtn, "err: [%d] %s, ", errno, strerror(errno));

        else if (chdir(file_name(&f)) == -1)
            chr_format(rtn, "err: [%d] %s, ", errno, strerror(errno));

        file_kill(&f);
    }

    if (getcwd(cwd, PATH_MAX) == NULL)
    {
        chr_format(rtn, "err: [%d] %s", errno, strerror(errno));
    }
    else
    {
        chr_format(rtn, "cwd: %s", cwd);
    }

    free(cwd);
}

void file_clr_win(win *w)
{
    cur loc;
    loc.ln = buf_len(w->b);
    loc.cn = 0;

    w->pri = (cur){0, 0};
    w->sec = (cur){0, 0};

    while ((loc.ln)--)
        buf_del_line(w->b, loc);
}
