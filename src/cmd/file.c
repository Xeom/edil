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

#include "cmd/file.h"

static int file_get_fullpath(vec *chrs, vec *fullpath);

static void file_save_line(vec *line, FILE *f);
static void file_load_line(vec *line, FILE *f);

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
}

void file_cmd_load(vec *rtn, vec *args, win *w)
{
    vec *fn;
    fn = &(w->b->fname);

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
        if (file_get_fullpath(path, fn) == -1)
        {
            chr_format(
                rtn,
                "err Parsing path: [%d] %s",
                errno, strerror(errno)
            );
            return;
        }

        w->b->flags |= buf_associated;
    }

    if (w->b->flags & buf_associated)
    {
        FILE *f;
        f = fopen(vec_get(fn, 0), "r");
        if (f == NULL)
        {
            if (errno == ENOENT)
            {
                chr_format(rtn, "New file");
                file_clr_win(w);
            }
            else
                chr_format(
                    rtn,
                    "err Opening '%s': [%d] %s",
                    vec_get(fn, 0), errno, strerror(errno)
                );
        }
        else
        {
            file_clr_win(w);
            file_load_win(w, f);

            w->b->flags &= ~buf_modified;

            if (ferror(f))
            {
                fclose(f);
                chr_format(
                    rtn,
                    "err Reading '%s': [%d] %s",
                    vec_get(fn, 0), errno, strerror(errno)
                );

                return;
            }
            else
            {
                chr_format(rtn, "Loaded '%s'", vec_get(fn, 0));
            }

            fclose(f);
        }
    }
    else
        chr_format(rtn, "err: No associated file");
}

void file_cmd_assoc(vec *rtn, vec *args, win *w)
{
    vec *fn;
    fn = &(w->b->fname);

    if (w->b->flags & buf_nofile)
    {
        chr_format(rtn, "err: Buffer cannot be a file");
        return;
    }

    if (vec_len(args) == 2)
    {
        vec *path;
        path = vec_get(args, 1);
        if (file_get_fullpath(path, fn))
        {
            chr_format(
                rtn,
                "err Parsing path: [%d] %s, ",
                errno, strerror(errno)
            );

            return;
        }

        w->b->flags |= buf_associated;
    }

    if (w->b->flags & buf_associated)
        chr_format(rtn, "file: '%s'", vec_get(fn, 0));
    else
        chr_format(rtn, "err: No associated file");
}

void file_cmd_save(vec *rtn, vec *args, win *w)
{
    FILE *f;
    vec  *fn;

    fn = &(w->b->fname);

    if (!(w->b->flags & buf_associated))
    {
        chr_format(rtn, "err: No associated file");
        return;
    }

    f = fopen(vec_get(fn, 0), "w");

    if (!f)
    {
        chr_format(
            rtn,
            "err Opening '%s': [%d] %s",
            vec_get(fn, 0), errno, strerror(errno)
        );

        return;
    }

    file_save_win(w, f);

    if (ferror(f))
        chr_format(
            rtn,
            "err Writing '%s': [%d] %s",
            vec_get(fn, 0), errno, strerror(errno)
        );

    else
        chr_format(rtn, "Wrote '%s'", vec_get(fn, 0));

    fclose(f);

    w->b->flags &= ~buf_modified;
}

