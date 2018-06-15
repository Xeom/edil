#if !defined _XOPEN_SOURCE /* For realpath() */
# define _XOPEN_SOURCE 500
# include <stdlib.h>
# include <limits.h>
# undef _XOPEN_SOURCE
#else
# include <stdlib.h>
# include <limits.h>
#endif

#include <errno.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>

#include "buf/line.h"
#include "buf/buf.h"
#include "container/vec.h"
#include "chr.h"

#include "file.h"

void file_init(file *f)
{
    vec_init(&(f->fname),    sizeof(char));
    vec_init(&(f->basename), sizeof(char));
    vec_init(&(f->dirname) , sizeof(char));

    f->flags = 0;
    f->fptr = NULL;
}

void file_init_pipe(file *f, FILE *pipe)
{
    file_init(f);

    f->flags |= file_pipe;
    f->fptr = pipe;
}

void file_kill(file *f)
{
    file_close(f);

    vec_kill(&(f->fname));
    vec_kill(&(f->basename));
    vec_kill(&(f->dirname));
}

char *file_name(file *f)
{
    static char *empty = "";

    if (!file_associated(f) || f->flags & file_pipe)
        return empty;

    return vec_first(&(f->fname));
}

char *file_base(file *f)
{
    static char *empty = "";

    if (!file_associated(f) || f->flags & file_pipe)
        return empty;

    return vec_first(&(f->basename));
}

int file_assoc(file *f, vec *chrname)
{
    if (f->flags & file_pipe)
        return -1;

    if (file_associated(f))
        file_deassoc(f);

    return file_set_paths(f, chrname);
}

void file_deassoc(file *f)
{
    vec_clr(&(f->fname));
    vec_clr(&(f->basename));
    vec_clr(&(f->dirname));
}

int file_associated(file *f)
{
    if (f->flags & file_pipe)
        return 0;

    return vec_len(&(f->fname)) > 0;
}

int file_exists(file *f)
{
    return access(vec_first(&(f->fname)), F_OK) != -1;
}

int file_open(file *f, const char *mode)
{
    char *path;
    path = vec_first(&(f->fname));

    if (f->flags & file_pipe)
        return 0;

    if (!file_associated(f))
        return -1;

    file_close(f);

    f->fptr = fopen(path, mode);

    if (!(f->fptr))
        return -1;
    else
        return 0;
}

int file_close(file *f)
{
    int rtn;

    if (!(f->fptr))
        return 0;

    rtn = fclose(f->fptr);
    f->fptr = NULL;

    return rtn;
}

int file_load(file *f, buf *b)
{
    if (f->flags & file_pipe)
    {
        if (!f->fptr)
        {
            errno = EBADF;
            return -1;
        }
    }
    else
    {
        if (file_open(f, "r") == -1)
            return -1;
    }

    buf_clr(b);

    while (!feof(f->fptr))
    {
        file_load_line(f, b);

        if (ferror(f->fptr))
            return -1;
    }

    buf_del_lines(b, (cur){ .ln = buf_len(b) - 1 }, 1);

    if (!(f->flags & file_pipe))
        file_close(f);

    return 0;
}

int file_read_line(file *f, vec *chrs)
{
    size_t width, ind;
    chr    utfchr = CHR("");
    int c;

    ind   = 0;
    width = 0;

    vec_clr(chrs);

    while (!feof(f->fptr))
    {
        c = fgetc(f->fptr);

        if (width == ind)
        {
            if (width)
                vec_app(chrs, &utfchr);

            width = chr_utf8_len(c);
            ind   = 0;
        }

        if (c == EOF) break;

        if (c == '\r')
        {
            f->flags |= file_cr;
            continue;
        }
        if (c == '\n') break;

        utfchr.utf8[ind++] = c;
    }

    if (ferror(f->fptr))
        return -1;
    else
        return  0;
}

int file_ended(file *f)
{
    if (!f->fptr)
        return 1;
    else
        return feof(f->fptr);
}

