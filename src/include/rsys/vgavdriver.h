#if !defined (_VGADRIVER_H_)
#define _VGADRIVER_H_

#include "rsys/vdriver.h"

namespace Executor {
typedef struct
{
  int width;
  int height;
  unsigned row_bytes;
  int mode_number;
  unsigned char log2_bpp;
  unsigned char planar_p;
  unsigned char interlaced_p;
  unsigned char multi_window_p;
  unsigned char win_read;
  unsigned char win_write;
#if defined (VGAHOST_VGA_MODE_EXTENSIONS)
  VGAHOST_VGA_MODE_EXTENSIONS
#endif
} vga_mode_t;


/* Relevant ports. */
#define VGA_SEQUENCER_PORT		0x3C4
#define VGA_SEQUENCER_DATA		0x3C5
#define VGA_GRAPHICS_CONTROLLER_PORT	0x3CE
#define VGA_GRAPHICS_CONTROLLER_DATA	0x3CF
#define VGA_CRTC_ADDR_PORT		0x3D4
#define VGA_CRTC_DATA_PORT		0x3D5


/* Sequencer registers. */
#define VGA_MAP_MASK_REG 0x2

/* Graphics controller registers. */
#define VGA_SR_VALUE_REG    0x0  /* set/reset */
#define VGA_SR_MASK_REG     0x1  /* enable set/reset */
#define VGA_DATA_ROTATE_REG 0x3  /* also has func in it */
#define VGA_BIT_MASK_REG    0x8

/* CRT controller registers. */
#define VGA_START_ADDR_HIGH_REG 0xC
#define VGA_START_ADDR_LOW_REG  0xD


#define VGA_WRITE_REG(port, reg, val)				\
do {									\
  if (__builtin_constant_p ((((val) & 0xFF) << 8) | (reg)))		\
    asm volatile ("outw %%ax,%%dx"					\
		  : : "a" ((((val) & 0xFF) << 8) | (reg)), "d" (port));	\
  else									\
    asm volatile ("movb %b1,%%ah\n\t"					\
		  "outw %%ax,%%dx"					\
		  : : "a" (reg), "bc" (val), "d" (port)			\
		  : "ax");						\
} while (0)

#define VGA_SET_WRITE_PLANE_MASK(mask) \
   VGA_WRITE_REG (VGA_SEQUENCER_PORT, VGA_MAP_MASK_REG, mask)
#define VGA_SET_WRITE_PLANE(plane) \
   VGA_SET_WRITE_PLANE_MASK (1 << (plane))

#define VGA_SET_BIT_MASK(mask) \
   VGA_WRITE_REG (VGA_GRAPHICS_CONTROLLER_PORT, VGA_BIT_MASK_REG, mask)

#define VGA_SET_SR_VALUE(value) \
   VGA_WRITE_REG (VGA_GRAPHICS_CONTROLLER_PORT, VGA_SR_VALUE_REG, value)
#define VGA_SET_SR_MASK(mask) \
   VGA_WRITE_REG (VGA_GRAPHICS_CONTROLLER_PORT, VGA_SR_MASK_REG, mask)

#define VGA_SET_DATA_ROTATE(value) \
   VGA_WRITE_REG (VGA_GRAPHICS_CONTROLLER_PORT, VGA_DATA_ROTATE_REG, value)



extern const vga_mode_t *vga_current_mode;
extern int vga_write_window, vga_read_window;
extern int vga_first_byte_offset;
extern uint8 *vga_portal_baseaddr;

extern void vga_update_cursor (int hot_x, int hot_y);


/* Functions which must be provided by the host using this code. */
extern boolean_t vgahost_init (int max_width, int max_height, int max_bpp,
			       boolean_t fixed_p, int *argc, char *argv[]);
extern void vgahost_shutdown (void);
extern boolean_t vgahost_set_mode (vga_mode_t *mode);
extern void vgahost_set_colors (int first_color, int num_colors,
				const ColorSpec *color_array);
extern vga_mode_t *vgahost_compute_vga_mode_list (void);

#if !defined (vgahost_alloc_fbuf)
extern void vgahost_alloc_fbuf (unsigned long size);
#endif

#if !defined (vgahost_mmap_linear_fbuf)
extern boolean_t vgahost_mmap_linear_fbuf (const vga_mode_t *mode);
#endif

#if !defined (vgahost_illegal_mode_p)
extern boolean_t vgahost_illegal_mode_p (int width, int height, int bpp,
					 boolean_t exact_match_p);
#endif

#if !defined (vgahost_unmap_linear_fbuf)
extern boolean_t vgahost_unmap_linear_fbuf (unsigned long num_bytes);
#endif

extern void vgahost_set_rw_windows (int win);
extern void vgahost_set_read_window (int win);
extern void vgahost_set_write_window (int win);
}

#endif /* !_VGADRIVER_H_ */