void file_cmd_chdir(vec *rtn, vec *args, win *w)
{
    char *cwd;
    cwd = malloc(PATH_MAX);

    if (vec_len(args) == 2)
    {
        vec dir, *arg;
        vec_init(&dir, sizeof(char));

        arg = vec_get(args, 1);

        if (file_get_fullpath(arg, &dir) == -1)
            chr_format(rtn, "err: [%d] %s, ", errno, strerror(errno));

        else if (chdir(vec_get(&dir, 0)) == -1)
            chr_format(rtn, "err: [%d] %s, ", errno, strerror(errno));

        vec_kill(&dir);
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

void file_save_win(win *w, FILE *f)
{
    size_t numlines, ind;
    vec *lines;

    lines = &(w->b->lines);
    numlines = vec_len(lines);
    for (ind = 0; ind < numlines; ind++)
    {
        if (ind)
        {
            if (fwrite("\n", 1, 1, f) != 1)
                break;
        }

        file_save_line(vec_get(lines, ind), f);

        if (ferror(f)) break;
    }
}

void file_save_line(vec *line, FILE *f)
{
    size_t len, ind;
    len = vec_len(line);

    for (ind = 0; ind < len; ind++)
    {
        size_t width;
        chr *c;
        c = vec_get(line, ind);

        if (!c || chr_is_blank(c)) continue;

        width = chr_len(c);

        if (fwrite(c->utf8, 1, width, f) != width)
            return;
    }
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

static void file_load_line(vec *line, FILE *f)
{
    size_t width, ind;
    chr utfchr = CHR("");
    int c;

    width = 0;
    ind   = 0;

    vec_del(line, 0, vec_len(line));

    while (!feof(f))
    {
        c = fgetc(f);

        if (c == EOF) break;

        if (width == ind)
        {
            if (width)
                vec_app(line, &utfchr);
            width = chr_utf8_len(c);
            ind   = 0;
        }

	    if (c == '\r') continue; /* When we encounter \r, ignore it */
        if (c == '\n') break;    /* When we encounter \n, new line  */

        utfchr.utf8[ind++] = c;
    }
}

void file_load_win(win *w, FILE *f)
{
    cur loc = { .cn = 0, .ln = 0 };
    vec line;
    vec_init(&line, sizeof(chr));

    while (!feof(f))
    {
        file_load_line(&line, f);

        if (ferror(f)) break;

        buf_ins(w->b, loc, vec_get(&line, 0), vec_len(&line));

        if (feof(f)) break;

        loc.ln += 1;
        buf_ins_line(w->b, loc);
    }

    indent_add_blanks_buf(w->b);

    vec_kill(&line);
}

/* chrs is a vec of chrs, fullpath is of chars */
static int file_get_fullpath(vec *chrs, vec *fullpath)
{
    /* Three copies of the stringified argument.     *
     * The first one to keep, and the second to feed *
     * to basename and dirname.                      */
    vec arg, basevec, dirvec;
    char *base, *dir, *path;

    vec_init(&arg, sizeof(char));
    chr_to_str(chrs, &arg);
    vec_app(&arg, NULL);

    /* Make throwaway copies of our argument path */
    vec_init(&basevec, sizeof(char));
    vec_init(&dirvec,  sizeof(char));
    vec_cpy(&basevec, &arg);
    vec_cpy(&dirvec,  &arg);

    /* We get our base and dir. These should NOT be free'd, as *
     * they may be inside dirvec and basevec. If not, they are *
     * statically allocated memory.                            */
    dir  = dirname( vec_get(&dirvec,  0));
    base = basename(vec_get(&basevec, 0));

    /* We must free this */
    path = realpath(dir, NULL);

    if (path)
    {
        vec_del(fullpath, 0, vec_len(fullpath));
        vec_ins(fullpath, 0, strlen(path), path);

        /* Add a trailing slash to fullpath if needed */
        if (path[strlen(path) - 1] != '/')
            vec_app(fullpath, "/");

        /* Add the basename to the directory name. Don't do this *
         * if it is ., or /, as these are pretend names.         */
        if (strcmp(base, "/") != 0 && strcmp(base, ".") != 0)
            vec_ins(fullpath, vec_len(fullpath), strlen(base), base);

        /* Null terminator */
        vec_app(fullpath, "\0");
    }

    free(path);

    vec_kill(&arg);
    vec_kill(&basevec);
    vec_kill(&dirvec);

    if (path)
        return 0;
    else
        return -1;
}

