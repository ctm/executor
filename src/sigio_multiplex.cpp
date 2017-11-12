/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_sigio_multiplex[] = "$Id: sigio_multiplex.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"

#if defined(LINUX) || defined(MACOSX_) || defined(MACOSX_)

#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#include "rsys/sigio_multiplex.h"
using namespace Executor;

/* a simple sigio multiplexor */

typedef struct sigio_hdlr
{
    void (*hdlr)(int signo);
    int fd;
} sigio_hdlr_record_t;

int n_sigio_hdlrs;
sigio_hdlr_record_t *sigio_hdlrs;

static void
sigio_multiplex_hdlr(int signo)
{
    int n_fds_available;
    int i;
    int max_fd;
    fd_set read_set;
    struct timeval nowait;

    max_fd = 0;
    FD_ZERO(&read_set);
    for(i = 0; i < n_sigio_hdlrs; i++)
    {
        int fd;

        fd = sigio_hdlrs[i].fd;
        max_fd = MAX(max_fd, fd);
        FD_SET(fd, &read_set);
    }

    nowait.tv_sec = nowait.tv_usec = 0;
    n_fds_available = select(max_fd + 1, &read_set, NULL, NULL, &nowait);
    if(n_fds_available < 0)
    {
        warning_unexpected("select returned negative value, panic!");
        return;
    }

    for(i = 0; i < n_sigio_hdlrs; i++)
    {
        int fd;

        fd = sigio_hdlrs[i].fd;
        if(FD_ISSET(fd, &read_set))
            (*sigio_hdlrs[i].hdlr)(signo);
    }
}

void sigio_multiplex_install_handler(int fd, sigio_hdlr_t hdlr)
{
    sigio_hdlrs = (sigio_hdlr_record_t *)realloc(sigio_hdlrs, (sizeof *sigio_hdlrs
                                                               * (n_sigio_hdlrs + 1)));
    sigio_hdlrs[n_sigio_hdlrs].fd = fd;
    sigio_hdlrs[n_sigio_hdlrs].hdlr = hdlr;

    n_sigio_hdlrs++;

    /* make sure we are handling sigio */
    {
#if !defined(USE_BSD_SIGNALS)
        struct sigaction sa;

        sa.sa_handler = sigio_multiplex_hdlr;
        sigemptyset(&sa.sa_mask);
        sigaddset(&sa.sa_mask, SIGIO);
        sa.sa_flags = 0;

        sigaction(SIGIO, &sa, NULL);
#else
        struct sigvec sv;

        sv.sv_handler = sigio_multiplex_hdlr;
        sv.sv_mask = sigmask(SIGIO);
        sv.sv_flags = 0;

        sigvec(SIGIO, &sv, NULL);
#endif
    }
}

void sigio_multiplex_remove_handler(int fd, sigio_hdlr_t hdlr)
{
    int i;

    for(i = 0; i < n_sigio_hdlrs; i++)
    {
        if(sigio_hdlrs[n_sigio_hdlrs].fd == fd
           && sigio_hdlrs[n_sigio_hdlrs].hdlr == hdlr)
        {
            /* found */
            memmove(&sigio_hdlrs[i], &sigio_hdlrs[i + 1],
                    (n_sigio_hdlrs - i - 1) * sizeof *sigio_hdlrs);
            n_sigio_hdlrs--;
            return;
        }
    }

    warning_unexpected("fd, hdlr pair not found");
}

#endif /* LINUX || MACOSX_ */