int file_load_line(file *f, buf *b)
{
    vec chrs;
    vec_init(&chrs, sizeof(chr));

    cur loc;
    loc = (cur){ .ln = buf_len(b) };

    file_read_line(f, &chrs);

    loc.ln -= 1;
    buf_ins_lines(b, loc, 1);
    buf_ins(b, loc, &chrs);

    vec_kill(&chrs);

    if (ferror(f->fptr))
        return -1;
    else
        return 0;
}

int file_save(file *f, buf *b)
{
    ssize_t numlines;
    char *newline;
    int extranl;
    cur loc = {0, 0};

    numlines = buf_len(b);

    if (f->flags & file_pipe)
    {
        if (!f->fptr)
        {
            errno = EBADF;
            return -1;
        }
    }

    if (f->flags & file_cr)
        newline = "\r\n";
    else
        newline = "\n";

    if (file_open(f, "w") == -1)
        return -1;

    extranl = (f->flags & file_eofnl) && (buf_last(b).cn != 0);

    while (loc.ln < numlines)
    {
        if (file_save_line(f, b, loc) == -1)
            return -1;

        if (ferror(f->fptr)) return -1;

        if (loc.ln < numlines - 1 || extranl)
        {
            if (fputs(newline, f->fptr) == EOF)
                return -1;
        }

        loc.ln += 1;
    }

    file_close(f);

    return 0;
}

int file_save_line(file *f, buf *b, cur loc)
{
    size_t len, ind;
    line *l;

    l   = buf_get_line(b, loc);
    len = line_len(l);

    for (ind = 0; ind < len; ind++)
    {
        size_t width;
        chr *c;

        c = line_chr(l, (cur){ .cn = ind });
        if (!c || chr_is_blank(c)) continue;

        width = chr_len(c);

        if (fwrite(c->utf8, 1, width, f->fptr) != width)
            return -1;
    }

    line_unlock(l);

    return 0;
}

/* chrs is a vec of chrs, fullpath is of chars */
int file_set_paths(file *f, vec *chrname)
{
    /* Three copies of the stringified argument.     *
     * The first one to keep, and the second to feed *
     * to basename and dirname.                      */
    vec dirvec, basevec;
    char *base, *dir, *path;

    /* Make throwaway copies of our argument path */
    vec_init(&basevec,  sizeof(char));
    vec_init(&dirvec,  sizeof(char));

    chr_to_str(chrname, &basevec);
    vec_app(&basevec, "\0");

    vec_cpy(&dirvec,  &basevec);

    /* We get our base and dir. These should NOT be free'd, as *
     * they may be inside dirvec and basevec. If not, they are *
     * statically allocated memory.                            */
    dir  =  dirname(vec_first(&dirvec));
    base = basename(vec_first(&basevec));

    vec_clr(&(f->basename));
    vec_clr(&(f->dirname));
    vec_clr(&(f->fname));

    if (dir && base)
    {
        /* We must free this */
        vec_str(&(f->basename), base);
        vec_str(&(f->dirname),  dir);

        vec_app(&(f->basename), "\0");
        vec_app(&(f->dirname),  "\0");

        path = realpath(dir, NULL);
    }
    else
        path = NULL;

    if (path)
    {
        vec_str(&(f->fname), path);

        /* Add a trailing slash to fullpath if needed */
        if (path[strlen(path) - 1] != '/')
            vec_app(&(f->fname), "/");

        /* Add the basename to the directory name. Don't do this *
         * if it is ., or /, as these are pretend names.         */
        if (strcmp(base, "/") != 0 && strcmp(base, ".") != 0)
            vec_str(&(f->fname), base);

        /* Null terminator */
        vec_app(&(f->fname), "\0");

        free(path);
    }

    vec_kill(&basevec);
    vec_kill(&dirvec);

    if (path)
        return 0;
    else
    {
        file_deassoc(f);
        return -1;
    }
}
