#include "cur.h"
#include "bind.h"
#include "ui.h"

#include "bind/kcd.h"

table bind_kcd;
table bind_kcd_names;

BIND_FUNCT(kcd_to_buf, bind_mode = bind_mode_buf)

void bind_kcd_init(table *tab)
{
    BIND_TO(kcd_to_buf, inp_key_ctrl | 'A', Switch to buffer mode);
}

void bind_kcd_key(win *w, inp_key k)
{
    char buf[32];
    vec chrbuf;

    inp_key_name(k, buf, sizeof(buf));

    vec_init(&chrbuf, sizeof(chr));
    chr_from_str(&chrbuf, buf);
    cur_ins_win(w, &chrbuf);
    cur_enter_win(w);
    vec_kill(&chrbuf);
}