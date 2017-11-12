#include "rsys/common.h"

/*
 * This file is a quick hack to hoist code from config/os/linux to where it
 * can be shared with the Mac OS X port.
 *
 * Eventually everything should be rejiggered to use the GNU build system.
 */

#if defined(LINUX) || defined(MACOSX)

#include <sys/types.h>
#include <sys/mman.h>

#include "rsys/os.h"
#include "rsys/memsize.h"
#include "rsys/mman.h"
#include "rsys/system_error.h"
#include "rsys/memory_layout.h"
#include "rsys/lowglobals.h"

#include "Gestalt.h"
#include "SegmentLdr.h"

#include "rsys/gestalt.h"

#include "rsys/lockunlock.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

using namespace Executor;

#if defined(MACOSX_)
// The code in here should work for Mac OS X as well as Linux, although
// in Mac OS X they use MAP_ANON instead of MAP_ANONYMOUS
#warning "Fix this, the code isn't Linux specific"
#define MAP_ANONYMOUS MAP_ANON
#endif

static void
my_fault_proc(int sig)
{
    // FIXME:  Change this to an internal Executor dialog
    fprintf(stderr, "Unexpected Application Failure\n");

    // If we are already in the browser, does this exit the program?
    C_ExitToShell();
}

static int except_list[] = { SIGSEGV, SIGBUS, 0 };

void install_exception_handler(void)
{
    int i;

    for(i = 0; except_list[i]; ++i)
    {
        signal(except_list[i], my_fault_proc);
    }
}

void uninstall_exception_handler(void)
{
    int i;

    for(i = 0; except_list[i]; ++i)
    {
        signal(except_list[i], SIG_DFL);
    }
}

static unsigned long
physical_memory(void)
{
    FILE *fp;
    unsigned long mem;

    mem = 0;
    fp = fopen("/proc/meminfo", "r");
    if(fp)
    {
        char buf[256];

        while(fgets(buf, sizeof buf - 1, fp))
            if(!strncmp(buf, "Mem:", 4) && sscanf(buf + 4, "%lu", &mem))
                break;

        fclose(fp);
    }

    replace_physgestalt_selector(gestaltPhysicalRAMSize, mem);
    return mem;
}

static void
guess_good_memory_settings(void)
{
    unsigned long new_appl_size;

    new_appl_size = physical_memory() / 4;

#if defined(powerpc) || defined(__ppc__)

    /* This hack prevents Photoshop 5.5 demo from complaining that we don't
     have enough memory when we run on a 64 MB Linux machine.  Our division
     by four is a bit naive above, so there's really no harm, other than
     ugliness, to this hack.  */

    {
        enum
        {
            PHOTOSHOP_55_PREFERRED_SIZE = 16584 * 1024
        };

        if(new_appl_size < PHOTOSHOP_55_PREFERRED_SIZE && new_appl_size >= PHOTOSHOP_55_PREFERRED_SIZE * 8 / 10)
            new_appl_size = PHOTOSHOP_55_PREFERRED_SIZE;
    }

#endif

    if(new_appl_size > ROMlib_applzone_size)
        ROMlib_applzone_size = MIN(MAX_APPLZONE_SIZE, new_appl_size);
}

bool Executor::os_init(void)
{
    guess_good_memory_settings();
#if defined(SDL)
    install_exception_handler();
#endif
    return true;
}

PUBLIC int
Executor::ROMlib_lockunlockrange(int fd, uint32 begin, uint32 count, lockunlock_t op)
{
    int retval;
    struct flock flock;

    warning_trace_info("fd = %d, begin = %d, count = %d, op = %d",
                       fd, begin, count, op);
    retval = noErr;
    switch(op)
    {
        case lock:
            flock.l_type = F_WRLCK;
            break;
        case unlock:
            flock.l_type = F_UNLCK;
            break;
        default:
            warning_unexpected("op = %d", op);
            retval = paramErr;
            break;
    }

    if(retval == noErr)
    {
        bool success;

        flock.l_whence = SEEK_SET;
        flock.l_start = begin;
        flock.l_len = count;

        success = fcntl(fd, F_SETLK, &flock) != -1;
        if(success)
            retval = noErr;
        else
        {
            switch(errno)
            {
                case EAGAIN:
                case EACCES:
                    retval = fLckdErr;
                    break;
#if 0
	    case ERROR_NOT_LOCKED:
	      retval = afpRangeNotLocked;
	      break;
#endif
#if 0
	    case ERROR_LOCK_FAILED:
	      retval = afpRangeOverlap;
	      break;
#endif
                default:
                    warning_unexpected("errno = %d", errno);
                    retval = noErr;
                    break;
            }
        }
    }
    return retval;
}

