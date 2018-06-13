#include "bind.h"

#include "bind/kcdmap.h"

void bind_kcdmap_init(void)
{
    BIND_MAP(kcd, mode_buf, inp_key_ctrl | '^');
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
