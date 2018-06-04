#include "bind.h"
#include "ui.h"

#include "bind/modebind.h"

static void bind_bar_deactivate(win *w);

BIND_FUNCT(mode_cmd,
    bind_cmd_activate(w);
)

BIND_FUNCT(mode_buf,
    bind_bar_deactivate(w);
    bind_mode = bind_mode_buf;
)

BIND_FUNCT(mode_kcd,
    bind_bar_deactivate(w);
    bind_mode = bind_mode_kcd;
)

BIND_FUNCT(mode_mov,
    bind_bar_deactivate(w);
    bind_mode = bind_mode_mov;
)

void bind_modebind_init(void)
{
    BIND_ADD(mode_cmd, Open the command bar);
    BIND_ADD(mode_buf, Switch to buffer mode);
    BIND_ADD(mode_kcd, Switch to keycode mode);
    BIND_ADD(mode_mov, Switch to movement mode);
}

void bind_cmd_activate(win *w)
{
    vec cmdprompt;
    vec_init(&cmdprompt, sizeof(chr));
    chr_from_str(&cmdprompt, "$ ");//ui_cmd_prompt);

    bind_mode = bind_mode_bar;

    bar_query(&(w->basebar), &cmdprompt, ui_cmd_cb);
    vec_kill(&cmdprompt);
}

static void bind_bar_deactivate(win *w)
{
    if (bind_mode == bind_mode_bar)
        bar_cancel(&(w->basebar));
}