#if !defined(NAMEVEC_H)
# define NAMEVEC_H
# include <stdlib.h>
# include "vec.h"
# include "win.h"
# include "bind.h"

typedef struct namevec_item_s namevec_item;

struct namevec_item_s
{
    char *name;
    union
    {
        void (*cmdfunct)(vec *rtn, vec *args, win *w);
        bind_info *bind;
    } data;
};

void namevec_init(vec *v, namevec_item *items, size_t bytes);
void namevec_sort(vec *v);

int namevec_startswith(namevec_item *item, char *str);

namevec_item *namevec_get_str(vec *v, char *str, size_t *n);
namevec_item *namevec_get_chrs(vec *v, vec *chrs, size_t *n);

#endif
