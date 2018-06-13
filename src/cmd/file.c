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

CMD_FUNCT(load,
    CMD_MAX_ARGS(1);

    buf *b;
    file *f;

    b = w->b;
    f = &(b->finfo);

    if (CMD_NARGS && file_switch_if_found(w, vec_get(args, 1), rtn))
        return;

    if (b->flags & buf_modified) CMD_ERR("Buffer modified");
    if (b->flags & buf_readonly) CMD_ERR("Buffer read-only");
    if (b->flags & buf_nofile)   CMD_ERR("Buffer cannot be a file");

    if (CMD_NARGS)
    {
        CMD_ARG(1, path);

        if (file_assoc(f, path) == -1)
            CMD_ERR_FMT(
                "Could not parse path - [%d] %s",
                errno, strerror(errno)
            );
    }

    if (!file_associated(f))
        CMD_ERR("No associated file");

    if (!file_exists(f))
    {
        CMD_RTN("New file");
        file_clr_win(w);
    }
    else if (file_load(f, b) == -1)
    {
        CMD_ERR_FMT(
            "Could not load file - [%d] %s",
            errno, strerror(errno)
        );
    }
    else
        CMD_RTN_FMT("Loaded '%s'", file_name(f));

    win_reset(w);
)

CMD_FUNCT(new,
    CMD_MAX_ARGS(1);

    buf *b;

    if (CMD_NARGS && file_switch_if_found(w, vec_get(args, 1), rtn))
        return;

    b = ring_new();
    win_set_buf(w, b);

    CMD_RTN_FMT("New buffer (%d)", ring_get_ind(b));

    if (CMD_NARGS)
    {
        CMD_RTN(", ");
        cmd_funct_load(args, rtn, w);
    }
)

CMD_FUNCT(discard,
    CMD_MAX_ARGS(0);

    if (w->b->flags & buf_readonly)
        CMD_ERR("Buffer read-only");

    file_clr_win(w);
    w->b->flags &= ~buf_modified;
    CMD_RTN("Contents of buffer discarded");

    win_out_after(w, (cur){0, 0});
)

CMD_FUNCT(associate,
    CMD_MAX_ARGS(1);

    file *f;
    f = &(w->b->finfo);

    if (w->b->flags & buf_nofile) CMD_ERR("Buffer cannot be a file");

    if (CMD_NARGS)
    {
        CMD_ARG(1, path);

        if (file_assoc(f, path) == -1)
            CMD_ERR_FMT(
                "Could not parse path - [%d] %s",
                errno, strerror(errno)
            );
    }

    if (!file_associated(f))
        CMD_ERR("No associated file");

    CMD_RTN_FMT("file: '%s'", file_name(f));
)

CMD_FUNCT(saveall,
    CMD_MAX_ARGS(1);

    int numsaved, force;
    numsaved = 0;
    force    = 0;

    if (CMD_NARGS == 1)
    {
        if (CMD_ARG_IS(1, "!"))
            force = 1;
        else
            CMD_ERR("saveall only takes '!' as an argument.");
    }

    RING_FOREACH(b,
        file *f;
        f = &(b->finfo);

        if (!file_associated(f))
            continue;

        if (!(b->flags & buf_modified) && !force)
            continue;

        if (file_save(f, b) == -1)
        {
            win_set_buf(w, b);
            CMD_ERR_FMT(
                "Could not write '%s': [%d] %s",
                file_name(f), errno, strerror(errno)
            );
        }

        ++numsaved;

        b->flags &= ~buf_modified;
    )

    CMD_RTN_FMT("Saved %d files", numsaved);
)

CMD_FUNCT(save,
    CMD_MAX_ARGS(0);

    file *f;
    buf  *b;
    b = w->b;
    f = &(b->finfo);

    if (!file_associated(f))
        CMD_ERR("No associated file");

    if (file_save(f, b) == -1)
        CMD_ERR_FMT(
            "Could not write '%s': [%d] %s",
            file_name(f), errno, strerror(errno)
        );

    CMD_RTN_FMT("Wrote '%s'", file_name(f));

    w->b->flags &= ~buf_modified;
)

