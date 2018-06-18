#include <stdlib.h>
#include <stdarg.h>

#include "container/vec.h"

#include "print.h"

static void print_destruct_vec(void *v);

static pthread_key_t print_bufs;

void print_init(void)
{
    pthread_key_create(&print_bufs, print_destruct_vec);
}

static void print_destruct_vec(void *v)
{
    vec_kill(v);
    free(v);
}

vec *print_get_vec(void)
{
    vec *rtn;

    rtn = pthread_getspecific(print_bufs);

    if (!rtn)
    {
        rtn = malloc(sizeof(vec));
        vec_init(rtn, sizeof(char));

        pthread_setspecific(print_bufs, rtn);
    }

    return rtn;
}

void print_flush(void)
{
    vec *v;
    v = print_get_vec();

    if (vec_len(v))
    {
        write(STDOUT_FILENO, vec_first(v), vec_len(v));
        fflush(stdout);
    }

    vec_clr(v);
}

void print_str(char *str)
{
    vec *v;
    v = print_get_vec();

    vec_str(v, str);
}

void print_mem(char *mem, size_t n)
{
    vec *v;
    v = print_get_vec();

    vec_ins(v, vec_len(v), n, mem);
}

void print_fmt(char *fmt, ...)
{
    va_list args;
    vec *v;
    char *buf;
    size_t len, written;

    v   = print_get_vec();

    len = 32;
    buf = NULL;

    do
    {
        va_start(args, fmt);

        len *= 2;
        buf = realloc(buf, len);

        written = vsnprintf(buf, len, fmt, args);
    } while (written >= len);

    va_end(args);

    vec_str(v, buf);

    free(buf);
}
