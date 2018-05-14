#include "cur.h"
#include "bind.h"
#include "ui.h"

#include "bind/kcd.h"

table bind_kcd;

BIND_FUNCT(kcd_to_buf, bind_mode = bind_mode_buf)

void bind_kcd_init(void)
{
    table *tab = &bind_kcd;
    table_init(tab, sizeof(bind_fptr), sizeof(inp_key));

    BIND_TO(kcd_to_buf, inp_key_ctrl | 'A');
}

void bind_kcd_kill(void)
{
    table_kill(&bind_kcd);
}

void bind_kcd_ins(win *w, vec *text)
{
    size_t ind, len;

    len = vec_len(text);
    for (ind = 0; ind < len; ++ind)
    {
        char buf[32];
        inp_key *k;
        vec chrbuf;

        k = vec_get(text, ind);
        inp_key_name(*k, buf, sizeof(buf));

        vec_init(&chrbuf, sizeof(chr));
        chr_from_str(&chrbuf, buf);
        cur_ins(w, &chrbuf);
        cur_enter(w);
        vec_kill(&chrbuf);
    }
}