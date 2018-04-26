#if !defined(_LAUNCH_H_)
#define _LAUNCH_H_

/*
 * Copyright 1999 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */
namespace Executor
{
extern uint32_t ROMlib_version_long;
extern void ROMlib_set_ppc(bool val);
extern int ROMlib_uaf;

typedef enum {
    launch_no_failure,
    launch_cfm_requiring,
    launch_ppc_only,
    launch_damaged,
    launch_compressed_ge7,
    launch_compressed_lt7,
} launch_failure_t;

struct vers_t
{
    GUEST_STRUCT;
    GUEST<unsigned char[4]> c;
    GUEST<int16_t> loc;
    GUEST<unsigned char[1]> shortname;
};

extern launch_failure_t ROMlib_launch_failure;
extern INTEGER ROMlib_exevrefnum;
}

#endif
