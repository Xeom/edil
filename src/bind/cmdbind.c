#include <string.h>

#include "ui.h"
#include "bind.h"

#include "bind/modebind.h"

#include "bind/cmdbind.h"

static void bind_cmd_shortcut_run(char *str, win *w);
static void bind_cmd_shortcut(char *str, win *w);

#define SCUT(_cmd) \
    BIND_FUNCT(cmd_ ## _cmd, bind_cmd_shortcut(#_cmd " ", w))

#define RCUT(_cmd) \
    BIND_FUNCT(cmd_r_ ## _cmd, bind_cmd_shortcut_run(#_cmd, w))

#define SCUT_ADD(_cmd) \
    BIND_ADD(cmd_ ## _cmd, Shortcut for _cmd command)

#define RCUT_ADD(_cmd) \
    BIND_ADD(cmd_r_ ## _cmd, Run _cmd command)

SCUT(new)
SCUT(load)
SCUT(save)
SCUT(goto)
SCUT(quit)
SCUT(saveall)

RCUT(swap)
RCUT(snap)
RCUT(save)
RCUT(saveall)
RCUT(next)
RCUT(prev)
RCUT(copy)
RCUT(paste)
RCUT(lineify)
RCUT(incrindent)
RCUT(decrindent)

void bind_cmdbind_init(void)
{
    SCUT_ADD(new);
    SCUT_ADD(load);
    SCUT_ADD(save);
    SCUT_ADD(goto);
    SCUT_ADD(quit);
    SCUT_ADD(saveall);

    RCUT_ADD(swap);
    RCUT_ADD(snap);
    RCUT_ADD(save);
    RCUT_ADD(next);
    RCUT_ADD(prev);
    RCUT_ADD(copy);
    RCUT_ADD(paste);
    RCUT_ADD(lineify);
    RCUT_ADD(saveall);
    RCUT_ADD(incrindent);
    RCUT_ADD(decrindent);
}

/* Command shortcuts */
static void bind_cmd_shortcut(char *str, win *w)
{
    vec chrs;
    vec_init(&chrs, sizeof(chr));
    chr_from_str(&chrs, str);

    bind_cmd_activate(w);
    bar_ins(&(w->basebar), &chrs);

    vec_kill(&chrs);
}

static void bind_cmd_shortcut_run(char *str, win *w)
{
    vec chrs;
    vec_init(&chrs, sizeof(chr));
    chr_from_str(&chrs, str);

    ui_cmd_cb(w, &chrs);

    vec_kill(&chrs);
}
