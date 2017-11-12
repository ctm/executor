#ifndef __SB_DEF
#define __SB_DEF
#include <inlines/pc.h>
#ifdef __cplusplus
extern "C" {
#endif

#ifndef BYTE
#define BYTE unsigned char
#endif
#ifndef WORD
#define WORD unsigned short
#endif
#ifndef DWORD
#define DWORD unsigned long
#endif

/* enum for return-values of many of the sb_ functions. */
typedef enum {
    SB_SUCCESS,
    SB_FAILURE,
    SB_BAD_BLASTER,
    SB_BAD_ADDRESS,
    SB_BAD_IRQ,
    SB_BAD_DMA,
    SB_BAD_DMA16,
    SB_BAD_FILE,
    SB_BUSY,
    SB_BAD_POINTER
} sb_status;

/* #define SB_SUPPORT_16_BIT */

#define SB_8_BIT 0x01
#ifdef SB_SUPPORT_16_BIT
#define SB_16_BIT 0x02
#endif
#define SB_MONO 0x04
#define SB_STEREO 0x08

#define SB_BUFFER_SIZE 2048
#define SB_MAX_8_BIT_MONO_SAMPLES (SB_BUFFER_SIZE)
#define SB_MAX_8_BIT_STEREO_SAMPLES (SB_BUFFER_SIZE / 2)
#ifdef SB_SUPPORT_16_BIT
#define SB_MAX_16_BIT_MONO_SAMPLES (SB_BUFFER_SIZE / 2)
#define SB_MAX_16_BIT_STEREO_SAMPLES (SB_BUFFER_SIZE / 4)
#endif

#ifdef __cplusplus
}
#endif
#endif
