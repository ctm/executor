/* Copyright 1994 - 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_vga[] = "$Id: NEWvga.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "vga.h"
#include "vgatables.h"
#include <dpmi.h>
#include <stubinfo.h>
#include "rsys/fatal.h"
#include "rsys/cquick.h"
#include "rsys/blockinterrupts.h"
#include "rsys/host.h"
#include "rsys/depthconv.h"
#include "rsys/flags.h"
#include "rsys/refresh.h"
#include "MemoryMgr.h"
#include "rsys/mman.h"
#include "SegmentLdr.h"
#include "rsys/segment.h"
#include "rsys/time.h"
#include "dosmem.h"
#include "dpmimem.h"
#include "dpmicall.h"
#include <sys/nearptr.h>

typedef uint32 vga_svga_compound_mode_t;
enum
{
    VBE_MODE_BIT = 1 << 31
}; /* or'd in when we use VESA to get mode */

/* List of known graphics modes.  Terminated with a width field of 0. */
static vga_mode_t mode_list[MAX_VGA_MODES + 1];

/* Original VGA mode. */
static vga_svga_compound_mode_t orig_vga_mode = -1;

/* Do we have a VESA driver? */
uint16 vesa_version;

/* Selector for real mode 0xA0000 - 0xAFFFF */
uint16 vga_window_selector;

/* Selector for the screen, wherever it is.  May be the same as
 * `vga_window_selector', or may be different for a linear frame buffer, etc.
 */
uint16 vga_screen_selector;

/* true iff we should pretend there's no VBE video driver. */
bool only_use_vga_p;

/* true iff we should try to make %ds enclose the real linear frame
 * buffer and thereby give programs direct access to the frame buffer.
 * Having such a "fat %ds" is dangerous, but can make graphics much
 * faster, esp. for programs that require "refresh" mode.
 */
bool try_to_use_fat_ds_vga_hack_p;

/* true iff we're actually using the fat %ds vga hack. */
static bool actually_using_fat_ds_vga_hack_p;

/* Saved vga state and its size, so we can clean up when we exit. */
static void *vga_state = NULL;
static unsigned vga_state_size;

#if 0
/* VBE 2.0 protected mode interface code pointers.  NULL if not available. */
static void *set_window_call_codep;
static void *set_display_start_codep;
static void *set_palette_data_codep;
#endif

/* 0x80 if we should wait until VBL to set the DAC. */
static uint8 set_palette_during_vbl_mask;

/* Bits per DAC element; usually 6, never more than 8. */
static int bits_per_dac_element = 6;

/* true iff the DAC can be switched to 8bpp. */
static bool switchable_dac_p;

/* Makes a call to the VESA driver, and returns true iff successful. */
static bool
vesa_call(int eax, __dpmi_regs *r)
{
    bool success_p;

    if(only_use_vga_p) /* only old-style VGA allowed? */
        success_p = false;
    else
    {
        r->d.eax = eax;
        success_p = (__dpmi_int(0x10, r) == 0 && r->x.ax == VESA_SUCCESS);
    }

    return success_p;
}

/* The following code may be a bad idea.  The VBE spec that we have
   says that VESA_GET_MODE_NUMBER may not return the correct value
   if the mode number hadn't already been set with VESA_SET_MODE.
   Still, people have been reporting mystery trouble with Executor,
   so this hack is worth trying.  */

static vga_svga_compound_mode_t
getmode(void)
{
    vga_svga_compound_mode_t retval;

    if(vesa_version >= 0x100 && !only_use_vga_p)
    {
        __dpmi_regs regs;

        dpmi_zero_regs(&regs);
        if(vesa_call(VESA_GET_MODE_NUMBER, &regs))
        {
            retval = regs.x.bx & ~DONT_CLEAR_DISPLAY;
            retval |= VBE_MODE_BIT;
        }
        else
        {
            only_use_vga_p = true;
            retval = getmode();
            only_use_vga_p = false;
        }
    }
    else
    {
        __dpmi_regs regs;

        dpmi_zero_regs(&regs);
        regs.h.ah = 0xF;
        if(__dpmi_int(0x10, &regs) != -1)
            retval = regs.h.al & 0x7F;
        else
            retval = -1;
    }

    return retval;
}

