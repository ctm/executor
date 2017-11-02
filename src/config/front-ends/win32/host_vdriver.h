
/* The main program entry point for Win32 is WinMain(), not main(), so 
   we call main() from within WinMain()...
 */
#ifndef main
#define main ROMlib_main
#endif

/* Choose our graphic subsystem */
#define WIN_DIB 0    /* Use CreateDIBSection() -- compilable with mingwin32 */
#define WIN_MGL 1    /* Use the SciTech MGL Library */
#define WIN_SUBSYSTEM WIN_DIB

/* Win32 is after all, a windowing environment */
#if 0 /* FIXME: Remove this when we support title-bar changes */
#define VDRIVER_DISPLAYED_IN_WINDOW
#endif
#define vdriver_fixed_clut_p false
#define vdriver_grayscale_p false

/* We don't have a function to notify the user that the system is busy */
#define vdriver_system_busy 0


#if WIN_SUBSYSTEM == WIN_DIB
/* We don't provide any accelerated display functions under WIN_DIB */
#define VDRIVER_BYPASS_INTERNAL_FBUF_P() false
#define vdriver_accel_rect_fill(t, l, b, r, c) VDRIVER_ACCEL_NO_UPDATE
#define vdriver_accel_rect_scroll(t, l, b, r, dx, dy) VDRIVER_ACCEL_NO_UPDATE
#define vdriver_accel_wait()

/* The modes we support under the DIB driver */
typedef VDRIVER_MODE_LIST_TYPE(2) vdriver_dib_mode_t;
extern vdriver_dib_mode_t vdriver_dib_modes;
#define vdriver_mode_list ((const vdriver_modes_t *)&vdriver_dib_modes)

#else
#error No windows graphics subsystem defined!
#endif /* WIN_DIB */







