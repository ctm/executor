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
#include "rsys/lowglobals.h"

#include "Gestalt.h"
#include "SegmentLdr.h"

#include "rsys/gestalt.h"

#include "rsys/lockunlock.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

using namespace Executor;

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
Executor::ROMlib_lockunlockrange(int fd, uint32_t begin, uint32_t count, lockunlock_t op)
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

bool Executor::host_has_spfcommon(void)
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

#endif /* defined (LINUX) || defined (MACOSX) */
