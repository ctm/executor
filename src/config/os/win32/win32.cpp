#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES
#include <windows.h>
#include <rsys/common.h>
#include "rsys/lockunlock.h"
#include "rsys/os.h"
#include "cleanup.h"

using namespace Executor;

bool Executor::os_init()
{
    atexit(call_cleanup_bat);

    return true;
}

int
Executor::ROMlib_lockunlockrange(int fd, uint32_t begin, uint32_t count, lockunlock_t op)
{
    return 0;
#if 0
    int retval;
    BOOL WINAPI (*routine)(HANDLE, DWORD, DWORD, DWORD, DWORD);

    warning_trace_info("fd = %d, begin = %d, count = %d, op = %d",
                       fd, begin, count, op);
    switch(op)
    {
        case lock:
            routine = LockFile;
            break;
        case unlock:
            routine = UnlockFile;
            break;
        default:
            warning_unexpected("op = %d", op);
            routine = 0;
            break;
    }

    if(!routine)
        retval = paramErr;
    else
    {
        BOOL success;
        HANDLE h;

        h = (HANDLE)_get_osfhandle(fd);
        success = routine(h, begin, 0, count, 0);
        if(success)
            retval = noErr;
        else
        {
            DWORD err;

            err = GetLastError();
            switch(err)
            {
                case ERROR_LOCK_VIOLATION:
                    retval = fLckdErr;
                    break;
                case ERROR_NOT_LOCKED:
                    retval = afpRangeNotLocked;
                    break;
                case ERROR_LOCK_FAILED:
                    retval = afpRangeOverlap;
                    break;
                default:
                    warning_unexpected("err = %ld, h = %p", err, h);
                    retval = noErr;
                    break;
            }
        }
    }
    return retval;
#endif
}

#if 0
int
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
#endif

bool Executor::host_has_spfcommon(void)
{
    return false;
}

bool
Executor::host_spfcommon(host_spf_reply_block *replyp, const char *prompt,
                         const char *incoming_filename, void *fp, void *filef, int numt,
                         void *tl, getorput_t getorput, sf_flavor_t flavor,
                         void *activeList, void *activateproc, void *yourdatap)
{
    return false;
}
