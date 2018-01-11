#if !defined(_COMMON_H_)
#define _COMMON_H_

#if defined(_WIN32) && !defined(WIN32)
#define WIN32 /* evil hackage needed to make SDL happy */
#endif

#if !defined(COMPILE_FOR_BUILD)
#include "host-os-config.h"
#include "host-arch-config.h"
#endif /* !COMPILE_FOR_BUILD */

#if !defined(CYGWIN32) && !defined(WIN32) && defined(USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES)
#undef USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES
#endif

#include "rsys/macros.h"
#include "rsys/types.h"
#include "rsys/mactype.h"
#include "rsys/byteswap.h"

#include <syn68k_public.h>

#include "rsys/pascal.h"
#include "ExMacTypes.h"
#include "rsys/slash.h"
#include "rsys/error.h"
#if !defined USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES
#include "rsys/lowglobals.h"
#endif

#include "rsys/trapglue.h"

#if !defined(COMPILE_FOR_BUILD)
#include "front-end-config.h"
#endif

namespace Executor
{

typedef struct
{
} host_spf_reply_block;

typedef enum { get,
               put } getorput_t;
typedef enum { original_sf,
               new_sf,
               new_custom_sf } sf_flavor_t;

extern bool host_has_spfcommon(void);
extern bool host_spfcommon(host_spf_reply_block *replyp,
                           const char *prompt, const char *filename,
                           void *fp, void *filef, int numt,
                           void *tl, getorput_t getorput,
                           sf_flavor_t flavor,
                           void *activeList, void *activateproc,
                           void *yourdatap);
}

#endif /* !_COMMON_H_ */
