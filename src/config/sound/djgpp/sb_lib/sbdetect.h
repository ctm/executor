#ifndef __SB_DETECT
#define __SB_DETECT
#ifdef __cplusplus
extern "C"
{
#endif

#include "sb_defs.h"

struct _sb_info
  {
    WORD reset;
    WORD readData;
    WORD writeData;
    WORD dataAvail;
    WORD dataAvail16;
    int IRQ;
    int DMA;
#ifdef SB_SUPPORT_16_BIT
    int DMA16;
#endif
    int dspVersion;
  };

extern struct _sb_info sb_info;
extern int sb_disable16bit;

/* Pass:                                                                      */
/* Returns: sb_status enum indicating anything that might have gone wrong.    */
  sb_status sb_is_present (void);

/*
   The following two functions set the 8- and 16-bit DMA channels to non-
   default settings (defaults are 1 and 5, respectively). This must be done
   only if the user's sound card is set to settings other than these AND the
   user has NOT set the BLASTER environment variable to tell us what his
   card's settings are.
 */
  void sb_change_dma8_channel (int);
  void sb_change_dma16_channel (int);

#ifdef __cplusplus
}
#endif
#endif
