#include "win.h"
#include "out.h"
#include "inp.h"
#include "con.h"

int main(void)
{

    buf b;
    win w;
    vec text, str;

    vec_init(&str,  sizeof(char));
    vec_init(&text, sizeof(chr));

    vec_ins(&str, 0, 12, "Hello World!");
    chr_from_str(&text, &str);

    inp_init();
    out_init(stdout);
    buf_init(&b);
    win_init(&w, &b);
    con_init();

    win_cur = &w;

    w.pri = cur_ins(w.pri, &b, &text);
    w.pri = cur_enter(w.pri, &b);

    w.cols = out_cols;
    w.rows = out_rows;

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

    buf_kill(&b);

    vec_kill(&str);
    vec_kill(&text);
}
