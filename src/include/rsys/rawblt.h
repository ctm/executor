#if !defined(_RSYS_RAWBLT_H_)
#define _RSYS_RAWBLT_H_

typedef struct
{
    const void *label;
    int32_t offset;
    int32_t arg;
} blt_section_t;

#define MAX_BLT_SECTIONS 1024

#endif /* !_RSYS_RAWBLT_H_ */
