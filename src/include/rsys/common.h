#if !defined (_COMMON_H_)
#define _COMMON_H_

#if !defined(COMPILE_FOR_BUILD)
#  include "config.h"
#endif

#if defined (_WIN32) && !defined(WIN32)
#define WIN32 /* evil hackage needed to make SDL happy */
#endif

/* #include this first, so we know what wrapper we are. */
#include "rsys/wrappers.h"

/* #include this second, so we know what release type we are. */
#include "rsys/release.h"

#if defined (COMPILE_FOR_BUILD)
#include "build-os-config.h"
#include "build-arch-config.h"
#else /* !COMPILE_FOR_BUILD */
#include "host-os-config.h"
#include "host-arch-config.h"
#endif /* !COMPILE_FOR_BUILD */

#if !defined (CYGWIN32) && defined (USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES)
#undef USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES
#endif

#if !defined (LINUX) && !defined (MSDOS) && !defined (NEXT) && !defined(CYGWIN32) && !defined (MACOSX_)
# error "Unsupported host"
#endif

#include "rsys/cruft.h"
#include "rsys/macros.h"
#include "rsys/types.h"
#include "rsys/mactype.h"
#include "rsys/byteswap.h"

#if !defined USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES
#include <syn68k_public.h>
#include "MacTypes.h"
#include "rsys/trapdecl.h"
#include "rsys/stubify.h"
#include "rsys/slash.h"
#include "rsys/error.h"
#include "rsys/lowglobals.h"
#endif

#if !defined (COMPILE_FOR_BUILD)
#include "front-end-config.h"
#endif

#ifdef __cplusplus
namespace ByteSwap {
template < size_t size >
inline void sized_byteswap(char* data);

template <>
inline void sized_byteswap< 2 >(char* data)
{
    uint16_t* ptr = reinterpret_cast<uint16_t*>(data);
    *ptr = __builtin_bswap16(*ptr);
}

template <>
inline void sized_byteswap< 4 >(char* data)
{
    uint32_t* ptr = reinterpret_cast<uint32_t*>(data);
    *ptr = __builtin_bswap32(*ptr);
}

	template <>
	inline void sized_byteswap< 8 >(char* data)
	{
		uint64_t* ptr = reinterpret_cast<uint64_t*>(data);
		*ptr = __builtin_bswap64(*ptr);
	}

	
template < typename T >
T BigEndianValue(T value)
{
    sized_byteswap< sizeof(T) >(reinterpret_cast<char*>(&value));
    return value;
}

	template < typename T >
	void BigEndianInPlace(T &value)
	{
		sized_byteswap< sizeof(T) >(reinterpret_cast<char*>(&value));
	}
}
namespace Executor {
#endif
typedef struct
{
}
host_spf_reply_block;

typedef enum { get, put } getorput_t;
typedef enum { original_sf, new_sf, new_custom_sf } sf_flavor_t;

extern boolean_t host_has_spfcommon (void);
extern boolean_t host_spfcommon (host_spf_reply_block *replyp,
				 const char *prompt, const char *filename,
				 void *fp, void *filef, int numt,
				 void *tl, getorput_t getorput,
				 sf_flavor_t flavor,
				 void *activeList, void *activateproc,
				 void *yourdatap);
#ifdef __cplusplus
}
#endif
#endif /* !_COMMON_H_ */
