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

static int file_switch_if_found(win *w, vec *fname, vec *rtn);
static buf *file_find(file *f);

void file_cmd_new(vec *rtn, vec *args, win *w)
{
    buf *b;

    if (file_switch_if_found(w, vec_get(args, 1), rtn))
    {
        return;
    }

    b = ring_new();
    win_set_buf(w, b);

    chr_format(rtn, "New buffer [%d]", ring_get_ind(b));

    if (vec_len(args) != 1)
    {
        chr_from_str(rtn, ", ");
        file_cmd_load(rtn, args, w);
    }
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

    if (file_switch_if_found(w, vec_get(args, 1), rtn))
    {
        return;
    }

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

        win_reset(w);
    }
    else
        chr_from_str(rtn, "err: No associated file");
}

static int file_switch_if_found(win *w, vec *fname, vec *rtn)
{
    file f;
    buf *found, *prev;

    file_init(&f);

    if (!fname) return 0;

    if (file_assoc(&f, fname) == -1)
    {
        file_kill(&f);
        return 0;
    }

    found = file_find(&f);
    file_kill(&f);

    if (found)
    {
        prev = w->b;
        win_set_buf(w, found);

        chr_format(
            rtn,
            "Switched buffer %d -> %d",
            ring_get_ind(prev), ring_get_ind(w->b)
        );

        return 1;
    }

    return 0;
}

static buf *file_find(file *f)
{
    size_t ind, len;

    len = vec_len(&ring_bufs);
    for (ind = 0; ind < len; ++ind)
    {
        buf **b;
        file *foth;

        b = vec_get(&ring_bufs, ind);
        foth = &(*b)->finfo;

        if (foth == f) continue;

        if (strcmp(file_name(f), file_name(foth)) == 0)
            return *b;
    }

    return NULL;
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
        chr_format(rtn, "err: [%d] %s", errno, strerror(errno));
    else
        chr_format(rtn, "cwd: %s", cwd);

    free(cwd);
}

void file_clr_win(win *w)
{
    cur loc;

    win_reset(w);

    loc.ln = buf_len(w->b);
    loc.cn = 0;

    while ((loc.ln)--)
        buf_del_line(w->b, loc);
}
