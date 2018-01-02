/* Copyright 1990 - 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Implementation of MAC memory manager routines */

/* Forward declarations in MemoryMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "MemoryMgr.h"
#include "SegmentLdr.h"
#include "QuickDraw.h"
#include "SysErr.h"
#include "TextEdit.h"
#include "OSEvent.h"

#include "rsys/mman_private.h"
#include "rsys/file.h"
#include "rsys/hook.h"
#include "rsys/memsize.h"
#include "rsys/executor.h"
#include "rsys/options.h"
#include "rsys/toolutil.h"
#include "rsys/gestalt.h"

#if defined(LINUX) || defined(MACOSX)
#include <sys/mman.h>

#if !defined(MAP_ANONYMOUS)
#define MAP_ANONYMOUS MAP_ANON
#endif

#endif /* LINUX */

#ifdef MACOSX_
#include <mach/mach_error.h>
#endif

namespace Executor
{

int ROMlib_applzone_size = DEFAULT_APPLZONE_SIZE;
int ROMlib_syszone_size = DEFAULT_SYSZONE_SIZE;
int ROMlib_stack_size = DEFAULT_STACK_SIZE;

/* these two variables define, in ROMlib space, the beginning of mac-memory
   and the end of mac memory.  They're purpose is to try to prevent routines
   like DisposHandle () from crashing when passed a bogus pointer.
   Specifically, I know an application that picks up 4 bytes from low-memory
   global 0x100 and then calls DisposHandle () on it.  That location contains
   0xFFFF0048 both here and on a Mac.  On a Mac, this doesn't cause a crash.
   */

PUBLIC uintptr_t ROMlib_syszone;
PUBLIC uintptr_t ROMlib_memtop;

#if defined(MM_MANY_APPLZONES)
/* for debugging, we can have multiple applzones which are used
   roundrobin */
int mm_n_applzones = 1;
static int mm_current_applzone;
#endif

/* for routines that simply set MemErr */
#define SET_MEM_ERR(err)   \
    do                     \
    {                      \
        GEN_MEM_ERR(err);  \
        MemErr = CWV(err); \
    } while(false)

SignedByte
hlock_return_orig_state(Handle h)
{
    block_header_t *block;
    SignedByte state;

    MM_SLAM("entry");

    block = HANDLE_TO_BLOCK(h);
    if(block == NULL)
    {
        SET_MEM_ERR(nilHandleErr);
        return 0;
    }

    if(USE(block) == FREE)
    {
        SET_MEM_ERR(memWZErr);
        return 0;
    }

    state = HANDLE_STATE(h, block);
    SET_HANDLE_STATE(h, block, state | LOCKBIT);
    MM_SLAM("exit");
    SET_MEM_ERR(noErr);
    return state;
}

Size zone_size(THz zone)
{
    return (char *)ZONE_BK_LIM(zone) - (char *)zone;
}

SignedByte
HGetState(Handle h)
{
    block_header_t *block;

    MM_SLAM("entry");

    /* there used to be a spewy check here that returned noErr (zero
     state) if the incoming handle address was `spewy' (less than
     0x2000 or between the end of the applzone and the current stack
     frame.  it got axed */

    block = HANDLE_TO_BLOCK(h);
    if(block == NULL)
    {
        SET_MEM_ERR(nilHandleErr);
        return nilHandleErr;
    }

    if(USE(block) == FREE)
    {
        SET_MEM_ERR(memWZErr);
        return memWZErr;
    }

    SET_MEM_ERR(noErr);
    return HANDLE_STATE(h, block);
}

void HSetState(Handle h, SignedByte flags)
{
    block_header_t *block;

    MM_SLAM("entry");

    block = HANDLE_TO_BLOCK(h);
    if(block == NULL)
    {
        SET_MEM_ERR(nilHandleErr);
        return;
    }

    if(USE(block) == FREE)
    {
        SET_MEM_ERR(memWZErr);
        return;
    }

    SET_HANDLE_STATE(h, block, flags);
    MM_SLAM("exit");
    SET_MEM_ERR(noErr);
}

void HLock(Handle h)
{
    block_header_t *block;

    MM_SLAM("entry");

    block = HANDLE_TO_BLOCK(h);
    if(block == NULL)
    {
        SET_MEM_ERR(nilHandleErr);
        return;
    }

    if(USE(block) == FREE)
    {
        SET_MEM_ERR(memWZErr);
        return;
    }

    SET_HANDLE_STATE(h, block, HANDLE_STATE(h, block) | LOCKBIT);
    MM_SLAM("exit");
    SET_MEM_ERR(noErr);
}

void HUnlock(Handle h)
{
    block_header_t *block;

    MM_SLAM("entry");

    block = HANDLE_TO_BLOCK(h);
    if(block == NULL)
    {
        SET_MEM_ERR(nilHandleErr);
        return;
    }

    if(USE(block) == FREE)
    {
        SET_MEM_ERR(memWZErr);
        return;
    }

    SET_HANDLE_STATE(h, block, HANDLE_STATE(h, block) & ~LOCKBIT);
    MM_SLAM("exit");
    SET_MEM_ERR(noErr);
}

void HPurge(Handle h)
{
    block_header_t *block;

    MM_SLAM("entry");

    block = HANDLE_TO_BLOCK(h);
    if(block == NULL)
    {
        SET_MEM_ERR(nilHandleErr);
        return;
    }

    if(USE(block) == FREE)
    {
        SET_MEM_ERR(memWZErr);
        return;
    }

    SET_HANDLE_STATE(h, block, HANDLE_STATE(h, block) | PURGEBIT);
    MM_SLAM("exit");
    SET_MEM_ERR(noErr);
}

void HNoPurge(Handle h)
{
    block_header_t *block;

    MM_SLAM("entry");

    block = HANDLE_TO_BLOCK(h);
    if(block == NULL)
    {
        SET_MEM_ERR(nilHandleErr);
        return;
    }

    if(USE(block) == FREE)
    {
        SET_MEM_ERR(memWZErr);
        return;
    }

    SET_HANDLE_STATE(h, block, HANDLE_STATE(h, block) & ~PURGEBIT);
    MM_SLAM("exit");
    SET_MEM_ERR(noErr);
}

void HSetRBit(Handle h)
{
    block_header_t *block;

    MM_SLAM("entry");

    block = HANDLE_TO_BLOCK(h);
    if(block == NULL)
    {
        SET_MEM_ERR(nilHandleErr);
        return;
    }

    if(USE(block) == FREE)
    {
        SET_MEM_ERR(memWZErr);
        return;
    }

    SET_HANDLE_STATE(h, block, HANDLE_STATE(h, block) | RSRCBIT);
    MM_SLAM("exit");
    SET_MEM_ERR(noErr);
}

void HClrRBit(Handle h)
{
    block_header_t *block;

    MM_SLAM("entry");

    block = HANDLE_TO_BLOCK(h);
    if(block == NULL)
    {
        SET_MEM_ERR(nilHandleErr);
        return;
    }

    if(USE(block) == FREE)
    {
        SET_MEM_ERR(memWZErr);
        return;
    }

    SET_HANDLE_STATE(h, block, HANDLE_STATE(h, block) & ~RSRCBIT);
    MM_SLAM("exit");
    SET_MEM_ERR(noErr);
}

/* Zone sizes will be zero modulo this number (which must be a power of 2). */
#define ZONE_ALIGN_SIZE 8192

static void
pin_and_align(int *vp, int min, int max)
{
    int v = *vp;

    v = (v + ZONE_ALIGN_SIZE - 1) & ~(ZONE_ALIGN_SIZE - 1);

    if(v < min)
        v = min;
    else if(v > max)
        v = max;

    *vp = v;
}

static void
canonicalize_memory_sizes(void)
{
    pin_and_align(&ROMlib_applzone_size, MIN_APPLZONE_SIZE, MAX_APPLZONE_SIZE);
    pin_and_align(&ROMlib_syszone_size, MIN_SYSZONE_SIZE, MAX_SYSZONE_SIZE);
    pin_and_align(&ROMlib_stack_size, MIN_STACK_SIZE, MAX_STACK_SIZE);
}

void InitApplZone(void)
{
/* ApplZone must already be set before getting here */

/* nisus writer demands that `ApplLimit - ZONE_BK_LIM (ApplZone)' be
     greater than (or equal to?) 16384 */
#define APPLZONE_SLOP 16384

    canonicalize_memory_sizes();

#define INIT_APPLZONE_SIZE \
    (ROMlib_applzone_size + APPLZONE_SLOP)

    HeapEnd = RM((Ptr)(char *)MR(ApplZone)
                 + INIT_APPLZONE_SIZE
                 - (MIN_BLOCK_SIZE + APPLZONE_SLOP));

    InitZone(0, 64, (Ptr)HEAPEND, (Zone *)MR(ApplZone));
    MM_SLAM("exit");
    SET_MEM_ERR(noErr);
}

#define MANDELSLOP (32L * 1024)

void print_mem_full_message(void)
{
    fprintf(stderr,
            "Executor has run out of memory.  Try specifying "
            "a smaller -memory size.\n");
}

unsigned long ROMlib_total_allocated_memory;

void ROMlib_InitZones()
{
    static bool beenhere = false;
    static Ptr stack_begin, stack_end;
    static unsigned long applzone_memory_segment_size;
    int init_syszone_size;

#if ERROR_SUPPORTED_P(ERROR_MEMORY_MANAGER_SLAM)
    /* Temporarily turn off heap slamming while the heap is being set up. */
    bool old_debug_enabled_p;
    old_debug_enabled_p = error_set_enabled(ERROR_MEMORY_MANAGER_SLAM, false);
#endif

#define INIT_SYSZONE_SIZE \
    (ROMlib_syszone_size)

    if(!beenhere)
    {
        char *memory;
        unsigned long total_allocated_memory, total_mac_visible_memory;
        Ptr mem_top;

        canonicalize_memory_sizes();

#define STACK_SIZE ROMlib_stack_size

#if defined(MM_MANY_APPLZONES)
        applzone_memory_segment_size = (mm_n_applzones * INIT_APPLZONE_SIZE);
#else /* !MM_MANY_APPLZONES */
        applzone_memory_segment_size = INIT_APPLZONE_SIZE;
#endif /* !MM_MANY_APPLZONES */

        /* Determine total allocated memory.  Round up to next 8K
       * since that's the page size we pretend to have.
       */
        total_mac_visible_memory = (INIT_SYSZONE_SIZE
                                    + INIT_APPLZONE_SIZE
                                    + STACK_SIZE);
        total_mac_visible_memory = (total_mac_visible_memory + 8191) & ~8191;

        total_allocated_memory = (INIT_SYSZONE_SIZE
                                  + applzone_memory_segment_size
                                  + STACK_SIZE);
        total_allocated_memory = (total_allocated_memory + 8191) & ~8191;
        ROMlib_total_allocated_memory = total_allocated_memory;
        /* Note the memory in gestalt, rounded down the next 8K page multiple. */
        gestalt_set_memory_size(total_mac_visible_memory);

        /* Allocate memory for SysZone, ApplZone, and stack, contiguously. */
        memory = (char *)malloc(total_allocated_memory);

        if(memory == NULL)
        {
            print_mem_full_message();
            exit(-1);
        }

        /* can't assign to low-memory globals yet */

        mem_top = (Ptr)memory + total_allocated_memory;

        stack_begin = ((Ptr)memory
                       + INIT_SYSZONE_SIZE
                       + applzone_memory_segment_size);
        stack_end = stack_begin + STACK_SIZE;

        init_syszone_size = INIT_SYSZONE_SIZE;

        ROMlib_offset = (uintptr_t)memory;
#if SIZEOF_CHAR_P > 4
        ROMlib_sizes[0] = total_allocated_memory;
#endif
        {
            int low_global_room = (char *)&lastlowglobal - (char *)&nilhandle;
            memory += low_global_room;
            init_syszone_size -= low_global_room;
        }

        MemTop = RM(mem_top);

        SysZone = RM((THz)memory);
        ROMlib_syszone = (uintptr_t)memory;
        ROMlib_memtop = (uintptr_t)(memory + total_allocated_memory);
        InitZone(0, 32, (Ptr)((uintptr_t)MR(SysZone) + init_syszone_size),
                 (Zone *)MR(SysZone));
        beenhere = true;
    }

#if defined(MM_MANY_APPLZONES)
    if(mm_n_applzones != 1)
    {
        if(munmap((char *)MR(SysZone)
                      + INIT_SYSZONE_SIZE,
                  applzone_memory_segment_size)
           == -1)
            warning_errno("unable to `munmap ()' previous applzone");

        ApplZone = RM((THz)((char *)MR(SysZone)
                            + INIT_SYSZONE_SIZE
                            + (mm_current_applzone * INIT_APPLZONE_SIZE)));
        mm_current_applzone = (mm_current_applzone + 1) % mm_n_applzones;

        if(mmap((char *)MR(ApplZone), INIT_APPLZONE_SIZE,
                PROT_READ | PROT_WRITE,
                MAP_ANONYMOUS | MAP_FIXED | MAP_PRIVATE, -1, 0)
           == (void *)-1)
            errno_fatal("unable to `mmap ()' new applzone");
    }
    else
        ApplZone = RM((THz)((Ptr)MR(SysZone) + INIT_SYSZONE_SIZE));
#else /* !MM_MANY_APPLZONES */
    ApplZone = RM((THz)((Ptr)MR(SysZone) + INIT_SYSZONE_SIZE));
#endif

    Executor::InitApplZone();

    ApplLimit = RM(((Ptr)MR(ApplZone)
                    + INIT_APPLZONE_SIZE));

    EM_A7 = US_TO_SYN68K(stack_end - 16 - MANDELSLOP);

    MemErr = CWC(noErr);

#if ERROR_SUPPORTED_P(ERROR_MEMORY_MANAGER_SLAM)
    error_set_enabled(ERROR_MEMORY_MANAGER_SLAM, old_debug_enabled_p);
#endif
}

void SetApplBase(Ptr newbase)
{
    int32_t totend;

    if((char *)newbase < (char *)ZONE_BK_LIM(MR(SysZone)) + MIN_BLOCK_SIZE)
    {
        SET_MEM_ERR(noErr);
        return;
    }

    /* Find out how big this makes the last bit of the system zone */
    totend = (char *)newbase - (char *)ZONE_BK_LIM(MR(SysZone));

    if(totend >= 24) /* Make two blocks */
    {
        block_header_t *newfree = ZONE_BK_LIM(MR(SysZone));
        block_header_t *newlast = POINTER_TO_BLOCK(newbase);

        mm_set_block_fields_offset(newfree,
                                   FREE_BLOCK_STATE, FREE, 0,
                                   (char *)newlast - (char *)newfree, 0);

        mm_set_block_fields_offset(newlast,
                                   FREE_BLOCK_STATE, FREE, 0,
                                   MIN_BLOCK_SIZE, 0);
    }
    /* Otherwise, blow it off.  There isn't room for another block at the end,
     so we just let the old bkLim stand. */
    ApplZone = RM((THz)newbase);

    InitApplZone();
    SET_MEM_ERR(noErr);
}

void MoreMasters(void)
{
    THz current_zone;
    GUEST<Ptr> *handles;
    int i;

    MM_SLAM("entry");

    current_zone = MR(TheZone);

    handles = (GUEST<Ptr> *)NewPtr(ZONE_MORE_MAST(current_zone)
                                   * sizeof(uint32_t));

    if(handles == NULL)
    {
        SET_MEM_ERR(memFullErr);
        return;
    }

    handles[0] = ZONE_HFST_FREE_X(current_zone);
    ZONE_HFST_FREE_X(current_zone)
        = RM((Ptr)&handles[ZONE_MORE_MAST(current_zone) - 1]);

    for(i = ZONE_MORE_MAST(current_zone) - 1; i > 0; i--)
        handles[i] = RM((Ptr)&handles[i - 1]);

    MM_SLAM("exit");
    SET_MEM_ERR(noErr);
    return;
}

#if !defined(NDEBUG)
void print_free(void)
{
    printf("%d %d\n", CL(MR(ApplZone)->zcbFree), CL(MR(SysZone)->zcbFree));
}
#endif

void InitZone(GrowZoneProcPtr pGrowZone, int16_t cMoreMasters,
              Ptr limitPtr, THz zone)
{
    block_header_t *last_block;
    block_header_t *first_block;

    /* following line is needed for PPC version of Illustrator 5.5
     TODO: check mac carefully to see how they handle the final block */

    limitPtr = limitPtr - 8;

    last_block = (block_header_t *)((char *)limitPtr - MIN_BLOCK_SIZE);
    first_block = ZONE_HEAP_DATA(zone);

    zone->bkLim = RM((Ptr)last_block);
    zone->purgePtr = CLC_NULL;
    zone->hFstFree = CLC_NULL;
    zone->zcbFree = CL(limitPtr - (Ptr)zone - 64);
    zone->gzProc = RM(pGrowZone);
    zone->moreMast = CW(cMoreMasters);
    zone->flags = CWC(0);
    zone->minCBFree = CLC(0);
    zone->purgeProc = CLC_NULL;
    zone->cntRel = zone->cntNRel = zone->cntEmpty = zone->cntHandles
        = CWC(0);

    /* hypercard checks byte `30' (the high byte of the `maxNRel' field)
     of the Zone structure to determine if it can set the alloc
     pointer to the address of a non-relocatable block it just
     `DisposePtr ()'ed (which is allowed in 24bit zones).
     
     #### testing on the mac to see what the `max*' fields actually
     mean in a 32bit zone is required, but this is a good a guess as
     anything */
    zone->maxRel = zone->maxNRel = CWC(-1);

#if 0
  zone->sparePtr = CLC ((Ptr) 0x83d2);	/* From experimentation */
#else
    zone->sparePtr = nullptr; /* Better safe than sorry */
#endif

    zone->allocPtr = RM((Ptr)first_block);

    mm_set_block_fields_offset(first_block,
                               FREE_BLOCK_STATE, FREE, 0,
                               (char *)last_block - (char *)first_block, 0);

    mm_set_block_fields_offset(last_block,
                               FREE_BLOCK_STATE, FREE, 0,
                               MIN_BLOCK_SIZE, 0);

    TheZone = RM(zone);
    MoreMasters();

    MM_SLAM_ZONE(zone, "exit");
    SET_MEM_ERR(noErr);
}

THz GetZone(void)
{
    MM_SLAM("entry");

    SET_MEM_ERR(noErr);
    return MR(TheZone);
}

void SetZone(THz hz)
{
    MM_SLAM("entry");
    TheZone = RM(hz);
    SET_MEM_ERR(noErr);
}

Handle
_NewEmptyHandle_flags(bool sys_p)
{
    GUEST<THz> save_zone;
    THz current_zone;
    Handle h;

    MM_SLAM("entry");

    save_zone = TheZone;
    if(sys_p)
        TheZone = SysZone;

    current_zone = MR(TheZone);

    for(;;)
    {
        h = (Handle)ZONE_HFST_FREE(current_zone);
        if(h)
        {
            ZONE_HFST_FREE_X(current_zone) = *h;
            *h = NULL;
            SET_MEM_ERR(noErr);
            break;
        }
        else
        {
            MoreMasters();
            if(MemError() != noErr)
            {
                SET_MEM_ERR(memFullErr);
                break;
            }
        }
    }

    MM_SLAM("exit");
    TheZone = save_zone;
    return h;
}

Handle
_NewHandle_flags(Size size, bool sys_p, bool clear_p)
{
    Handle newh;
    block_header_t *block;
    GUEST<THz> save_zone;
    THz current_zone;

    MM_SLAM("entry");

    save_zone = TheZone;
    if(sys_p)
        TheZone = SysZone;
    current_zone = MR(TheZone);

    newh = NewEmptyHandle();
    if(newh == NULL)
    {
        SET_MEM_ERR(memFullErr);
        goto done;
    }

    size += HDRSIZE;
    if(ROMlib_relalloc(size, &block) != noErr)
    {
        DisposHandle(newh);
        newh = NULL;
        SET_MEM_ERR(memFullErr);
        goto done;
    }

    gui_assert(block < ZONE_BK_LIM(current_zone));

    ROMlib_setupblock(block, size, REL, newh, 0);
    *newh = BLOCK_DATA_X(block);

    if(clear_p)
        memset(BLOCK_DATA(block), 0, size - HDRSIZE);

    SET_MEM_ERR(noErr);

/* fall through */
done:

    MM_SLAM("exit");
    TheZone = save_zone;
    return newh;
}

#define TTS_HACK (ROMlib_options & ROMLIB_DISPOSHANDLE_HACK_BIT)

void DisposHandle(Handle h)
{
    MM_SLAM("entry");

    if(TTS_HACK && h == (MR(*(GUEST<Handle> *)SYN68K_TO_US(256))))
        h = 0;

    if(h)
    {
        block_header_t *block;
        THz current_zone;
        GUEST<THz> save_zone;

        save_zone = TheZone;
        current_zone = HandleZone(h);
        if(!current_zone)
        {
            SET_MEM_ERR(memAZErr);
            return;
        }
        TheZone = RM(current_zone);

        block = HANDLE_TO_BLOCK(h);

        if(*h && BLOCK_TO_HANDLE(MR(TheZone), block) != h)
        {
            TheZone = save_zone;
            SET_MEM_ERR(memAZErr);
            return;
        }

        if(block)
        {
            if(USE(block) == FREE)
            {
                TheZone = save_zone;
                SET_MEM_ERR(memWZErr);
                return;
            }

            if(HANDLE_STATE(h, block) & RSRCBIT)
            {
                if(h == (Handle)ROMlib_phoney_name_string)
                {
                    SET_MEM_ERR(memAZErr);
                    return;
                }
                warning_unexpected("disposing resource handle");
            }

            ROMlib_freeblock(block);
        }

        *h = ZONE_HFST_FREE_X(current_zone);
        ZONE_HFST_FREE_X(current_zone) = RM((Ptr)h);

        TheZone = save_zone;
    }

    MM_SLAM("exit");
    SET_MEM_ERR(noErr);
}

Size GetHandleSize(Handle h)
{
    block_header_t *block;
    Size retval;

    MM_SLAM("entry");

    block = HANDLE_TO_BLOCK(h);
    if(block == NULL)
    {
        SET_MEM_ERR(nilHandleErr);
        retval = 0;
    }
    else if(USE(block) == FREE)
    {
        SET_MEM_ERR(memWZErr);
        return 0;
    }
    else
    {
        SET_MEM_ERR(noErr);
        retval = LSIZE(block);
    }

    return retval;
}

void SetHandleSize(Handle h, Size newsize)
{
    block_header_t *block;
    int32_t oldpsize;
    GUEST<THz> save_zone;
    THz current_zone;
    bool save_memnomove_p;
    unsigned int state;

#if defined(X) /* what about MACOSX_? */
    if(h == MR(TEScrpHandle))
        WeOwnScrapX();
#endif

    MM_SLAM("entry");

    newsize += HDRSIZE; /* Header */

    block = HANDLE_TO_BLOCK(h);
    if(!block)
    {
        SET_MEM_ERR(nilHandleErr);
        return;
    }

    if(USE(block) == FREE)
    {
        SET_MEM_ERR(memWZErr);
        return;
    }

    save_zone = TheZone;
    current_zone = HandleZone(h);
    TheZone = RM(current_zone);
    state = HANDLE_STATE(h, block);

    oldpsize = PSIZE(block);
    if(newsize <= oldpsize)
        ROMlib_setupblock(block, newsize, REL, h, state);
    else
    {
        block_header_t *nextblock;
        block_header_t *newblock;

        nextblock = BLOCK_NEXT(block);

        /* First try and grow it forward */
        save_memnomove_p = ROMlib_memnomove_p;
        if(USE(nextblock) == REL && ROMlib_locked(block))
            ROMlib_memnomove_p = false;
        if(ROMlib_makespace(&nextblock, newsize - oldpsize))
        {
            ROMlib_memnomove_p = save_memnomove_p;
            ZONE_ZCB_FREE_X(current_zone)

                = CL(ZONE_ZCB_FREE(current_zone) - PSIZE(nextblock));

            SETPSIZE(block, oldpsize + PSIZE(nextblock));
            SETSIZEC(block, 0);
            ROMlib_setupblock(block, newsize, REL, h, state);
        }
        else
        {
            ROMlib_memnomove_p = save_memnomove_p;
            /* At this point, if the block is locked, we lose */
            if(ROMlib_locked(block))
                goto bad;

            GZRootHnd = RM(h);
            /* Now try and find the space elsewhere */
            if(ROMlib_relalloc(newsize, &newblock))
            {
                GZRootHnd = NULL;
                goto bad;
            }
            GZRootHnd = NULL;
            ROMlib_moveblock(block, newblock, newsize);
        }
    }

    /* And now we're done. */
    TheZone = save_zone;
    SET_MEM_ERR(noErr);
    MM_SLAM("exit");
    return;

bad:
    TheZone = save_zone;
    MM_SLAM("exit");
    SET_MEM_ERR(memFullErr);
}

#define HANDLE_IN_ZONE_P(handle, z)          \
    ((uintptr_t)(handle) >= (uintptr_t)MR(z) \
     && (uintptr_t)(handle) < (uintptr_t)ZONE_BK_LIM(MR(z)))

#define PTR_IN_ZONE_P(ptr, z)             \
    ((uintptr_t)(ptr) >= (uintptr_t)MR(z) \
     && (uintptr_t)(ptr) <= (uintptr_t)ZONE_BK_LIM(MR(z)))

THz HandleZone(Handle h)
{
    THz zone;
    block_header_t *block;
    bool applzone_p;
    bool syszone_p;

    MM_SLAM("entry");

    if(h == NULL)
    {
        SET_MEM_ERR(nilHandleErr);
        return NULL;
    }

    applzone_p = false;
    syszone_p = false;
    if(HANDLE_IN_ZONE_P(h, ApplZone))
    {
        Ptr p;

        p = STARH(h);
        if(p && !PTR_IN_ZONE_P(p, ApplZone))
        {
            SET_MEM_ERR(memAZErr);
            return NULL;
        }
        applzone_p = true;
    }
    else if(HANDLE_IN_ZONE_P(h, SysZone))
    {
        Ptr p;

        p = STARH(h);
        if(p && !PTR_IN_ZONE_P(p, SysZone))
        {
            SET_MEM_ERR(memAZErr);
            return NULL;
        }
        syszone_p = true;
    }
    /*
   * Prevent us from returning a zone when a dereference of the handle would
   * cause a segmentation fault.
   *
   * NOTE: we don't use the SysZone or MemTop low-memory globals, because
   *       it's possible that they have been modified in such a way that
   *       this test would fail even with an address that can legitimately
   *       be dereferenced.
   */
    else if(!VALID_ADDRESS(h))
    {
        SET_MEM_ERR(memAZErr);
        return NULL;
    }

    block = HANDLE_TO_BLOCK(h);
    if(block && USE(block) == FREE)
    {
        SET_MEM_ERR(memWZErr);
        return NULL;
    }

    if(block)
        zone = (THz)((Ptr)h - (int32_t)BLOCK_LOCATION_OFFSET(block));
    else if(applzone_p)
        zone = MR(ApplZone);
    else if(syszone_p)
        zone = MR(SysZone);
    else
        zone = MR(TheZone);

    SET_MEM_ERR(noErr);
    return zone;
}

Handle
_RecoverHandle_flags(Ptr p, bool sys_p)
{
    block_header_t *block;
    THz zones[3];
    Handle h = 0;
    int i;

    MM_SLAM("entry");

    block = POINTER_TO_BLOCK(p);

    if(sys_p)
    {
        zones[0] = MR(SysZone);
        zones[1] = MR(TheZone);
    }
    else
    {
        zones[0] = MR(TheZone);
        zones[1] = MR(SysZone);
    }
    zones[2] = MR(ApplZone);

    for(i = 0; i < 3; i++)
    {
        h = BLOCK_TO_HANDLE(zones[i], block);

        if((Ptr)h > (Ptr)zones[i]
           && (Ptr)h < (Ptr)ZONE_BK_LIM(zones[i])
           && STARH(h) == p)
            break;
    }

    if(i < 3)
        SET_MEM_ERR(noErr);
    else
    {
        h = 0;
        // FIXME: #warning FIND OUT WHAT A REAL MAC DOES HERE
    }
    return h;
}

void ReallocHandle(Handle h, Size size)
{
    block_header_t *oldb, *newb;
    int32_t newsize;
    GUEST<THz> save_zone;
    THz current_zone;
    unsigned int state;

    MM_SLAM("entry");

    if(h == NULL)
        warning_unexpected("called with NULL_STRING handle");

    oldb = HANDLE_TO_BLOCK(h);

    save_zone = TheZone;
    current_zone = HandleZone(h);
    TheZone = RM(current_zone);

    size += HDRSIZE;

    if(!oldb)
        state = 0;
    else
    {
        if(ROMlib_locked(oldb))
        {
            TheZone = save_zone;
            SET_MEM_ERR(memPurErr);
            return;
        }

        if(USE(oldb) == FREE)
        {
            TheZone = save_zone;
            SET_MEM_ERR(memWZErr);
            return;
        }

        state = BLOCK_STATE(oldb);
        if(PSIZE(oldb) >= (uint32_t)size)
        {
            ROMlib_setupblock(oldb, size, REL, h, state);
            goto done;
        }

        newb = BLOCK_NEXT(oldb);
        if(USE(newb) == FREE)
        {
            ROMlib_coalesce(newb);
            newsize = PSIZE(oldb) + PSIZE(newb);

            if(newsize >= size)
            {
                SETPSIZE(oldb, newsize);
                ZONE_ZCB_FREE_X(current_zone)
                    = CL(ZONE_ZCB_FREE(current_zone) - PSIZE(newb));
                ROMlib_setupblock(oldb, size, REL, h, state);
                goto done;
            }
        }
    }

    if(ROMlib_relalloc(size, &newb))
    {
        TheZone = save_zone;
        SET_MEM_ERR(memFullErr);
        return;
    }

    ROMlib_setupblock(newb, size, REL, h, state);
    SETMASTER(h, BLOCK_DATA(newb));

    if(oldb)
        ROMlib_freeblock(oldb);
/* fall through */
done:
    TheZone = save_zone;
    MM_SLAM("exit");
    SET_MEM_ERR(noErr);
}

#if 1 && !defined(NDEBUG)
int do_save_alloc = 0;
#endif

Ptr _NewPtr_flags(Size size, bool sys_p, bool clear_p)
{
    Ptr p;
    block_header_t *b;
    GUEST<THz> save_zone;
    THz current_zone;

    MM_SLAM("entry");

    save_zone = TheZone;
    if(sys_p)
        TheZone = SysZone;

    current_zone = MR(TheZone);

    size += HDRSIZE;

#if 1
    auto save_alloc_ptr = ZONE_ALLOC_PTR_X(current_zone);
#endif

    ZONE_ALLOC_PTR_X(current_zone) = CLC_NULL;

    ResrvMem(size);
    if(ROMlib_relalloc(size, &b))
    {
#if 0
      ZONE_ALLOC_PTR_X (current_zone) = save_alloc_ptr;
#endif
        TheZone = save_zone;
        SET_MEM_ERR(memFullErr);
        MM_SLAM("exit");
        return NULL;
    }

#if 1 && !defined(NDEBUG)
    if(do_save_alloc)
        ZONE_ALLOC_PTR_X(current_zone)
            = save_alloc_ptr;
    else
        checkallocptr();
#endif

    gui_assert(b < ZONE_BK_LIM(current_zone));

    ROMlib_setupblock(b, size, NREL, 0);
    p = BLOCK_DATA(b);

    if(clear_p)
        memset(p, 0, size - HDRSIZE);

    TheZone = save_zone;
    SET_MEM_ERR(noErr);
    MM_SLAM("exit");
    return p;
}

void DisposPtr(Ptr p)
{
    MM_SLAM("entry");

    if(p)
    {
        block_header_t *block;
        THz zone;

        block = POINTER_TO_BLOCK(p);

        if(USE(block) == FREE)
        {
            SET_MEM_ERR(memWZErr);
            return;
        }

        zone = PtrZone(p);
        if(zone)
        {
            TheZoneGuard guard(RM(zone));

            ROMlib_freeblock(block);
        }
        else
            warning_unexpected(NULL_STRING);
    }
    SET_MEM_ERR(noErr);
    MM_SLAM("exit");
}

Size GetPtrSize(Ptr p)
{
    block_header_t *block;

    MM_SLAM("entry");

    block = POINTER_TO_BLOCK(p);

    if(USE(block) == FREE)
    {
        SET_MEM_ERR(memWZErr);
        return 0;
    }

    SET_MEM_ERR(noErr);
    MM_SLAM("exit");
    return LSIZE(block);
}

void SetPtrSize(Ptr p, Size newsize)
{
    block_header_t *block;
    LONGINT oldpsize;
    GUEST<THz> save_zone;
    THz current_zone;

    MM_SLAM("entry");

    if(p == NULL)
        warning_unexpected("attempt to set NULL_STRING pointer size");

    newsize += HDRSIZE;
    block = POINTER_TO_BLOCK(p);

    if(USE(block) == FREE)
    {
        SET_MEM_ERR(memWZErr);
        return;
    }

    save_zone = TheZone;
    current_zone = PtrZone(p);

    if(!current_zone)
    {
        warning_unexpected(NULL_STRING);
        SET_MEM_ERR(memWZErr); /* Not really sure what we should return in
				 this case.  It's not like the Mac has any
				 decent error semantics */
    }
    else
    {
        TheZone = RM(current_zone);

        oldpsize = PSIZE(block);
        if(newsize <= oldpsize)
            ROMlib_setupblock(block, newsize, NREL, 0);
        else
        {
            block_header_t *nextblock;

            nextblock = BLOCK_NEXT(block);

            /* First try and grow it forward */
            if(ROMlib_makespace(&nextblock, newsize - oldpsize))
            {
                // FIXME: #warning original code was endian-inconsistent
                ZONE_ZCB_FREE_X(current_zone)
                    = CL(ZONE_ZCB_FREE(current_zone) - PSIZE(nextblock));

                SETPSIZE(block, oldpsize + PSIZE(nextblock));
                SETSIZEC(block, 0);
                ROMlib_setupblock(block, newsize, NREL, 0);
            }
            else
            {
                TheZone = save_zone;
                SET_MEM_ERR(memFullErr);
                return;
            }
        }
    }

    /* And now we're done. */
    TheZone = save_zone;
    SET_MEM_ERR(noErr);
    MM_SLAM("exit");
}

PRIVATE bool
legit_addr_p(void *addr)
{
    bool retval;

    retval = true; /* all addresses are valid for now */
    return retval;
}

PRIVATE bool
legit_zone_p(THz zone)
{
    bool retval;

    if(!legit_addr_p(zone))
        retval = false;
    else
    {
        block_header_t *blockp;

        blockp = ZONE_HEAP_DATA(zone);
        retval = BLOCK_LOCATION_ZONE(blockp) == zone;
    }

    return retval;
}

THz PtrZone(Ptr p)
{
    block_header_t *block;
    THz zone;

    MM_SLAM("entry");

    if(p == NULL)
        warning_unexpected("attempt to set NULL_STRING pointer size");

    block = POINTER_TO_BLOCK(p);

    if(USE(block) == FREE)
    {
        SET_MEM_ERR(memWZErr);
        /* don't count on this value (IMII-38) */
        return NULL;
    }

    zone = BLOCK_LOCATION_ZONE(block);

    if(!legit_zone_p(zone))
    {
        SET_MEM_ERR(memWZErr); /* not sure what this should be */
        return NULL;
    }

    SET_MEM_ERR(noErr);
    return zone;
}

int32_t _FreeMem_flags(bool sys_p)
{
    uint32_t freespace;

    MM_SLAM("entry");

    if(sys_p)
        freespace = ZONE_ZCB_FREE(MR(SysZone));
    else
        freespace = ZONE_ZCB_FREE(MR(TheZone));

    SET_MEM_ERR(noErr);
    return freespace;
}

Size _MaxMem_flags(Size *growp, bool sys_p)
{
    block_header_t *b;
    GUEST<THz> save_zone;
    THz current_zone;
    uint32_t biggestfree;
    uint32_t sizesofar;
    block_header_t *startb;
    Size grow;
    enum
    {
        SEARCHING,
        COUNTING
    } state;

    MM_SLAM("entry");

    sizesofar = 0;
    startb = 0;

    save_zone = TheZone;
    if(sys_p)
        TheZone = SysZone;

    current_zone = MR(TheZone);

    /* Purge everyone */
    for(b = ZONE_HEAP_DATA(current_zone);
        b != ZONE_BK_LIM(current_zone);
        b = BLOCK_NEXT(b))
    {
        Handle h;

        if(PSIZE(b) < MIN_BLOCK_SIZE)
            HEAP_DEATH();

        h = BLOCK_TO_HANDLE(current_zone, b);
        if(USE(b) == REL
           && (HANDLE_STATE(h, b) & (LOCKBIT | PURGEBIT)) == PURGEBIT)
            EmptyHandle(h);
    }

    /* Compact the whole thing */
    CompactMem(0x7FFFFFFF);

    /* Compress the free blocks */
    state = SEARCHING;
    biggestfree = 0;

    for(b = ZONE_HEAP_DATA(current_zone);
        b != ZONE_BK_LIM(current_zone);
        b = BLOCK_NEXT(b))
    {
        if(PSIZE(b) < MIN_BLOCK_SIZE)
            HEAP_DEATH();

        if(state == SEARCHING)
        {
            if(USE(b) == FREE)
            {
                startb = b;
                sizesofar = PSIZE(b);
                state = COUNTING;
            }
        }
        else
        {
            if(USE(b) == FREE)
                sizesofar += PSIZE(b);
            else
            {
                SETPSIZE(startb, sizesofar);
                if(ZONE_ALLOC_PTR(current_zone) > startb
                   && ((char *)ZONE_ALLOC_PTR(current_zone)
                       < (char *)startb + sizesofar))
                    ZONE_ALLOC_PTR_X(current_zone) = RM((Ptr)startb);
                if(sizesofar > biggestfree)
                    biggestfree = sizesofar;
                state = SEARCHING;
            }
        }
    }
    if(state == COUNTING)
    {
        SETPSIZE(startb, sizesofar);
        if(ZONE_ALLOC_PTR(current_zone) > startb
           && ((char *)ZONE_ALLOC_PTR(current_zone)
               < (char *)startb + sizesofar))
            ZONE_ALLOC_PTR_X(current_zone) = RM((Ptr)startb);

        if(sizesofar > biggestfree)
            biggestfree = sizesofar;
    }

    if(TheZone == ApplZone)
        grow = (Ptr)MR(ApplLimit) - (Ptr)HEAPEND;
    else
        grow = 0;

    TheZone = save_zone;

    *growp = grow;
    SET_MEM_ERR(noErr);
    MM_SLAM("exit");
    return biggestfree;
}

Size _CompactMem_flags(Size sizeneeded, bool sys_p)
{
    int32_t amtfree;
    block_header_t *src, *target, *ap;
    GUEST<THz> save_zone;
    THz current_zone;
    bool startfront_p;

    MM_SLAM("entry");

    save_zone = TheZone;
    if(sys_p)
        TheZone = SysZone;
    current_zone = MR(TheZone);

    /* We've seen HyperCard load allocPtr with -8 before ... yahoo.
     Specifically, HC 2.1 would do this after you use the Chart making
     stack to make a chart and then you try to "Go Home" via the menu */

    ap = ZONE_ALLOC_PTR(current_zone);
    if(ap >= ZONE_HEAP_DATA(current_zone)
       && ap <= ZONE_BK_LIM(current_zone))
        src = target = ap;
    else
        src = target = ZONE_HEAP_DATA(current_zone);
    startfront_p = (src == ZONE_HEAP_DATA(current_zone));

repeat:
    ZONE_ALLOC_PTR_X(current_zone) = CLC_NULL;
    amtfree = 0;

    while(src != ZONE_BK_LIM(current_zone)
          && amtfree < sizeneeded)
    {
        if(USE(src) == REL && !ROMlib_locked(src))
        {
            if(src == target)
                src = target = BLOCK_NEXT(target);
            else if(src < target)
                gui_abort();
            else
            {
                *target = *src;

                SETMASTER(BLOCK_TO_HANDLE(current_zone, src),
                          BLOCK_DATA(target));

                BlockMove(BLOCK_DATA(src), BLOCK_DATA(target), LSIZE(src));

                src = (block_header_t *)((char *)src + PSIZE(target));
                target = BLOCK_NEXT(target);
            }
        }
        else if(USE(src) == FREE)
            src = BLOCK_NEXT(src);
        else
        {
            if(src > target)
            {
                int32_t src_target_diff = (char *)src - (char *)target;

                mm_set_block_fields_offset(target,
                                           FREE_BLOCK_STATE, FREE, 0,
                                           src_target_diff, 0);

                if(src_target_diff > amtfree)
                {
                    amtfree = src_target_diff;
                    if(!ZONE_ALLOC_PTR_X(current_zone))
                        ZONE_ALLOC_PTR_X(current_zone) = RM((Ptr)target);
                }
            }
            src = target = BLOCK_NEXT(src);
        }
    }

    /* If we got out because we hit the end, do the last case above for 
     the final free block. */
    if(src == ZONE_BK_LIM(current_zone) && src > target)
    {
        int32_t src_target_diff = (char *)src - (char *)target;

        mm_set_block_fields_offset(target,
                                   FREE_BLOCK_STATE, FREE, 0,
                                   src_target_diff, 0);

        if(src_target_diff > amtfree)
        {
            amtfree = src_target_diff;
            if(!ZONE_ALLOC_PTR_X(current_zone))
                ZONE_ALLOC_PTR_X(current_zone) = RM((Ptr)target);
        }
    }

    if(amtfree < sizeneeded && !startfront_p)
    {
        startfront_p = true;
        src = target = ZONE_HEAP_DATA(current_zone);
        goto repeat;
    }

    TheZone = save_zone;
    SET_MEM_ERR(noErr);
    MM_SLAM("exit");
    return amtfree;
}

void _ResrvMem_flags(Size needed, bool sys_p)
{
    GUEST<THz> save_zone;
    THz current_zone;
    block_header_t *b;
    Size free;
    long avail;
    bool already_maxed_p;

    MM_SLAM("entry");

    if(needed <= 0)
    {
        SET_MEM_ERR(noErr);
        return;
    }

    save_zone = TheZone;
    if(sys_p)
        TheZone = SysZone;
    current_zone = MR(TheZone);
    already_maxed_p = false;

again:
    for(b = ZONE_HEAP_DATA(current_zone);
        b != ZONE_BK_LIM(current_zone);
        b = BLOCK_NEXT(b))
    {
        if(PSIZE(b) < MIN_BLOCK_SIZE)
            HEAP_DEATH();

        if(ROMlib_makespace(&b, needed))
        {
            TheZone = save_zone;
            MM_SLAM("exit");
            SET_MEM_ERR(noErr);
            return;
        }
    }

    avail = MaxMem(&free);
    if(avail >= needed && !already_maxed_p)
    {
        already_maxed_p = true;
        goto again;
    }
    if(free >= needed)
    {
        /* relalloc will do the actual extension. */
        TheZone = save_zone;
        MM_SLAM("exit");
        SET_MEM_ERR(noErr);
        return;
    }

    TheZone = save_zone;
    MM_SLAM("exit");
    SET_MEM_ERR(memFullErr);
}

void _PurgeMem_flags(Size sizeneeded, bool sys_p)
{
    long amount_free, max_free;
    block_header_t *b;
    GUEST<THz> save_zone;
    THz current_zone;

    MM_SLAM("entry");

    amount_free = 0;

    save_zone = TheZone;
    if(sys_p)
        TheZone = SysZone;
    current_zone = MR(TheZone);

    max_free = 0;
    for(b = ZONE_HEAP_DATA(current_zone);
        b != ZONE_BK_LIM(current_zone);
        b = BLOCK_NEXT(b))
    {
        Handle h;

        if(PSIZE(b) < MIN_BLOCK_SIZE)
            HEAP_DEATH();

        h = BLOCK_TO_HANDLE(current_zone, b);

        if(USE(b) == REL
           && (HANDLE_STATE(h, b) & (LOCKBIT | PURGEBIT)) == PURGEBIT)
            EmptyHandle(h);

        amount_free = ROMlib_amtfree(b);
        if(amount_free >= max_free)
            max_free = amount_free;
        if(amount_free >= sizeneeded)
            break;
    }

    TheZone = save_zone;

    if(amount_free < sizeneeded)
        SET_MEM_ERR(memFullErr);
    else
        SET_MEM_ERR(noErr);
    MM_SLAM("exit");
}

PRIVATE void
BlockMove_and_possibly_flush_cache(Ptr src, Ptr dst, Size cnt,
                                   bool flush_p)
{
    if(cnt > 0)
    {
        /* ugly, but probably better than crashing when we try to
	 dereference 0 */

        if(!dst)
            dst = (Ptr)SYN68K_TO_US(0);

        if(!src)
            src = (Ptr)SYN68K_TO_US(0);

        memmove(dst, src, cnt);
        if(flush_p)
            ROMlib_destroy_blocks(US_TO_SYN68K(dst), cnt, true);
    }

    /* don't use `SET_MEM_ERR' since that will do a heap slam and we
     will lose */
    MemErr = CWC(noErr);
}

void BlockMove(Ptr src, Ptr dst, Size cnt)
{
    BlockMove_and_possibly_flush_cache(src, dst, cnt, true);
}

void BlockMoveData(Ptr src, Ptr dst, Size cnt)
{
    BlockMove_and_possibly_flush_cache(src, dst, cnt, false);
}

void BlockMove_the_trap(Ptr src, Ptr dst, Size cnt, bool flush_p)
{
    MM_SLAM("entry");
    BlockMove_and_possibly_flush_cache(src, dst, cnt, flush_p);
    MM_SLAM("exit");
}

void MaxApplZone(void)
{
    MM_SLAM("entry");

    /* #warning MaxApplZone does not do anything -- we start out with max */
    SET_MEM_ERR(noErr);
}

void MoveHHi(Handle h)
{
    MM_SLAM("entry");

    /* Oh No! More Lemmings appears to assume that MoveHHi will
   * flush the cache.  This is not an unreasonable assumption, since
   * large BlockMove's are guaranteed to flush the cache, and
   * MoveHHi of a large piece of memory would typically involve a
   * big BlockMove.  Since MoveHHi isn't called very often,
   * and is expected to be fairly slow, we use this opportunity
   * to flush the cache.  We only nuke code whose checksums
   * have changed, for speed.
   *
   * We make an exception for Microsoft Word, because the spelling checker
   * will be real slow if we don't.
   */

    if(ROMlib_creator != TICK("MSWD") && ROMlib_creator != TICK("ddOr"))
        ROMlib_destroy_blocks(0, ~0, true);

    /* #### there used to be a lot of code here; but it was unused and
     returned noErr #if !MOVEHIWORKS -- see the rcsfile for details */
    /* #warning MoveHHi not implemented */
    /* #### just emits too many unecessary warnings
     warning_unimplemented (""); */
    SET_MEM_ERR(noErr);
}

int32_t _MaxBlock_flags(bool sys_p)
{
    GUEST<THz> save_zone;
    THz current_zone;
    int32_t max_free;
    int32_t total_free;
    block_header_t *b;

    MM_SLAM("entry");

    save_zone = TheZone;
    if(sys_p)
        TheZone = SysZone;
    current_zone = MR(TheZone);

    max_free = total_free = 0;

    for(b = ZONE_HEAP_DATA(current_zone);
        b != ZONE_BK_LIM(current_zone);
        b = BLOCK_NEXT(b))
    {
        if(PSIZE(b) < MIN_BLOCK_SIZE)
            HEAP_DEATH();

        if(USE(b) == FREE)
            total_free += PSIZE(b);
        else if(USE(b) == NREL || ROMlib_locked(b))
        {
            if(total_free > max_free)
                max_free = total_free;
            total_free = 0;
        }
    }

    TheZone = save_zone;
    SET_MEM_ERR(noErr);
    MM_SLAM("exit");
    return MAX(total_free, max_free) - HDRSIZE;
}

void _PurgeSpace_flags(Size *total_out, Size *contig_out, bool sys_p)
{
    GUEST<THz> save_zone;
    THz current_zone;
    int32_t total_free;
    int32_t this_contig;
    int32_t max_contig;
    block_header_t *b;

    MM_SLAM("entry");

    save_zone = TheZone;
    if(sys_p)
        TheZone = SysZone;
    current_zone = MR(TheZone);

    total_free = this_contig = max_contig = 0;
    for(b = ZONE_HEAP_DATA(current_zone);
        b != ZONE_BK_LIM(current_zone);
        b = BLOCK_NEXT(b))
    {
        Handle h;

        if(PSIZE(b) < MIN_BLOCK_SIZE)
            HEAP_DEATH();

        h = BLOCK_TO_HANDLE(current_zone, b);
        if(USE(b) == FREE
           || (USE(b) == REL
               && (HANDLE_STATE(h, b) & (LOCKBIT | PURGEBIT)) == PURGEBIT))
        {
            this_contig += PSIZE(b);
            total_free += PSIZE(b);
        }
        else if(USE(b) == NREL || ROMlib_locked(b))
        {
            if(this_contig > max_contig)
                max_contig = this_contig;
            this_contig = 0;
        }
    }

    TheZone = save_zone;

    SET_MEM_ERR(noErr);
    *total_out = total_free - HDRSIZE;
    *contig_out = MAX(this_contig, max_contig) - HDRSIZE;
    MM_SLAM("exit");
}

Size StackSpace(void)
{
    intptr_t fp;

    MM_SLAM("entry");

    fp = (intptr_t)SYN68K_TO_US(EM_A7);

    /* Stack pointer on return is fp + 8 (4 for old fp, 4 for return
     address) */

    SET_MEM_ERR(noErr);
    return (fp + 8) - (intptr_t)HEAPEND;
}

void SetApplLimit(Ptr new_limit)
{
    /* NOTE TO CLIFF: 
     We can't do any sanity checks here (not even a brk()), since
     the ApplLimit might be directly changed by programs, and we have
     to deal with that.  */
    /* Making the ApplLimit too small has no effect (IMII-30), and making
     it too big shouldn't cause a problem until the excess memory starts
     being used (by incrementation of HEAPEND). */

    MM_SLAM("entry");

    ApplLimit = RM(new_limit);
    HeapEnd = RM(new_limit - MIN_BLOCK_SIZE);

    SET_MEM_ERR(noErr);
}

void SetGrowZone(GrowZoneProcPtr newgz)
{
    MM_SLAM("entry");

    ZONE_GZ_PROC_X(MR(TheZone)) = RM(newgz);
    SET_MEM_ERR(noErr);
}

void EmptyHandle(Handle h)
{
    GUEST<THz> save_zone;
    THz current_zone;
    block_header_t *b;

    MM_SLAM("entry");

    b = HANDLE_TO_BLOCK(h);
    if(b == NULL)
    {
        SET_MEM_ERR(noErr);
        return;
    }

    save_zone = TheZone;
    current_zone = HandleZone(h);
    TheZone = RM(current_zone);

    if(ROMlib_locked(b))
    {
        TheZone = save_zone;
        SET_MEM_ERR(memPurErr);
        return;
    }
    if(USE(b) == FREE)
    {
        TheZone = save_zone;
        SET_MEM_ERR(memWZErr);
        return;
    }

    if(ZONE_PURGE_PROC_X(current_zone))
    {
        uint32_t saved0, saved1, saved2, savea0, savea1;
        ROMlib_hook(memory_purgeprocnumber);

        saved0 = EM_D0;
        saved1 = EM_D1;
        saved2 = EM_D2;
        savea0 = EM_A0;
        savea1 = EM_A1;
        PUSHADDR(US_TO_SYN68K(h));
        CALL_EMULATOR(US_TO_SYN68K(ZONE_PURGE_PROC(current_zone)));
        EM_D0 = saved0;
        EM_D1 = saved1;
        EM_D2 = saved2;
        EM_A0 = savea0;
        EM_A1 = savea1;
    }

    ROMlib_freeblock(b);
    SETMASTER(h, NULL);

    TheZone = save_zone;
    MM_SLAM("exit");
    SET_MEM_ERR(noErr);
}

/* Fluff for Cliff */
void ROMlib_installhandle(Handle sh, Handle dh)
{
    GUEST<THz> save_zone;

    MM_SLAM("entry");
    save_zone = TheZone;
    TheZone = RM(HandleZone(dh));

    if(true
       || ROMlib_locked(HANDLE_TO_BLOCK(dh))
       || HandleZone(sh) != MR(TheZone))
    {
        Size size;

        size = GetHandleSize(sh);
        SetHandleSize(dh, size);
        if(MemErr == CWC(noErr))
            BlockMove(STARH(sh), STARH(dh), size);
        DisposHandle(sh);
    }
    else
    {
        block_header_t *db = HANDLE_TO_BLOCK(dh);
        block_header_t *sb = HANDLE_TO_BLOCK(sh);
        ROMlib_freeblock(db);
        SETMASTER(dh, STARH(sh));
        BLOCK_LOCATION_OFFSET_X(sb) = CL((Ptr)dh - (Ptr)MR(TheZone));
        *sh = guest_cast<Ptr>(ZONE_HFST_FREE_X(MR(TheZone)));
        ZONE_HFST_FREE_X(MR(TheZone)) = RM((Ptr)sh);
    }
    TheZone = save_zone;
    MM_SLAM("exit");
}

OSErr MemError(void)
{
    MM_SLAM("entry");

    return CW(MemErr);
}

THz SystemZone(void)
{
    MM_SLAM("entry");

    return MR(SysZone);
}

THz ApplicZone(void)
{
    MM_SLAM("entry");

    return MR(ApplZone);
}

/* Like NewHandle, but fills in the newly allocated memory by copying
 * data from the supplied pointer.  Use the NewHandle_copy_ptr and
 * NewHandleSys_copy_ptr macros to access this function.
 */
Handle
_NewHandle_copy_ptr_flags(Size size, const void *data_to_copy,
                          bool sys_p)
{
    Handle h;

    h = _NewHandle_flags(size, sys_p, false);
    if(MemErr == CWC(noErr))
        memcpy(STARH(h), data_to_copy, size);
    return h;
}

/* Like NewHandle, but fills in the newly allocated memory by copying
 * data from the supplied Handle.  Use the NewHandle_copy_handle and
 * NewHandleSys_copy_handle macros to access this function.
 */
Handle
_NewHandle_copy_handle_flags(Size size, Handle data_to_copy, bool sys_p)
{
    Handle h;

    if(GetHandleSize(data_to_copy) < size)
        warning_unexpected("Not enough bytes to copy!");
    h = _NewHandle_flags(size, sys_p, false);
    if(MemErr == CWC(noErr))
        memcpy(STARH(h), STARH(data_to_copy), size);
    return h;
}

/* Like NewPtr, but fills in the newly allocated memory by copying
 * data from the supplied pointer.  Use the NewPtr_copy_ptr and
 * NewPtrSys_copy_ptr macros to access this function.
 */
Ptr _NewPtr_copy_ptr_flags(Size size, const void *data_to_copy,
                           bool sys_p)
{
    Ptr p;

    p = _NewPtr_flags(size, sys_p, false);
    if(MemErr == CWC(noErr))
        memcpy(p, data_to_copy, size);
    return p;
}

/* Like NewPtr, but fills in the newly allocated memory by copying
 * data from the supplied Handle.  Use the NewPtr_copy_handle and
 * NewPtrSys_copy_handle macros to access this function.
 */
Ptr _NewPtr_copy_handle_flags(Size size, Handle data_to_copy, bool sys_p)
{
    Ptr p;

    if(GetHandleSize(data_to_copy) < size)
        warning_unexpected("Not enough bytes to copy!");
    p = _NewPtr_flags(size, sys_p, false);
    if(MemErr == CWC(noErr))
        memcpy(p, STARH(data_to_copy), size);
    return p;
}
}
