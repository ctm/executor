#if !defined(__TRAPGLUE__)
#define __TRAPGLUE__
#include <syn68k_public.h>

namespace Executor
{

#define TOOLBIT (0x0800)
#define NTOOLENTRIES (0x400)
#define NOSENTRIES (0x100)

extern syn68k_addr_t tooltraptable[NTOOLENTRIES];
extern syn68k_addr_t ostraptable[NOSENTRIES];

extern unsigned short mostrecenttrap;
}

#endif