static bool
setmode(vga_svga_compound_mode_t mode)
{
    bool retval;

    if(mode & VBE_MODE_BIT)
    {
        __dpmi_regs regs;

        dpmi_zero_regs(&regs);
        regs.x.bx = (mode & ~VBE_MODE_BIT) | DONT_CLEAR_DISPLAY; /* some S3 based
							     cards will hang
							     the system if
							     you try to clear
							     the display here
							     no kidding */
        retval = vesa_call(VESA_SET_MODE, &regs);
    }
    else
    {
        __dpmi_regs regs;

        dpmi_zero_regs(&regs);
#if 0
      regs.x.ax = mode | 0x80;   /* Don't clear memory */
#else
        regs.x.ax = mode & 0x7F; /* clear memory */
#endif
        retval = __dpmi_int(0x10, &regs) != -1;
    }
    return retval;
}

typedef enum {
    COMPUTE_BUFFER_SIZE = 0,
    SAVE_STATE = 1,
    RESTORE_STATE = 2
} state_func_t;

static bool (*state_glue_funcp)(state_func_t, unsigned *);

/* #define USE_VESA_STATE_SAVE */

#if defined(USE_VESA_STATE_SAVE)

/* NOTE: users started reported serious problems once I added this
 * routine.  I'm leaving it out by default for now.
 */

/* This function saves video state, restores video state, or computes
 * the size of a buffer needed to hold the buffer size depending on
 * the OPERATION.  If non-NULL, and the operation is
 * COMPUTE_BUFFER_SIZE, *buffer_size_ret is filled in with the
 * buffer size.  This routine assumes the state buffer is at
 * `dos_buf_segment:0'.  Returns true on success, false on failure.
 */
static bool
vesa_state_glue(state_func_t operation, unsigned *buffer_size_ret)
{
    __dpmi_regs regs;
    bool success_p;
    unsigned buf_size;

    /* Call the video driver. */
    dpmi_zero_regs(&regs);
    regs.h.dl = operation;
    regs.x.cx = VESA_STATE_TO_SAVE; /* all state */
    regs.x.es = dos_buf_segment;
    regs.x.bx = 0;
    if(vesa_call(VESA_STATE_CONTROL, &regs))
    {
        buf_size = regs.x.bx * 64; /* only valid for COMPUTE_BUFFER_SIZE */
        success_p = true;
    }
    else
    {
        buf_size = 0;
        success_p = false;
    }

    if(buffer_size_ret != NULL)
        *buffer_size_ret = buf_size;

    if(!success_p)
        warning_unexpected("vesa_state_glue call failed!");

    return success_p;
}
#endif /* defined (USE_VESA_STATE_SAVE) */

/* Same as vesa_state_glue, but only uses standard BIOS calls. */
static bool
vga_state_glue(state_func_t operation, unsigned *buffer_size_ret)
{
    __dpmi_regs regs;
    bool success_p;
    unsigned buf_size;

    /* Call the video driver. */
    dpmi_zero_regs(&regs);
    regs.h.ah = 0x1C;
    regs.h.al = operation;
    regs.x.cx = VGA_STATE_TO_SAVE; /* all state */
    regs.x.es = dos_buf_segment;
    regs.x.bx = 0;
    if(__dpmi_int(0x10, &regs) == 0 && regs.h.al == 0x1C)
    {
        buf_size = regs.x.bx * 64; /* only valid for COMPUTE_BUFFER_SIZE */
        success_p = true;
    }
    else
    {
        buf_size = 0;
        success_p = false;
    }

    if(buffer_size_ret != NULL)
        *buffer_size_ret = buf_size;
    return success_p;
}

/* Restores the VGA state specified in vga_state. */
static void
restore_vga_state(void)
{
    /* Set the mode. */
    if(orig_vga_mode != -1)
    {
        vga_svga_compound_mode_t current_vga_mode;

        current_vga_mode = getmode();

        /* Only reset the mode if it has changed, so we don't clear the
       * screen on exception.
       */
        if(current_vga_mode != orig_vga_mode)
            setmode(orig_vga_mode);
    }

    /* Restore the actual state. */
    if(vga_state != NULL)
    {
        /* Restore the random VGA state. */
        movedata(dos_pm_ds, (unsigned)vga_state, dos_buf_selector, 0,
                 vga_state_size);
        state_glue_funcp(RESTORE_STATE, NULL);
    }
}

