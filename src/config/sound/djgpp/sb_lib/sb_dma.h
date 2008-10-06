#ifndef __SB_HW
#define __SB_HW
#ifdef __cplusplus
extern "C"
{
#endif

/* Pass: Linear Address of DOS buffer; Size of buffer (in BYTES) minus 1      */
/* Returns:                                                                   */
  void sb_dma8bitReadSC (unsigned long, int);
  void sb_dma16bitReadSC (unsigned long, unsigned int);

  int sb_dma_module_lock_your_memory (void);

#ifdef __cplusplus
}
#endif
#endif
