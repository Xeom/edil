#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "util.h"
#include "cmd.h"
#include "text.h"

#include "inp.h"

#define INP_COL_CUR1  text_col_rev|text_col_bold
#define INP_COL_CUR2  text_col_under|text_col_bold

#define INP_COL_BLANK text_col_black|text_col_bold
#define INP_STR_BLANK "\xc2\xbb"

#define USING_MTX(mtx, code)          \
        pthread_mutex_lock(&(mtx));   \
        do { code } while (0);        \
        pthread_mutex_unlock(&(mtx)); \

vec inp_keycodes;

inp_keycode inp_keycodes_static[] =
{
    {inp_key_up,     "[A"},
    {inp_key_down,   "[B"},
    {inp_key_right,  "[C"},
    {inp_key_left,   "[D"},
    {inp_key_home,   "[1~"},
    {inp_key_del,    "[3~"},
    {inp_key_end,    "[4~"},
    {inp_key_pgup,   "[5~"},
    {inp_key_pgdn,   "[6~"},
    {inp_key_insert, "[4h"},
    {inp_key_f1,     "OP"},
    {inp_key_f2,     "OQ"},
    {inp_key_f3,     "OR"},
    {inp_key_f4,     "OS"},
    {inp_key_f5,     "[15~"},
    {inp_key_f6,     "[17~"},
    {inp_key_f7,     "[18~"},
    {inp_key_f8,     "[19~"},
    {inp_key_f9,     "[20~"},
    {inp_key_f10,    "[21~"},
    {inp_key_f11,    "[23~"},
    {inp_key_f12,    "[24~"}
};

static int inp_keycode_cmp(const void *a, const void *b)
{
    return strcmp(((const inp_keycode *)a)->code, ((const cli_keycode *)b)->code);
}

static inp_key inp_get_escaped_key(char chr)
{
    static char code[16];
    static int  code_ind   = 0;
    static int  search_ind = 0;
    static int  rtn;

    code[code_ind] = chr;
    code[code_ind + 1] = '\0';
    code_ind += 1;

    while (code_ind <= 14)
    {
        int cmp;
        inp_keycode *curr;

        curr = vec_get(&inp_keycodes, search_ind);
        if (curr) cmp = strcmp(code, curr->code);

        if (cmp == 0) {rtn = curr->key;             break;}
        if (!curr)    {rtn = inp_key_esc | code[0]; break;}
        if (cmp >  0) search_ind++;
        if (cmp <  0)
        {
            if (memcmp(code, curr->code, code_ind) == 0)
                return 0;
            
            rtn = inp_key_esc | code[0];
            break;
        }
    }

    code_ind   = 0;
    search_ind = 0;

    return rtn;
}

inp_key inp_get_key(char chr)
{
    static int escaped = 0;
    inp_key    rtn;

    if (escaped)
    {    
        rtn = inp_get_escaped_key(chr);
        if (rtn == 0) return inp_key_none;
    }
    else if (chr == '\033')
    {
        escaped = 1;
        return inp_key_none;
    }
    else
        rtn = chr;

    escaped = 0;

    if ((rtn & ~inp_key_esc) < 32)
    {
        rtn += 0x40;
        rtn |= inp_key_ctrl;
    }

    return rtn;
}

tqueue inp_queue;

pthread_t inp_listen_thread;

void *inp_listen(void *arg)
{
    char   chr;
    tqueue *tq;

    tq = &inp_queue;

    while (1)
    {
        char chr;
        inp_key key;

        chr = getchar();
        key = inp_get_key(chr);               
        tqueue_in(tq, &key);
    }
}

void inp_loop(void)
{   
    tqueue *tq;
    inp_key *key;

    tq = &inp_queue;    

    do
    {
        key = tqueue_out(tq);
        cmd_handle_key(*key);
    } while (!tqueue_empty(tq));

    cmd_ins_flush();
    fflush(stdout);        
}

void inp_init(void)
{
    size_t numkeys;

    /* Load the keycodes */
    numkeys = sizeof(inp_keycodes_static) / sizeof(cli_keycode);

    vec_init(&inp_keycodes, sizeof(cli_keycode));
    vec_ins(&inp_keycodes, 0, numkeys, &inp_keycodes_static);
    vec_sort(&inp_keycodes, inp_keycode_cmp);

    tqueue_init(&inp_queue, sizeof(inp_key));

    /* Create the input listener thread */
    pthread_create(&inp_listen_thread, NULL, inp_listen, NULL);
}    


void inp_kill(void)
{
    vec_kill(&inp_keycodes);
}

