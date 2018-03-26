#include <string.h>

#include "vec.h"
#include "chr.h"

#include "cmd/file.h"
#include "cmd/nav.h"

#include "cmd.h"

vec cmd_infos;

static cmd_info cmd_infos_static[] =
{
    { "load",      file_cmd_load    },
    { "save",      file_cmd_save    },
    { "discard",   file_cmd_discard },
    { "associate", file_cmd_assoc   },
    { "cd",        file_cmd_chdir   },
    { "goto",      nav_cmd_goto     },
    { "swap",      nav_cmd_swap     }
};

/* Check whether a cmd name starts with a string */
static int cmd_info_startswith(cmd_info *info, char *str);
/* Compare cmd infos, for orting cmd_infos */
static int cmd_info_cmp(const void *a, const void *b);

/* Increment ind until whitespace isn't found, return this *
 * value.                                                  */
static size_t cmd_eat_whitespace(vec *chrs, size_t ind);

static size_t cmd_parse_str(vec *str, vec *chrs, size_t ind);
static size_t cmd_parse_word(vec *str, vec *chrs, size_t ind);

static int cmd_info_startswith(cmd_info *info, char *str)
{
    size_t len;

    if (!info) return 0;

    len = strlen(str);

    if (strncmp(str, info->name, len) == 0)
        return 1;

    else
        return 0;
}

static int cmd_info_cmp(const void *a, const void *b)
{
    const cmd_info *ainfo, *binfo;

    ainfo = a;
    binfo = b;

    return strcmp(ainfo->name, binfo->name);
}

void cmd_init(void)
{
    size_t numcmds;

    numcmds = sizeof(cmd_infos_static) / sizeof(cmd_info);

    vec_init(&cmd_infos, sizeof(cmd_info));
    vec_ins(&cmd_infos, 0, numcmds, &cmd_infos_static);
    vec_sort(&cmd_infos, cmd_info_cmp);
}

void cmd_kill(void)
{
    vec_kill(&cmd_infos);
}

void cmd_run(vec *args, vec *rtn, win *w)
{
    size_t len, ind;
    vec name;
    char *namestr;
    cmd_info *info;

    len = vec_len(args);
    if (len == 0) return;

    vec_init(&name, sizeof(char));
    chr_to_str(vec_get(args, 0), &name);
    vec_ins(&name, vec_len(&name), 1, NULL);

    namestr = vec_get(&name, 0);

    ind = vec_bst(&cmd_infos, &(cmd_info){ .name = namestr }, cmd_info_cmp);
    info = vec_get(&cmd_infos, ind);

    if (cmd_info_startswith(info, namestr))
    {
        cmd_info *next;
        next = vec_get(&cmd_infos, ++ind);
        if (cmd_info_startswith(next, namestr))
        {
            chr_format(rtn, "err: %s is ambiguous (%s", namestr, info->name);

            while (next && cmd_info_startswith(next, namestr))
            {
                chr_format(rtn, ", %s", next->name);
                next = vec_get(&cmd_infos, ++ind);
            }

            chr_format(rtn, ")");
            vec_kill(&name);

            return; /* Not a unique name */
        }
        else
            info->funct(rtn, args, w);
    }
    else
    {
        chr_format(rtn, "err: %s is not a known command", namestr);
    }

    vec_kill(&name);
}

void cmd_parse(vec *args, vec *chrs, size_t ind)
{
    size_t len;

    len = vec_len(chrs);

    for (; ind < len; ind++)
    {
        chr *c;
        vec *arg;
        ind = cmd_eat_whitespace(chrs, ind);

        c = vec_get(chrs, ind);
        if (!c) break;

        arg = vec_ins(args, vec_len(args), 1, NULL);
        vec_init(arg, sizeof(chr));

        if (strcmp("\"", c->utf8) == 0)
            ind = cmd_parse_str(arg, chrs, ind + 1);

        else
            ind = cmd_parse_word(arg, chrs, ind);
    }
}

static size_t cmd_eat_whitespace(vec *chrs, size_t ind)
{
    size_t len;

    len = vec_len(chrs);

    for (; ind < len; ind++)
    {
        chr *c;
        c = vec_get(chrs, ind);

        if (strcmp(" ", c->utf8) != 0)
            break;
    }

    return ind;
}

static size_t cmd_parse_str(vec *str, vec *chrs, size_t ind)
{
    int    escaped = 0;
    size_t len;

    len = vec_len(chrs);

    for (; ind < len; ind++)
    {
        chr *c;
        c = vec_get(chrs, ind);

        if (escaped)
            vec_ins(str, vec_len(str), 1, c);
        else if (strcmp("\\", c->utf8) == 0)
            escaped = 1;
        else if (strcmp("\"", c->utf8) == 0)
        {
            ind += 1;
            break;
        }
        else
            vec_ins(str, vec_len(str), 1, c);
    }

    return ind;
}

static size_t cmd_parse_word(vec *str, vec *chrs, size_t ind)
{
    int    escaped = 0;
    size_t len;

    len = vec_len(chrs);
  
    for (; ind < len; ind++)
    {
        chr *c;
        c = vec_get(chrs, ind);

        if (escaped)
            vec_ins(str, vec_len(str), 1, c);
        else if (strcmp("\\", c->utf8) == 0)
            escaped = 1;
        else if (strcmp(" ",  c->utf8) == 0)
            break;
        else if (strcmp(")",  c->utf8) == 0)
            break;
        else if (strcmp("\"", c->utf8) == 0)
            break;
        else
            vec_ins(str, vec_len(str), 1, c);
    }

    return ind;
}