/* Saves the vga state into vga_state, allocating space as necessary. */
static void
save_vga_state(void)
{
    /* Fetch the current mode number. */
    orig_vga_mode = getmode();

#if 0
  if (state_glue_funcp (COMPUTE_BUFFER_SIZE, &vga_state_size)
      && vga_state_size <= DOS_BUF_SIZE - DOS_MIN_STACK_SPACE
      && state_glue_funcp (SAVE_STATE, NULL))
    {
      if (vga_state != NULL)
	{
	  warning_unexpected ("vga_state = 0x%lx", (long) vga_state);
	}
      vga_state = malloc (vga_state_size);
      if (vga_state != NULL)
	{
	  /* Save the state away in our memory space. */
	  movedata (dos_buf_selector, 0, dos_pm_ds, (unsigned) vga_state,
		    vga_state_size);
	}
    }
#endif

/* Restore the VGA state because many BIOSes corrupt it when they
   * save it (from the INTER document).  This shouldn't be necessary
   * for VESA drivers, but it's conceivable some stupid VESA driver
   * calls one of the buggy BIOS routines which is known to need this.
   */
#if 0
  {
    int save_orig_mode;

    save_orig_mode = orig_vga_mode;
    orig_vga_mode = -1;	/* Don't reset mode! */
    restore_vga_state ();
    orig_vga_mode = save_orig_mode;
  }
#else
    if(vga_state != NULL)
    {
        /* don't have to move the data back into dos_buf, since nothing
	 has changed there */
        state_glue_funcp(RESTORE_STATE, NULL);
    }
#endif
}

/* Attempts to fetch the VESA information block.  Returns true
 * iff successful, else false (in which case it should be assumed
 * that no VESA driver is present).
 */
static bool
get_vesa_info_block(vesa_info_t *vesa_info)
{
    __dpmi_regs regs;
    bool success_p;

    /* Move "VBE2" into the "VESA" signature field, to request 2.0 driver. */
    movedata(dos_pm_ds, (unsigned)"VBE2", dos_buf_selector, 0, 4);

    /* Make the VESA query. */
    dpmi_zero_regs(&regs);
    regs.x.es = dos_buf_segment;
    regs.x.di = 0;

    if(!vesa_call(VESA_GET_VGA_INFO, &regs))
        success_p = false;
    else
    {
        movedata(dos_buf_selector, 0, dos_pm_ds, (unsigned)vesa_info,
                 sizeof *vesa_info);
        success_p = !strncmp(vesa_info->signature, "VESA", 4);
    }

    return success_p;
}

static unsigned
compute_vesa_version(void)
{
    vesa_info_t vesa_info;
    return get_vesa_info_block(&vesa_info) ? vesa_info.version : 0;
}

#if 0
static int
call_vbe2_pmi (int ax, int bx, int cx, int dx)
{
}


/* Sets up the VBE protected mode interface pointers.  See VBE function
 * 0x0A for more information.  Returns true iff successful.
 */
static bool
set_up_vbe2_protected_mode_interface (void)
{
  bool success_p;

  success_p = false;  /* default */
  set_window_call_codep = NULL;
  set_display_start_codep = NULL;
  set_palette_data_code_p = NULL;

  if (vesa_version >= 0x200)  /* new with VBE 2.0 */
    {
      __dpmi_regs regs;
      dpmi_zero_regs (&regs);
      regs.x.bl = 0;	/* get protected mode table. */
      if (vesa_call (VESA_GET_PMI, &regs))
	{
	  int pmi_size;

	  pmi_size = regs.x.cx;
	  if (vesa_pmi != NULL)
	    free (vesa_pmi);
	  vesa_pmi = malloc (pmi_size);
	  if (vesa_pmi != NULL)
	    {
	      movedata (dos_buf_selector, (regs.x.es << 4) + regs.x.di,
			dos_pm_ds, (unsigned) vesa_pmi,
			pmi_size);
	      set_window_codep = ((char *) vesa_pmi
				       + vesa_pmi->set_window_offset);
	      set_display_start_codep = ((char *) vesa_pmi
					 + vesa_pmi->set_display_start_offset);
	      set_palette_data_code_p = ((char *) vesa_pmi
					 + vesa_pmi->set_palette_data_offset);
	      success_p = true;

	      if (vesa_pmi->io_priv_offset && mmap_io_sel == 0)
		{
		  const uint16 *p;

		  for (p = (const uint16 *) ((char *)vesa_pmi
					     + vesa_pmi->io_priv_offset);
		       *p != 0xFFFF;
		       p++)
		    ;
		  if (p[1] != 0xFFFF)
		    {
		      uint32 addr = *(const uint32 *)(p + 1);
		      uint16 length = p[3];
		      int sel;

		      if (addr < 1024 * 1024 && length <= 65536)
			
		      sel = selector_for_phys_mem (addr, length);
		      if (sel == -1)
			{
			  mmap_io_sel = 0;
			  set_window_codep = NULL;
			  success_p = false;
			}
		      else
			{
			  mmap_io_sel = sel;
			}
		    }
		}
	    }
	}
    }

  return success_p;
}
#endif

