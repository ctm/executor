#if !defined(_DIRTYRECT_H_)
#define _DIRTYRECT_H_

#include "host_bltmacros.h"
namespace Executor
{
extern void dirty_rect_accrue(int top, int left, int bottom, int right);
extern void dirty_rect_update_screen(void);
extern bool dirty_rect_subsumed_p(int top, int left, int bottom,
                                  int right);

extern int num_dirty_rects;
}

#endif /* !_DIRTYRECT_H_ */
