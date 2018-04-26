#if !defined(_APPEARANCE_H_)
#define _APPEARANCE_H_

/*
 * Copyright 2000 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

/*
  NOTE: this is not an attempt to recreate Apple's appearance manager.
  this is just a hack to make it so that end-users can ask for a different
  set of WDEFs, CDEFs, etc. at run time
*/
namespace Executor
{
typedef enum {
    appearance_sys7,
    appearance_win3,
} appearance_t;

extern void ROMlib_set_appearance(void);
extern bool ROMlib_parse_appearance(const char *appearance_str);
extern appearance_t ROMlib_get_appearance(void);
}
#endif