bool vgahost_init(int max_width, int max_height, int max_bpp, bool fixed_p,
                  int *argc, char *argv[])
{
    int sel;

    /* Grab a 64K selector for 0xA000:0000, for the frame buffer. */
    sel = __dpmi_segment_to_descriptor(0xA000);
    if(sel == -1)
        return false;
    vga_window_selector = sel;
    vga_screen_selector = -1; /* no value yet. */

    vesa_version = compute_vesa_version();

#if !defined(USE_VESA_STATE_SAVE)
    state_glue_funcp = vga_state_glue;
#else /* USE_VESA_STATE_SAVE */
    if(vesa_version >= 0x100 && !only_use_vga_p)
        state_glue_funcp = vesa_state_glue;
    else
        state_glue_funcp = vga_state_glue;
#endif /* USE_VESA_STATE_SAVE */

    /* Save the current VGA state, so we can clean up well when we exit. */
    save_vga_state();

    /* WinNT appears to not show us a VESA driver unless we start in
   * full screen mode.  However, if we set a plain VGA graphics mode
   * it switches us to full screen mode.  If we didn't see a VESA
   * driver, this attempts to switch to fullscreen mode and then try
   * again.
   */
    if(vesa_version == 0 && !only_use_vga_p)
    {
        if(setmode(0x12)) /* 640x480 VGA mode.  */
        {
            vesa_version = compute_vesa_version();
            restore_vga_state();
        }
    }

    return true;
}

void vdriver_opt_register(void)
{
}

void vgahost_alloc_fbuf(unsigned long size)
{
    unsigned long p;

/* Allocate page-aligned memory for the screen.  We page-align
   * it so we can do various mmap-type things.
   */
#if defined(SBRK_PERMANENT_MEMORY)
    /* sbrk an integral number of pages.   We add two page size's here
   * because we need to round up the amount we allocate to a page
   * multiple and we'll also end up rounding up the base to the next
   * page size.
   */
    p = (unsigned long)sbrk((size + DPMI_PAGE_SIZE + DPMI_PAGE_SIZE - 1)
                            & ~(DPMI_PAGE_SIZE - 1));
#else
    p = (unsigned long)malloc(size + DPMI_PAGE_SIZE - 1);
#endif
    vdriver_fbuf = (uint8 *)((p + DPMI_PAGE_SIZE - 1) & ~(DPMI_PAGE_SIZE - 1));
}

void vgahost_shutdown(void)
{
#if 0
  if (vga_state != NULL)
    {
      /* Clean up the display, but only if we've done a mode set.
       * This makes sure that error messages don't get erased.
       */
      if (vga_current_mode != NULL)
	restore_vga_state ();
      free (vga_state);
      vga_state = NULL;
    }
#else
    if(vga_current_mode != NULL)
        restore_vga_state();

#endif
}

static int
selector_for_phys_mem(uint32 base, uint32 num_bytes)
{
    int sel;
    uint32 seg_lim;
    __dpmi_meminfo minfo;

    /* Allocate a descriptor. */
    sel = __dpmi_allocate_ldt_descriptors(1);
    if(sel == -1)
        return -1;

    seg_lim = ((num_bytes + 4095) & ~4095) - 1;

    /* Map the physical memory into linear address space. */
    minfo.handle = 0; /* unused */
    minfo.size = seg_lim + 1;
    minfo.address = base;
    if(__dpmi_physical_address_mapping(&minfo) != 0)
        return -1;

    if(__dpmi_set_segment_base_address(sel, minfo.address) == -1)
        return -1;
    if(__dpmi_set_segment_limit(sel, seg_lim) == -1)
        return -1;

    return sel;
}

/* This function queries the system and constructs a list of useful
 * graphics modes.
 */

