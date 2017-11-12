#if !defined(_CHECKPOINT_H_)
#define _CHECKPOINT_H_

/*
 * Copyright 1997 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: checkpoint.h 63 2004-12-24 18:19:43Z ctm $
 */

#define CHECKPOINT_FILE "failure.txt"
namespace Executor
{
typedef struct
{
    bool sound_fails;
    bool aspi_fails;
    uint32 bad_macdrives;
    uint32 bad_dosdrives;
} checkpoint_t;

typedef enum {
    begin,
    end,
} checkpoint_option;

extern checkpoint_t *checkpointp;

extern checkpoint_t *checkpoint_init(void);
extern void checkpoint_sound(checkpoint_t *cp, checkpoint_option option);
extern void checkpoint_aspi(checkpoint_t *cp, checkpoint_option option);
extern void checkpoint_macdrive(checkpoint_t *cp, checkpoint_option option,
                                uint32 drive);
extern void checkpoint_dosdrives(checkpoint_t *cp, checkpoint_option option,
                                 uint32 drive);
extern void disable_checkpointing(void);
}
#endif