PUBLIC int
ROMlib_launch_native_app(int n_filenames, char **filenames)
{
    char **v;

    v = (char **)alloca(sizeof *v * (n_filenames + 1));
    memcpy(v, filenames, n_filenames * sizeof *v);
    v[n_filenames] = 0;
    if(fork() == 0)
        execv(filenames[0], v);

    return 0;
}

/*
 * There is a very bad problem associated with the use of the db
 * shared libraries under Linux.  Specifically, the calling convention
 * for functions which return structs that are larger than 32 bits
 * somehow got changed when some of the Linux distributions switched
 * from gcc to egcs.  Both compilers put an extra pointer on the stack
 * before calling the routine that returns the large struct, but gcc
 * expects the caller to pop that extra pointer, where egcs expects
 * the called to pop it.  This means that if you compile the caller
 * with gcc and call a shared library that was called with egcs, the
 * stack pointer will be off by 4 bytes after the function returns and
 * the stack is adjusted.  That can be a catastrophe if further code
 * expects the stack to be correct after adjustments.  On the other
 * hand, if we make the questionable call and then do nothing else,
 * the "leave" instruction will restore the stack pointer by using the
 * frame pointer and we'll never be bothered by the extra pop.
 *
 * So, as a workaround, we can wrap the routines, then check the
 * assembly code that the compiler produces to make sure that it's
 * tolerant of the error, then call the wrappers.  That makes the
 * wrapper routines look like voodoo code that was written by a
 * superstitious programmer, but the code (or some other workaround)
 * is absolutely necessary because we want to have one Executor
 * executable that can run with both the shared libraries from Red Hat
 * 5.2 as well as the new ones in SuSE 6.0 (and the new ones that will
 * probably be in Red Hat 6.0).
 *
 * These wrappers absolutely have to be compiled with enough
 * optimization so that the stack isn't adjusted before the leave
 * instruction.  If it is adjusted, then any interrupt that occurs
 * between the adjustment and the transfering of the data from the
 * temporary stack space to the address passed will cause corruption.
 *
 * If you do not understand the above, or if you disagree with it,
 * please contact Cliff before changing the following code.  Axing the
 * code alone and then testing the result is not sufficient, unless
 * you're sure that your test involves *both* db shared libaries.
 *
 */

PUBLIC void
_dbm_fetch(datum *datump, DBM *db, datum datum)
{
    *datump = dbm_fetch(db, datum);
}

PUBLIC void
_dbm_firstkey(datum *datump, DBM *db)
{
    *datump = dbm_firstkey(db);
}

PUBLIC void
_dbm_nextkey(datum *datump, DBM *db)
{
    *datump = dbm_nextkey(db);
}

PUBLIC bool Executor::host_has_spfcommon(void)
{
    return false;
}

PUBLIC bool
Executor::host_spfcommon(host_spf_reply_block *replyp, const char *prompt,
                         const char *incoming_filename, void *fp, void *filef, int numt,
                         void *tl, getorput_t getorput, sf_flavor_t flavor,
                         void *activeList, void *activateproc, void *yourdatap)
{
    return false;
}

void mmap_lowglobals(void)
{
    if(!force_big_offset)
    {
        caddr_t addr;

        addr = (caddr_t)mmap((caddr_t)PAGE_ZERO_START,
                             PAGE_ZERO_SIZE,
                             PROT_READ | PROT_WRITE,
                             MAP_ANONYMOUS | MAP_FIXED | MAP_PRIVATE, -1, 0);
        gui_assert(addr == (caddr_t)PAGE_ZERO_START);
    }
}

#if !defined(powerpc) && !defined(__ppc__)
PRIVATE caddr_t
round_up_to_page_size(unsigned long addr)
{
    caddr_t retval;
    size_t page_size;

    page_size = getpagesize();
    retval = (caddr_t)((addr + page_size - 1) / page_size * page_size);
    return retval;
}
#endif

static jmp_buf segv_return;