vga_mode_t *
vgahost_compute_vga_mode_list(void)
{
    vesa_info_t vesa_info;
    static const vga_mode_t standard_vga_modes[NUM_STANDARD_VGA_MODES] = {
        { 640, 480, 80, 0x12, 2, true, false, false, 0, 0, 65536, 65536, -1 }
    };
    static const vga_mode_t standard_vesa_modes[NUM_VESA_MODES] = {
        /* Many of these fields are intentionally left blank or have defaults. */
        { 640, 400, 0, 0x100, 3, false, false, true, 0, 0, 65536, 65536, -1 },
        { 640, 480, 0, 0x101, 3, false, false, true, 0, 0, 65536, 65536, -1 },
        { 800, 600, 0, 0x102, 2, true, false, false, 0, 0, 65536, 65536, -1 },
        { 800, 600, 0, 0x103, 3, false, false, true, 0, 0, 65536, 65536, -1 },
        { 1024, 768, 0, 0x104, 2, true, false, true, 0, 0, 65536, 65536, -1 },
        { 1024, 768, 0, 0x105, 3, false, false, true, 0, 0, 65536, 65536, -1 },
        { 1280, 1024, 0, 0x106, 2, true, false, true, 0, 0, 65536, 65536, -1 },
        { 1280, 1024, 0, 0x107, 3, false, false, true, 0, 0, 65536, 65536, -1 },
    };
    int num_modes, i;
    __dpmi_regs regs;

    /* Default to an empty list. */
    mode_list[0].width = 0;

    /* Add in the normal VGA modes we know about. */
    memcpy(mode_list, standard_vga_modes, sizeof standard_vga_modes);
    num_modes = (sizeof standard_vga_modes) / (sizeof standard_vga_modes[0]);
    for(i = 0; i < num_modes; i++)
        mode_list[i].screen_selector = vga_window_selector;

    if(get_vesa_info_block(&vesa_info))
    {
        unsigned long list, num_mode_numbers, n;
        unsigned short mode_num, *vesa_mode_numbers;

        /* Figure out what type of DAC we have. */
        switchable_dac_p = (vesa_version >= 0x200
                            && (vesa_info.capabilities[0] & SWITCHABLE_DAC));
        set_palette_during_vbl_mask = ((vesa_info.capabilities[0]
                                        & SET_PALETTE_DURING_VBL)
                                           ? 0x80
                                           : 0);

        /* Count the number of modes in their list. */
        list = (vesa_info.mode_list_segment * 16
                + vesa_info.mode_list_offset);
        for(num_mode_numbers = 0, mode_num = 0;
            mode_num != 0xFFFF;
            num_mode_numbers++)
        {
            dosmemget(list + num_mode_numbers * sizeof(short),
                      sizeof mode_num, &mode_num);
        }
        num_mode_numbers--; /* Don't count the 0xFFFF */

        /* Allocate an array to hold the modes. */
        vesa_mode_numbers = ((unsigned short *)
                                 alloca(num_mode_numbers
                                        * sizeof vesa_mode_numbers[0]));
        dosmemget(list, num_mode_numbers * sizeof vesa_mode_numbers[0],
                  vesa_mode_numbers);

        for(n = 0; n < num_mode_numbers && num_modes < MAX_VGA_MODES; n++)
        {
            unsigned short mode_num;
            mode_info_t mode_info;
            vga_mode_t *m;

            /* Mask out all but the real mode bits. */
            mode_num = vesa_mode_numbers[n] & 0x3FFF;

            if(!VESA_MODE_P(mode_num)) /* Is this not a VESA mode? */
                continue;

            /* Call BIOS function to get SVGA mode information. */
            dpmi_zero_regs(&regs);
            regs.x.cx = mode_num;
            regs.x.es = dos_buf_segment;
            regs.x.di = 0;

            if(!vesa_call(VESA_GET_MODE_INFO, &regs))
                continue;

            /* Pull the new information in from DOS memory. */
            movedata(dos_buf_selector, 0, dos_pm_ds, (unsigned)&mode_info,
                     sizeof mode_info);

            if(!(mode_info.mode_attributes & ATTR_LEGAL_MODE)
               || !(mode_info.mode_attributes & ATTR_GRAPHICS_MODE))
                continue;

            m = &mode_list[num_modes];
            memset(m, 0, sizeof *m);

            if(mode_info.mode_attributes & ATTR_EXTENDED_INFO)
            {
                /* Filter out unacceptable graphics modes.  We want either
	       * simple packed screens or 4bpp planar screens only.
	       */
                if(mode_info.width < VDRIVER_MIN_SCREEN_WIDTH
                   || mode_info.height < VDRIVER_MIN_SCREEN_HEIGHT)
                    continue;
                if(mode_info.bits_per_pixel > 8)
                    continue;
                if(mode_info.memory_model != MEMORY_MODEL_PACKED_PIXELS
                   && (mode_info.bits_per_pixel != 4
                       || (mode_info.memory_model
                           != MEMORY_MODEL_FOUR_PLANES)))
                    continue;
                if(mode_info.num_planes != 1
                   && (mode_info.num_planes != 4
                       || mode_info.bits_per_pixel != 4))
                    continue;

                m->width = mode_info.width;
                m->height = mode_info.height;
                m->log2_bpp = ROMlib_log2[mode_info.bits_per_pixel];
                m->planar_p = (mode_info.num_planes != 1);
            }
            else if(mode_num >= 0x100 && mode_num < 0x108)
            {
                /* No explicit information about this mode.  VESA spec
	       * says this is legal as long as it is one of the
	       * standard VESA-defined modes, which it is.
	       */
                *m = standard_vesa_modes[mode_num - 0x100];
            }
            else
                continue; /* Strange; no information about this mode! */

            m->row_bytes = mode_info.row_bytes;
            m->win_granularity = mode_info.win_granularity * 1024;
            m->interlaced_p = false;
            if((mode_info.win_a_attributes & 0x3) == 0x3)
                m->win_read = 0;
            else if((mode_info.win_b_attributes & 0x3) == 0x3)
                m->win_read = 1;
            else
                continue; /* No readable window at all! */
            if((mode_info.win_a_attributes & 0x5) == 0x5)
                m->win_write = 0;
            else if((mode_info.win_b_attributes & 0x5) == 0x5)
                m->win_write = 1;
            else
                continue; /* No writeable window at all! */

            /* Default to not knowing the physical base address. */
            m->phys_base_addr = 0;

            if(vesa_version >= 0x200
               && (mode_info.mode_attributes & ATTR_LINEAR_FBUF)
               && mode_info.phys_base_addr >= 1024 * 1024)
            {
                int sel;

                m->phys_base_addr = mode_info.phys_base_addr;

                sel = selector_for_phys_mem(m->phys_base_addr,
                                            m->row_bytes * m->height);
                if(sel != -1)
                {
                    mode_num |= USE_LINEAR_FBUF;
                    m->multi_window_p = false;
                    m->screen_selector = sel;
                    m->win_size = m->row_bytes * m->height;
                }
            }

            /* If we failed to set to linear mode, do it the
	   * old-fashioned way.
	   */
            if(!(mode_num & USE_LINEAR_FBUF))
            {
                m->multi_window_p = (m->row_bytes * m->height > 65536);
                m->screen_selector = vga_window_selector;
                m->win_size = mode_info.win_size * 1024;
                if(m->win_size > 65536) /* So we don't violate selector. */
                    m->win_size = 65536;
            }

            m->mode_number = mode_num;

            num_modes++;
            if(m->planar_p && m->row_bytes * m->height > m->win_size)
            {
                m->log2_bpp = 0; /* Only allow the 1bpp version. */
            }
            else
            {
                /* If we found a 4bpp planar mode, add the corresponding
	       * 1bpp mode we can get by only dealing with one of the
	       * planes.
	       */
                if(num_modes < MAX_VGA_MODES && m->planar_p)
                {
                    mode_list[num_modes] = *m;
                    mode_list[num_modes].log2_bpp = 0;
                    num_modes++;
                }
            }
        }
    }

    /* Terminate the mode list. */
    mode_list[num_modes].width = 0;

    return mode_list;
}

