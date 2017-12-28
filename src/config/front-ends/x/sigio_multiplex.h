/* Copyright 1995 by Abacus Research and Development, Inc.
   All rights reserved.
 */

#if !defined(_SIGIO_MULTIPLEX_H_)
#define _SIGIO_MULTIPLEX_H_

/* Xlib is not reentrant, so block incoming signals whenever we call
   Xlib functions */
#define BLOCK_SIGIO_DECL \
    sigset_t _mask;      \
    sigset_t _orig_mask
#define BLOCK_SIGIO_BEGIN     \
    sigemptyset(&_mask);      \
    sigaddset(&_mask, SIGIO); \
    sigprocmask(SIG_BLOCK, &_mask, &_orig_mask);

#define BLOCK_SIGIO_END \
    sigprocmask(SIG_SETMASK, &_orig_mask, NULL);

class BlockSigIOGuard
{
    BLOCK_SIGIO_DECL;

public:
    BlockSigIOGuard()
    {
        BLOCK_SIGIO_BEGIN;
    }
    ~BlockSigIOGuard()
    {
        BLOCK_SIGIO_END;
    }
};

typedef void (*sigio_hdlr_t)(int signo);
void sigio_multiplex_install_handler(int fd, sigio_hdlr_t hdlr);
void sigio_multiplex_remove_handler(int fd, sigio_hdlr_t hdlr);

#endif /* _SIGIO_MULTIPLEX_H_ */