static void
segv_handler(int signum_ignored __attribute__((unused)))
{
    siglongjmp(segv_return, 1);
}

static bool
mmap_conflict(void *start, size_t length)
{
    bool retval;
    long page_size;

    retval = false;

    page_size = sysconf(_SC_PAGESIZE);

    if((long)start % page_size != 0)
    {
        retval = true;
        warning_unexpected("start = %p, page_size = %ld, "
                           "start %% page_size = %ld",
                           start, page_size, (long)start % page_size);
    }
    else if(length % page_size != 0)
    {
        retval = true;
        warning_unexpected("length = %ld, page_size = %ld, "
                           "length %% page_size = %ld",
                           (long)length, page_size, (long)length % page_size);
    }
    else
    {
        sig_t old_segv_handler;
        volatile int n_pages;
        volatile int n_failures;
        volatile char *volatile addr;
        char *stop;

        n_pages = 0;
        n_failures = 0;
        stop = (char *)start + length;

        old_segv_handler = signal(SIGSEGV, segv_handler);
        for(addr = (char *)start; addr < stop; addr += page_size)
        {
            ++n_pages;
            if(sigsetjmp(segv_return, 1) != 0)
                ++n_failures;
            else
                *addr;
        }
        signal(SIGSEGV, old_segv_handler);
        retval = n_failures < n_pages;
        if(retval)
            warning_unexpected("%d pages were already mapped",
                               n_pages - n_failures);
    }

    return retval;
}

/*
 * This code used to try to get us memory that we could use directly (meaning
 * it either included page 0 or was adjacent to page 0 and we could use some
 * other trick to get page 0) without having to offset the emulated addresses.
 *
 * I doubt it works on many (any?) platforms anymore, and since speed of
 * emulation isn't an issue, we really shouldn't care.
 */

void *
mmap_permanent_memory(unsigned long amount_wanted)
{
    return NULL;
#if defined(MACOSX_)
    return NULL;
#else
    caddr_t addr_got;
    caddr_t badness_start;

    /* Only do this if our text segment is up nice and high out of the way. */
    if(((unsigned long)mmap_permanent_memory & 0xFF000000L) == 0)
        return NULL;

    {
        extern void *_start;

        badness_start = (caddr_t)((unsigned long)&_start
                                  / (1024 * 1024) * (1024 * 1024));
    }

#if !defined(powerpc) && !defined(__ppc__)
    {
        caddr_t addr_wanted;

        addr_wanted = round_up_to_page_size(PAGE_ZERO_START + PAGE_ZERO_SIZE);

        if(addr_wanted + amount_wanted > badness_start)
        {
            warning_unexpected("addr_wanted = %p, amount_wanted = 0x%lx, "
                               "badness_start = %p",
                               addr_wanted, amount_wanted,
                               badness_start);
            return NULL;
        }

        if(mmap_conflict(addr_wanted, amount_wanted))
            addr_got = NULL;
        else
        {
            addr_got = (caddr_t)mmap(addr_wanted, amount_wanted, PROT_READ | PROT_WRITE,
                                     MAP_ANONYMOUS | MAP_FIXED | MAP_PRIVATE, -1, 0);
            if(addr_got == (caddr_t)-1)
                addr_got = NULL;
            else if(addr_got != addr_wanted)
                warning_unexpected("addr_wanted = %p, addr_got = %p",
                                   addr_wanted, addr_got);
        }
    }
#else

#warning THIS CODE IS PROBABLY WRONG

    /*
   * I haven't tested a powerpc build in a while, but I just noticed that
   * we're trying to mmap from 0 and then we're returning addr_got.  I think
   * that when we return 0, the caller believes that we weren't able to
   * mmap the low globals.  As such, the code below PROBABLY DOESN'T DO
   * ANYTHING DIFFERENT THAN SIMPLY RETURNING NULL.
   */

    if(amount_wanted > badness_start)
    {
        warning_unexpected("amount_wanted = 0x%x, badness_start = %p",
                           amount_wanted, badness_start);
        addr_got = NULL;
    }
    else
        addr_got = mmap(0, amount_wanted, PROT_READ | PROT_WRITE,
                        MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if(addr_got == (caddr_t)-1)
        addr_got = NULL;
#endif

    return addr_got;
#endif
}

#endif /* defined (LINUX) || defined (MACOSX) */