#define SET_VGA_COLOR(c, r, g, b)         \
    asm volatile("outb %%al,%%dx\n\t"     \
                 "inb $0x80,%%al\n\t"     \
                 "incl %%edx\n\t"         \
                 "movl %2,%%eax\n\t"      \
                 "shrb $2,%%al\n\t"       \
                 "outb %%al,%%dx\n\t"     \
                 "inb $0x80,%%al\n\t"     \
                 "movl %3,%%eax\n\t"      \
                 "shrb $2,%%al\n\t"       \
                 "outb %%al,%%dx\n\t"     \
                 "inb $0x80,%%al\n\t"     \
                 "movl %4,%%eax\n\t"      \
                 "shrb $2,%%al\n\t"       \
                 "outb %%al,%%dx"         \
                 :                        \
                 : "a"(c), "d"(0x3c8),    \
                   "g"(r), "g"(g), "g"(b) \
                 : "ax", "dx")

typedef struct
{
    uint8 blue, green, red, filler; /* VESA spec is backwards */
} vbe2_rgb_t;

static inline unsigned const
shift_and_round(unsigned a, int round_bit, int shift)
{
    a += (a & round_bit);
    if(a > 0xFF)
        a = 0xFF;
    a >>= shift;
    return a;
}

void vgahost_set_colors(int first_color, int num_colors,
                        const ColorSpec *color_array)
{
    if(vesa_version >= 0x200)
    {
        __dpmi_regs regs;
        int k, shift;
        vbe2_rgb_t *r;

        /* Fill in our table assuming an 8 bpp DAC at first. */
        r = alloca(num_colors * sizeof r[0]);
        for(k = num_colors - 1; k >= 0; k--)
        {
            r[k].filler = 0;
            r[k].red = (*(const uint8 *)&color_array[k].rgb.red); /* MSB */
            r[k].green = (*(const uint8 *)&color_array[k].rgb.green); /* MSB */
            r[k].blue = (*(const uint8 *)&color_array[k].rgb.blue); /* MSB */
        }

        /* Next round to nearest value and shift bits for < 8 bpp DACs. */
        shift = 8 - bits_per_dac_element;
        if(shift > 0)
        {
            int j, round_bit;

            round_bit = 1 << (shift - 1);
            for(j = num_colors - 1; j >= 0; j--)
            {
                r[j].red = shift_and_round(r[j].red, round_bit, shift);
                r[j].green = shift_and_round(r[j].green, round_bit, shift);
                r[j].blue = shift_and_round(r[j].blue, round_bit, shift);
            }
        }

        /* Transfer the color array to conventional memory. */
        movedata(dos_pm_ds, (unsigned)r, dos_buf_selector, 0,
                 num_colors * sizeof r[0]);

        /* Call the VESA function to set the palette. */
        dpmi_zero_regs(&regs);
        regs.h.bl = 0x00;
        regs.x.cx = num_colors;
        regs.x.dx = first_color;
        regs.x.es = dos_buf_segment;
        regs.x.di = 0;
        /* Only wait for a VBL if we're setting many colors.  If we're
       * setting one at a time, it would be bad to wait for a new VBL for
       * each!
       */
        if(num_colors > 30) /* arbitrary */
            regs.h.bl |= set_palette_during_vbl_mask;
        if(!vesa_call(VESA_PALETTE_MANIP, &regs))
            warning_unexpected("Failed to set palette!");
    }
    else
    {
        const ColorSpec *c;
        int i;

        for(c = color_array, i = 0; i < num_colors; c++, i++)
        {
            unsigned r, g, b;

            r = *(const uint8 *)&c->rgb.red;
            if(r < 0xFC)
                r += (r & 2); /* Round to nearest. */
            g = *(const uint8 *)&c->rgb.green;
            if(g < 0xFC)
                g += (g & 2); /* Round to nearest. */
            b = *(const uint8 *)&c->rgb.blue;
            if(b < 0xFC)
                b += (b & 2); /* Round to nearest. */

            SET_VGA_COLOR((first_color + i), r, g, b);
        }
    }
}

