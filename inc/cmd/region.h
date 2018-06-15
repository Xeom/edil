#if !defined(CMD_REGION)
# define CMD_REGION
# include "buf/buf.h"
# include "vec.h"
# include "win.h"

extern buf *region_clipboard;

void cmd_region_init(void);

#endif
