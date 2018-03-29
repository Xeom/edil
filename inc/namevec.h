#if !defined(NAMEVEC_H)
# define NAMEVEC_H
# include <stdlib.h>
# include "vec.h"

typedef struct namevec_item_s namevec_item;

struct namevec_item_s
{
    char *name;
    union
    {
        void *ptr;
        void (*fptr)(void);
    } data;
};

void namevec_init(vec *v, namevec_item *items, size_t bytes);

int namevec_startswith(namevec_item *item, char *str);

namevec_item *namevec_get_str(vec *v, char *str, size_t *n);
namevec_item *namevec_get_chrs(vec *v, vec *chrs, size_t *n);

#endif