void vgahost_set_write_window(int window_num)
{
    if(window_num != vga_write_window && vga_current_mode->multi_window_p)
    {
        __dpmi_regs regs;

        /* Set up the write window to point to the right place. */
        dpmi_zero_regs(&regs);
        regs.x.ax = VESA_WINDOW_CONTROL;
        regs.x.bx = vga_current_mode->win_write; /* BH == 0x00 */
        regs.x.dx = window_num;
        __dpmi_int(0x10, &regs);

        vga_write_window = window_num;
        if(vga_current_mode->win_read == vga_current_mode->win_write)
            vga_read_window = window_num;
    }
}

void vgahost_set_read_window(int window_num)
{
    if(window_num != vga_read_window && vga_current_mode->multi_window_p)
    {
        __dpmi_regs regs;

        /* Set up the write window to point to the right place. */
        dpmi_zero_regs(&regs);
        regs.x.ax = VESA_WINDOW_CONTROL;
        regs.x.bx = vga_current_mode->win_read; /* BH == 0x00 */
        regs.x.dx = window_num;
        __dpmi_int(0x10, &regs);

        vga_read_window = window_num;
        if(vga_current_mode->win_read == vga_current_mode->win_write)
            vga_write_window = window_num;
    }
}

bool vgahost_illegal_mode_p(int width, int height, int bpp,
                            bool exact_match_p)
{
    if(!actually_using_fat_ds_vga_hack_p || vga_current_mode == NULL)
        return false;
    return (vdriver_fbuf == vdriver_real_screen_baseaddr
            && bpp != (1 << vga_current_mode->log2_bpp));
}

/* Attempts to mmap sufficient memory for the specified video mode
 * over the memory pointed to by vdriver_fbuf, which must be
 * page-aligned.  Returns true if successful, else false.
 */