CMD_FUNCT(cd,
    CMD_MAX_ARGS(1);
    char cwd[PATH_MAX];

    if (CMD_NARGS)
    {
        file f;
        file_init(&f);

        CMD_ARG(1, path);

        if (file_assoc(&f, path) == -1)
            CMD_RTN_FMT(
                "err: Could not parse path - [%d] %s",
                errno, strerror(errno)
            );

        if (chdir(file_name(&f)) == -1)
            CMD_RTN_FMT(
                "err: Could not change directory - [%d] %s",
                errno, strerror(errno)
            );

        file_kill(&f);
    }

    if (getcwd(cwd, PATH_MAX) == NULL)
        CMD_ERR_FMT("Could not get cwd - [%d] %s", errno, strerror(errno));

    chr_format(rtn, "cwd: %s", cwd);
)


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

void file_clr_win(win *w)
{
    cur loc;

    win_reset(w);

    loc.ln = buf_len(w->b);
    loc.cn = 0;

    while ((loc.ln)--)
        buf_del_line(w->b, loc);
}

void cmd_file_init(void)
{
    CMD_ADD(new,
        Create a new buffer,
        "Create a new buffer and switch to it. Optionally, open a new\n"
        "file in the new buffer.\n\n"

        "The argument is a filename, which is associated with the buffer\n"
        "and loaded to it, just as if the [load](#load-command) command\n"
        "had been run. i.e. `new file1` is equivalent to `new` followed\n"
        "by `load file1`.\n\n"

        "If the file does not exist, it is created, and if it is already\n"
        "open, the command simply switches to the buffer where it is open\n"
        "instead of creating a new one.\n\n"
    );

    CMD_ADD(discard,
        Discard the contents of a buffer,
        "This command is useful when you don't want to save the contents of\n"
        "a buffer. It deletes the contents of the current buffer and removes\n"
        "its modified flag. The buffer can then be killed without edil\n"
        "complaining.\n"
    );

    CMD_ADD(load,
        Load a file to a buffer,
        "This command will load the contents of an associated file to the\n"
        "current buffer, or reload them if they are already loaded.\n\n"

        "The command takes one argument optionally, which is the path of\n"
        "a file to associate with the buffer before loading, just as if the\n"
        "[associate](#associate-command) command had been run. i.e.\n"
        "`load file1` is equivalent to `associate file1` followed by\n"
        "`load`.\n\n"

        "If the file specified to the command does not exist, it is created,\n"
        "and if it is already open, the command simply switches to the\n"
        "buffer where it is open instead of opening it twice.\n"
    );

    CMD_ADD(associate,
        Associate a buffer with a file,
        "If an argument is given, then that file is associated with the\n"
        "current buffer. This means that the [save](#save-command), and\n"
        "[load](#load-command) will work for the associated file.\n\n"

        "The contents of the buffer are unaffected by this command, so it\n"
        "is, for example, possible to copy a file:\n"
        "```\n"
        "new \"file1\"\n"
        "associate \"file2\"\n"
        "save\n"
        "```\n"
        "This will copy file1 to file2.\n\n"

        "Regardless of whether any argument is given, the file associated\n"
        "with the current buffer is returned.\n"
    );

    CMD_ADD(save,
        Save a buffer to a file,
        "Save the contents of the current buffer to the file associated\n"
        "with it.\n\n"
        "This command takes no arguments\n"
    );

    CMD_ADD(saveall,
        Save all buffers,
        "Performs the equivalent of the [save](#save-command) command to all\n"
        "buffers, where they are modified, and associated with a file.\n\n"

        "This command takes no arguments and returns the number of files\n"
        "that have been saved.\n"
    );

    CMD_ADD(cd,
        Change directory,
        "Change Edil's current working directory to one specified as an\n"
        "argument. Even if no argument is given, the command returns the\n"
        "current working directory.\n"
    );
}
