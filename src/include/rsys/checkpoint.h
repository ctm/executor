#if !defined(_CHECKPOINT_H_)
#define _CHECKPOINT_H_

/*
 * Copyright 1997 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#define CHECKPOINT_FILE "failure.txt"
namespace Executor
{
typedef struct
{
    bool sound_fails;
    bool aspi_fails;
    uint32_t bad_macdrives;
    uint32_t bad_dosdrives;
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
                                uint32_t drive);
extern void checkpoint_dosdrives(checkpoint_t *cp, checkpoint_option option,
                                 uint32_t drive);
extern void disable_checkpointing(void);
}
#endif