bool vgahost_mmap_linear_fbuf(const vga_mode_t *mode)
{
    bool success_p;

    success_p = false; /* Default */
    if(mode->phys_base_addr != 0) /* do we have a linear frame buffer? */
    {
        if(!__djgpp_map_physical_memory(vdriver_fbuf,
                                        ((mode->height * mode->row_bytes
                                          + DPMI_PAGE_SIZE - 1)
                                         & ~(DPMI_PAGE_SIZE - 1)),
                                        mode->phys_base_addr))
        {
            success_p = true;
        }
        else if(mode->screen_selector != 0 && try_to_use_fat_ds_vga_hack_p)
        {
            /* We failed to mmap it the DPMI 1.0 way, so we'll see if we
	   * can use the fat %ds hack.
	   */
            if(__djgpp_nearptr_enable())
            {
                unsigned long executor_base = 0, fbuf_base = 0;
                if(!__dpmi_get_segment_base_address(dos_pm_ds, &executor_base)
                   && !__dpmi_get_segment_base_address(mode->screen_selector,
                                                       &fbuf_base))
                {
                    vdriver_fbuf = (uint8 *)(fbuf_base - executor_base);
                    actually_using_fat_ds_vga_hack_p = true;
                    success_p = true;
                }
            }
        }
    }

    if(success_p)
    {
        /* Cool beans.  Note that the screen is in our address space. */
        vga_screen_selector = dos_pm_ds;
    }

    return success_p;
}

/* Replaces the frame buffer space with RAM, instead of memory
 * mapped on to the video card.  Returns true If successful,
 * else false.
 */
bool vgahost_unmap_linear_fbuf(unsigned long num_bytes)
{
    bool success_p;

    if(actually_using_fat_ds_vga_hack_p)
    {
        /* Can't unmap a fat %ds frame buffer! */
        success_p = false;
    }
    else
    {
        /* Set all pages to committed, r/w, not accessed, not dirty;
       * see DPMI spec for details.
       */
        success_p = !__djgpp_set_page_attributes(vdriver_fbuf,
                                                 ((num_bytes + DPMI_PAGE_SIZE
                                                   - 1)
                                                  & ~(DPMI_PAGE_SIZE - 1)),
                                                 0x1 | (0x1 << 3) | (0x1 << 4));
    }

    return success_p;
}

bool vgahost_set_mode(vga_mode_t *mode)
{
    __dpmi_regs regs;
    bool success_p;
    ColorSpec clut[256];
    int i;

    success_p = true;

    /* Actually set up the current mode.   Always do this, even if the
   * mode isn't changing, because sometimes the video mode gets
   * scrogged when people hotkey around under Windows, etc. and we
   * want to be able to reset the mode.
   */
    if(VESA_MODE_P(mode->mode_number))
    {
        if(!setmode(mode->mode_number ^ VBE_MODE_BIT))

        {
            success_p = false;
            goto done;
        }

        /* Guarantee that we've got window #0. */
        vga_write_window = -1;
        vgahost_set_write_window(0);
    }
    else /* Old-style VGA mode.  Don't use any SVGA calls. */
    {
        if(!setmode(mode->mode_number))
        {
            success_p = false;
            goto done;
        }
    }

    /* Attempt to set the DAC to 8 bit mode if possible. */
    bits_per_dac_element = 6; /* default */
    if(switchable_dac_p)
    {
        __dpmi_regs set_dac_regs;

        /* Attempt to set the DAC to 8 bit mode. */
        dpmi_zero_regs(&set_dac_regs);
        set_dac_regs.x.bx = 0x0800; /* set DAC format to 8 bit. */
        if(vesa_call(VESA_SET_GET_PALETTE_FORMAT, &set_dac_regs))
            bits_per_dac_element = set_dac_regs.h.bh;
    }

    /* Reset CLUT to black, initially, gray later.  This seems
   * to look better when the splash screen first comes up.
   */
    clut[0] = (vga_current_mode == NULL
                   ? ROMlib_black_cspec
                   : ROMlib_gray_cspec);
    for(i = 255; i > 0; i--)
        clut[i] = clut[0];
    vgahost_set_colors(0, 256, clut);

    if(mode->planar_p)
    {
        int i;

        /* Select 16 pages of 16 color registers. */
        dpmi_zero_regs(&regs);
        regs.x.ax = 0x1013;
        regs.x.bx = 0x0100;
        __dpmi_int(0x10, &regs);

        /* Select page 0. */
        dpmi_zero_regs(&regs);
        regs.x.ax = 0x1013;
        regs.x.bx = 0x0001;
        __dpmi_int(0x10, &regs);

        /* Map palette register N to color register N. */
        for(i = 0; i < 16; i++)
        {
            dpmi_zero_regs(&regs);
            regs.x.ax = 0x1000;
            regs.x.bx = i | (i << 8);
            __dpmi_int(0x10, &regs);
        }
    }

    /* The screen always starts 0 bytes into the screen segment;
   * this value will be changed later if we successfully mmap
   * the screen into our address space
   */
    vga_portal_baseaddr = 0;
    vdriver_real_screen_baseaddr = 0;

    /* Grab the selector for the current screen. */
    vga_screen_selector = mode->screen_selector;

done:
    return success_p;
}
