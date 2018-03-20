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

#include "cmd.h"
#include "chr.h"

#include "cmd/file.h"

static void file_load_win(win *w, FILE *f);
static void file_load_line(vec *line, FILE *f);
static void file_clr_win(win *w);

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
        char fullpath[PATH_MAX];
        vec relpath;

        vec_init(&relpath, sizeof(char));
        chr_to_str(vec_get(args, 1), &relpath);
        vec_ins(&relpath, vec_len(&relpath), 1, NULL);

        if (realpath(vec_get(&relpath, 0), fullpath) == NULL)
        {
            chr_format(rtn, "err: Parsing '%s' [%d] %s", vec_get(&relpath, 0), errno, strerror(errno));
            return;
        }

        vec_ins(fn, 0, strlen(fullpath) + 1, fullpath);

        w->b->flags |= buf_associated;
    }

    if (w->b->flags & buf_associated)
    {
        FILE *f;
        f = fopen(vec_get(fn, 0), "r");
        if (f == NULL)
        {
            chr_format(rtn, "err Opening '%s': [%d] %s", vec_get(fn, 0), errno, strerror(errno));
        }
        else
        {
            file_clr_win(w);
            file_load_win(w, f);

            if (ferror(f))
                chr_format(rtn, "err Reading '%s': [%d] %s", vec_get(fn, 0), errno, strerror(errno));
        }
            
                /* Reload */
    }
    else
        chr_format(rtn, "err: No associated file");
}

void file_cmd_dump(vec *rtn, vec *args, win *w){}

void file_cmd_chdir(vec *rtn, vec *args, win *w)
{
    char cwd[64];

    if (vec_len(args) == 2)
    {
        vec dir, *arg;
        vec_init(&dir, sizeof(char));

        arg = vec_get(args, 1);

        chr_to_str(arg, &dir);
        vec_ins(&dir, vec_len(&dir), 1, NULL);

        if (chdir(vec_get(&dir, 0)) == -1)
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

static void file_load_win(win *w, FILE *f)
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
}
