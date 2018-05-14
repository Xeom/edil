#include <string.h>

#include "indent.h"
#include "cur.h"
#include "chr.h"
#include "cmd.h"
#include "bind.h"
#include "out.h"

#include "ui.h"

int ui_alive = 1;
char *ui_cmd_prompt = "$ ";


void ui_cmd_cb(win *w, vec *chrs)
{
    vec args, rtn;
    size_t    argind;

    vec_init(&args, sizeof(vec));
    vec_init(&rtn,  sizeof(chr));

    cmd_parse(&args, chrs, 0);
    cmd_log(chrs, 1);

    cmd_run(&args, &rtn, w);

    out_log(&rtn, stdout);
    cmd_log(&rtn, 0);

    for (argind = 0; argind < vec_len(&args); ++argind)
        vec_kill(vec_get(&args, argind));

    win_out_all();

    vec_kill(&args);
    vec_kill(&rtn);
}
