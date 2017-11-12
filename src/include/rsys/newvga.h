#if !defined(_VGA_H_)
#define _VGA_H_

#include "QuickDraw.h"
namespace Executor
{
#define NUM_STANDARD_VGA_MODES 2
#define NUM_VESA_MODES 8
#define EXTRA_VGA_MODES 16 /* Arbitrary, but allow for extra modes. */
#define MAX_VGA_MODES \
    (NUM_STANDARD_VGA_MODES + (NUM_VESA_MODES * 2) + EXTRA_VGA_MODES)

typedef struct
{
    int width;
    int height;
    unsigned char log2_bpp;
    unsigned char planar_p;
    unsigned char win_read;
    unsigned char win_write;
    unsigned short row_bytes;
    unsigned short bios_mode;
    unsigned long win_granularity; /* Size in BYTES */
    unsigned long win_size; /* Size in BYTES */
    unsigned short winfunc_offset, winfunc_segment;
} vga_mode_t;

#define VESA_MODE_P(mode) (((mode)&0x100) != 0)

#define VESA_GET_VGA_INFO 0x4F00
#define VESA_GET_MODE_INFO 0x4F01
#define VESA_SET_MODE 0x4F02
#define VESA_WINDOW_CONTROL 0x4F05
#define VESA_SUCCESS 0x004F

#define MEMORY_MODEL_FOUR_PLANES 0x3
#define MEMORY_MODEL_PACKED_PIXELS 0x4

#define VGA_STATE_TO_SAVE 0x7 /* All state. */

typedef struct
{
    char signature[4];
    unsigned short version;
    unsigned short oem_string_offset, oem_string_segment;
    char capabilities[4];
    unsigned short mode_list_offset, mode_list_segment;
} vesa_info_t;

typedef struct
{
    unsigned short mode_attributes;
    unsigned char win_a_attributes;
    unsigned char win_b_attributes;
    unsigned short win_granularity;
    unsigned short win_size;
    unsigned short win_a_segment;
    unsigned short win_b_segment;
    unsigned short winfunc_offset, winfunc_segment;
    unsigned short row_bytes;

    /* Optional information, valid iff (mode_attributes & 0x2). */
    unsigned short width;
    unsigned short height;
    unsigned char char_width;
    unsigned char char_height;
    unsigned char num_planes;
    unsigned char bits_per_pixel;
    unsigned char num_banks;
    unsigned char memory_model;
    unsigned char bank_size;
} mode_info_t;

#define ATTR_LEGAL_MODE 0x1
#define ATTR_EXTENDED_INFO 0x2
#define ATTR_GRAPHICS_MODE 0x10

typedef struct
{
    unsigned char and_mask, xor_mask;
} vga_cursor_xfer_t;

typedef struct
{
    int elt_size;
    int row_bytes;
    union {
        vga_cursor_xfer_t *packed;
        unsigned char *raw_bits;
    } cursor_bits;
} vga_cursor_mask_desc_t;

extern int vdriver_init(int max_width, int max_height, int max_bpp);
extern void vdriver_shutdown(void);
extern void vdriver_update_screen(int top, int left, int bottom, int right);
extern int vdriver_info(int *max_bpp, Point *sizes);
extern void *vdriver_frame_buffer(int *width, int *height, int *row_bytes,
                                  int *bpp);
extern void vdriver_set_colors(int first_color, int num_colors,
                               const RGBColor *color_array);
extern int vdriver_set_mode(int width, int height, int bpp);
}
#endif /* !_VGA_H_ */
