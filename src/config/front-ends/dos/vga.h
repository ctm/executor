#if !defined(_VGA_H_)
#define _VGA_H_

#include "rsys/vgavdriver.h"

#define NUM_STANDARD_VGA_MODES 1
#define NUM_VESA_MODES 8
#define EXTRA_VGA_MODES 32 /* Arbitrary, but allow for extra modes. */
#define MAX_VGA_MODES \
    (NUM_STANDARD_VGA_MODES + (NUM_VESA_MODES * 2) + EXTRA_VGA_MODES)

#define VESA_MODE_P(mode) (((mode)&0x100) != 0)

#define VESA_GET_VGA_INFO 0x4F00
#define VESA_GET_MODE_INFO 0x4F01
#define VESA_SET_MODE 0x4F02
#define VESA_GET_MODE_NUMBER 0x4F03
#define VESA_STATE_CONTROL 0x4F04
#define VESA_WINDOW_CONTROL 0x4F05
#define VESA_SET_GET_PALETTE_FORMAT 0x4F08
#define VESA_PALETTE_MANIP 0x4F09
#define VESA_GET_PMI 0x4F0A
#define VESA_SUCCESS 0x004F

#define MEMORY_MODEL_FOUR_PLANES 0x3
#define MEMORY_MODEL_PACKED_PIXELS 0x4
#define MEMORY_MODEL_RGB 0x6
#define MEMORY_MODEL_YUV 0x7

#define VESA_STATE_TO_SAVE 0xF /* All state. */
#define VGA_STATE_TO_SAVE 0x7 /* All state. */

typedef struct
{
    uint8 signature[4];
    uint16 version;
    uint16 oem_string_offset, oem_string_segment;
    uint8 capabilities[4];
    uint16 mode_list_offset, mode_list_segment;

    /* VBE 2.0 fields. */
    uint16 total_memory;
    uint16 oem_software_rev;
    uint16 oem_vendor_name_offset, oem_vendor_name_segment;
    uint16 oem_product_name_offset, oem_product_name_segment;
    uint16 oem_product_rev_offset, oem_product_rev_segment;
    uint8 reserved[222];
    uint8 oemdata[256];
} vesa_info_t;

typedef struct
{
    uint16 mode_attributes;
    uint8 win_a_attributes;
    uint8 win_b_attributes;
    uint16 win_granularity;
    uint16 win_size;
    uint16 win_a_segment;
    uint16 win_b_segment;
    uint16 winfunc_offset, winfunc_segment;
    uint16 row_bytes;

    /* Optional information, valid iff (mode_attributes & 0x2). */
    uint16 width;
    uint16 height;
    uint8 char_width;
    uint8 char_height;
    uint8 num_planes;
    uint8 bits_per_pixel;
    uint8 num_banks;
    uint8 memory_model;
    uint8 bank_size;
    uint8 num_image_pages;
    uint8 reserved_byte;

    /* Direct color fields (only valid for RGB or YUV). */
    uint8 red_mask_size;
    uint8 red_field_position;
    uint8 green_mask_size;
    uint8 green_field_position;
    uint8 blue_mask_size;
    uint8 blue_field_position;
    uint8 reserved_mask_size;
    uint8 reserved_field_position;
    uint8 direct_color_mode_info;

    /* VBE 2.0 info. */
    uint32 phys_base_addr;
    int32 off_screen_mem_offset;
    uint16 off_screen_mem_size; /* in 1K units */

    uint8 reserved[206];
} mode_info_t;

#define SWITCHABLE_DAC (1 << 0)
#define SET_PALETTE_DURING_VBL (1 << 2)

#define ATTR_LEGAL_MODE (1 << 0)
#define ATTR_EXTENDED_INFO (1 << 1)
#define ATTR_GRAPHICS_MODE (1 << 4)
#define ATTR_NOT_VGA_COMPATIBLE (1 << 5)
#define ATTR_HAS_WINDOWED_MODE (1 << 6)
#define ATTR_LINEAR_FBUF (1 << 7)

#define USE_LINEAR_FBUF (1 << 14)

extern uint16 vesa_version;
extern bool only_use_vga_p;

enum
{
    DONT_CLEAR_DISPLAY = (1 << 15)
};

#endif /* !_VGA_H_ */
