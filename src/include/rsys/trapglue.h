#if !defined(__TRAPGLUE__)
#define __TRAPGLUE__
#include <syn68k_public.h>

namespace Executor
{

typedef struct
{
    void *wheretogo;
    uint64_t magic;
} ptocblock_t;

typedef struct
{
    ptocblock_t ptoc;
    syn68k_addr_t orig;
} toolstuff_t;

typedef struct
{
    syn68k_addr_t orig;
    void *func;
} osstuff_t;

#define TOOLBIT (0x0800)
#define NTOOLENTRIES (0x400)
#define NOSENTRIES (0x100)

extern syn68k_addr_t tooltraptable[NTOOLENTRIES];
extern syn68k_addr_t ostraptable[NOSENTRIES];

extern syn68k_addr_t PascalToCCall(syn68k_addr_t ignoreme, ptocblock_t *infop);

extern unsigned short mostrecenttrap;

extern uintptr_t CToPascalCall(void *, uint64_t, ...);
extern toolstuff_t toolstuff[NTOOLENTRIES];
extern osstuff_t osstuff[];
}

#endif
