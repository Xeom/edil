#include "win.h"
#include "out.h"
#include "inp.h"
#include "con.h"
#include "cmd.h"

int main(void)
{
    buf b;
    win w;
    vec text;

    vec_init(&text, sizeof(chr));

    chr_from_str(&text, "Hello world!");

    inp_init();
    out_init(stdout);
    buf_init(&b);
    win_init(&w, &b);
    con_init();
    cmd_init();

    win_cur = &w;

    w.pri = cur_ins(w.pri, &b, &text);
    w.pri = cur_enter(w.pri, &b);

    w.cols = out_cols;
    w.rows = out_rows - 1;

    win_out_after(&w, (cur){0, 0}, stdout);
    fflush(stdout);

    while (con_alive)
    {
        inp_wait();
        fflush(stdout);
    }

    out_kill(stdout);
    inp_kill();
    con_kill();
    cmd_kill();
    win_kill(&w);

    buf_kill(&b);

    vec_kill(&text);
}
