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

#include "cmd/file.h"

static int file_get_fullpath(vec *chrs, vec *fullpath);
static void file_clr_win(win *w);

static void file_save_line(vec *line, FILE *f);
static void file_load_line(vec *line, FILE *f);

/* chrs is a vec of chrs, fullpath is of chars */
static int file_get_fullpath(vec *chrs, vec *fullpath)
{
    vec str;
    char *base, *dir, *path;

    vec_init(&str, sizeof(char));
    chr_to_str(chrs, &str);
    vec_ins(&str, vec_len(&str), 1, NULL);

    dir = dirname(vec_get(&str, 0));
    path = realpath(dir, NULL);    

    if (path == NULL)
    {
        vec_kill(&str);
        return -1;
    }

    vec_del(fullpath, 0, vec_len(fullpath));
    vec_ins(fullpath, 0, strlen(path), path);

    if (strcmp(path, "/") != 0)
        vec_ins(fullpath, vec_len(fullpath), 1, "/");

    base = basename(vec_get(&str, 0));

    if (strcmp(base, "/") != 0 && strcmp(base, ".") != 0)
        vec_ins(fullpath, vec_len(fullpath), strlen(base), base);

    vec_ins(fullpath, vec_len(fullpath), 1, NULL);

    vec_kill(&str);

    return 0;
}

void file_cmd_discard(vec *rtn, vec *args, win *w)
{
    file_clr_win(w);
    w->b->flags &= ~buf_modified;
    chr_format(rtn, "Contents of buffer discarded");
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

    if (vec_len(args) == 2)
    {
        vec *path;
        path = vec_get(args, 1);
        if (file_get_fullpath(path, fn) == -1)
        {
            chr_format(rtn, "err Parsing path: [%d] %s", errno, strerror(errno));
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
                chr_format(rtn, "err Opening '%s': [%d] %s", vec_get(fn, 0), errno, strerror(errno));
        }
        else
        {
            file_clr_win(w);
            file_load_win(w, f);

            w->b->flags &= ~buf_modified;

            if (ferror(f))
            {
                fclose(f);
                chr_format(rtn, "err Reading '%s': [%d] %s", vec_get(fn, 0), errno, strerror(errno));

                return;
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

    if (vec_len(args) == 2)
    {
        vec *path;
        path = vec_get(args, 1);
        if (file_get_fullpath(path, fn))
        {
            chr_format(rtn, "err Parsing path: [%d] %s, ", errno, strerror(errno));
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
        chr_format(rtn, "err Opening '%s': [%d] %s", vec_get(fn, 0), errno, strerror(errno));
        return;
    }

    file_save_win(w, f);

    if (ferror(f))
        chr_format(rtn, "err Writing '%s': [%d] %s", vec_get(fn, 0), errno, strerror(errno));
    else
        chr_format(rtn, "Wrote '%s'", vec_get(fn, 0));

    fclose(f);

    w->b->flags &= ~buf_modified;
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

void file_cmd_chdir(vec *rtn, vec *args, win *w)
{
    char cwd[64];

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

    getcwd(cwd, sizeof(cwd));
    chr_format(rtn, "cwd: %s", cwd);
}

static void file_clr_win(win *w)
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
    chr    utfchr = { .fnt = { .fg = col_none, .bg = col_none } };
    int   c;

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
                vec_ins(line, vec_len(line), 1, &utfchr);
            width = chr_utf8_len(c);
            ind   = 0;
        }

        if (c == '\n') break;

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

    vec_kill(&line);
}
