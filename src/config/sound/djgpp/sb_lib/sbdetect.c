#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dpmi.h>
#include <go32.h>
#include <inlines/pc.h>
#include <time.h>

#include "sbdetect.h"

#define RETURN_ERROR(err)  do { temp=(err); goto end; }while(0)

struct _sb_info sb_info =
{
  0, 0, 0, 0, 0, 0, 0,
#ifdef SB_SUPPORT_16_BIT
  0,
#endif
  0
};

static volatile int testCount;
static int dontSetDMA8 = 0;
int sb_disable16bit = 0;
#ifdef SB_SUPPORT_16_BIT
static int dontSetDMA16 = 0;
#endif

static __inline__ void 
sb_dspWrite (int val)
{
  while ((inportb (sb_info.writeData) & 0x80) != 0);
  outportb (sb_info.writeData, val);
}

static __inline__ int 
sb_dspRead (void)
{
  while ((inportb (sb_info.dataAvail) & 0x80) == 0);
  return inportb (sb_info.readData);
}

static void 
topOfFunctions (void)
{
}

static void 
testInt2 (void)
{
  if(testCount==0) {
    testCount = 2;
    inportb (sb_info.dataAvail);
  }
  outportb (0x20, 0x20);
}

static void 
testInt3 (void)
{
  testCount = 3;
  inportb (sb_info.dataAvail);
  outportb (0x20, 0x20);
}

static void 
testInt5 (void)
{
  testCount = 5;
  inportb (sb_info.dataAvail);
  outportb (0x20, 0x20);
}

static void 
testInt7 (void)
{
  testCount = 7;
  inportb (sb_info.dataAvail);
  outportb (0x20, 0x20);
}

static void
testInt9 (void)
{
  testCount = 9;
  inportb (sb_info.dataAvail);
  outportb (0x20, 0x20);
  outportb (0xA0, 0x20);
}

static void
testInt10 (void)
{
  testCount = 10;
  inportb (sb_info.dataAvail);
  outportb (0x20, 0x20);
  outportb (0xA0, 0x20);
}

static void 
bottomOfFunctions (void)
{
}

static int 
findInterrupt (void)
{
  __dpmi_paddr old2, old3, old5, old7, old9, old10;
  __dpmi_paddr new2, new3, new5, new7, new9, new10;
  _go32_dpmi_seginfo wrap2, wrap3, wrap5, wrap7, wrap9, wrap10;
  BYTE pic1Default, pic2Default;
  static int start_time_1;
  int failed, retval;

  testCount = 0;

  if (!_go32_dpmi_lock_code (topOfFunctions,
			     ((char *) bottomOfFunctions
			      - (char *) topOfFunctions)))
    {
      if (_go32_dpmi_lock_data ((void *) &testCount, sizeof testCount))
	  return 0;
    }
  else
    return 0;

  __dpmi_get_protected_mode_interrupt_vector (0x0A, &old2);
  __dpmi_get_protected_mode_interrupt_vector (0x0B, &old3);
  __dpmi_get_protected_mode_interrupt_vector (0x0D, &old5);
  __dpmi_get_protected_mode_interrupt_vector (0x0F, &old7);
  __dpmi_get_protected_mode_interrupt_vector (0x71, &old9);
  __dpmi_get_protected_mode_interrupt_vector (0x72, &old10);

  wrap2.pm_offset = (int) testInt2;
  wrap2.pm_selector = _my_cs ();
  wrap3.pm_offset = (int) testInt3;
  wrap3.pm_selector = _my_cs ();
  wrap5.pm_offset = (int) testInt5;
  wrap5.pm_selector = _my_cs ();
  wrap7.pm_offset = (int) testInt7;
  wrap7.pm_selector = _my_cs ();
  wrap9.pm_offset = (int) testInt9;
  wrap9.pm_selector = _my_cs ();
  wrap10.pm_offset = (int) testInt10;
  wrap10.pm_selector = _my_cs ();

  _go32_dpmi_allocate_iret_wrapper (&wrap2);
  _go32_dpmi_allocate_iret_wrapper (&wrap3);
  _go32_dpmi_allocate_iret_wrapper (&wrap5);
  _go32_dpmi_allocate_iret_wrapper (&wrap7);
  _go32_dpmi_allocate_iret_wrapper (&wrap9);
  _go32_dpmi_allocate_iret_wrapper (&wrap10);

  new2.offset32 = wrap2.pm_offset;
  new2.selector = wrap2.pm_selector;
  new3.offset32 = wrap3.pm_offset;
  new3.selector = wrap3.pm_selector;
  new5.offset32 = wrap5.pm_offset;
  new5.selector = wrap5.pm_selector;
  new7.offset32 = wrap7.pm_offset;
  new7.selector = wrap7.pm_selector;
  new9.offset32 = wrap9.pm_offset;
  new9.selector = wrap9.pm_selector;
  new10.offset32 = wrap10.pm_offset;
  new10.selector = wrap10.pm_selector;

  pic1Default = inportb (0x21);
  pic2Default = inportb (0xA1);

  outportb (0x21, pic1Default & 0x53);	/* Clear all relevant masks */
  outportb (0xA1, pic2Default & 0xF9);
  __dpmi_set_protected_mode_interrupt_vector (0x0A, &new2);
  __dpmi_set_protected_mode_interrupt_vector (0x0B, &new3);
  __dpmi_set_protected_mode_interrupt_vector (0x0D, &new5);
  __dpmi_set_protected_mode_interrupt_vector (0x0F, &new7);
  __dpmi_set_protected_mode_interrupt_vector (0x71, &new9);
  __dpmi_set_protected_mode_interrupt_vector (0x72, &new10);

  /* Some SB's get into a strange state where this hangs, so we add
   * a timeout here.
   */
  start_time_1 = rawclock ();
  failed = 0;
  while ((inportb (sb_info.writeData) & 0x80) != 0)
    {
      DWORD now1 = rawclock ();
      if (now1 - start_time_1 >= 18)
	{
	  failed = 1;
	  break;
	}
    }

  if (failed)
    retval = 0;
  else
    {
      int start_time_2;

      start_time_2 = rawclock ();

      testCount = 0;
      outportb (sb_info.writeData, 0xF2);	/* force the DSP to signal */

      while (!(retval = testCount))
	{				/* a hardware interrupt.             */
	  DWORD now2 = rawclock ();
	  if (now2 - start_time_2 >= 18)
	    break;
	}
    }

  outportb (0x21, pic1Default);
  outportb (0xA1, pic2Default);
  __dpmi_set_protected_mode_interrupt_vector (0x0A, &old2);
  __dpmi_set_protected_mode_interrupt_vector (0x0B, &old3);
  __dpmi_set_protected_mode_interrupt_vector (0x0D, &old5);
  __dpmi_set_protected_mode_interrupt_vector (0x0F, &old7);
  __dpmi_set_protected_mode_interrupt_vector (0x71, &old9);
  __dpmi_set_protected_mode_interrupt_vector (0x72, &old10);

  _go32_dpmi_free_iret_wrapper (&wrap2);
  _go32_dpmi_free_iret_wrapper (&wrap3);
  _go32_dpmi_free_iret_wrapper (&wrap5);
  _go32_dpmi_free_iret_wrapper (&wrap7);
  _go32_dpmi_free_iret_wrapper (&wrap9);
  _go32_dpmi_free_iret_wrapper (&wrap10);

  return retval;
}

static void 
waitInit (void)
{
  int temp;

  temp = inportb (0x61);
  temp &= 0xFD;
  temp |= 0x01;
  outportb (0x61, temp);
}

static void 
microWait (WORD usec)
{
  WORD elapsed;
  unsigned long failsafe;

  outportb (0x43, 0xB0);
  outportb (0x42, 0xFF);
  outportb (0x42, 0xFF);

  /* Sometimes this timer doesn't seem to work, and our program hangs! */
  failsafe = usec * 10000;

  do
    {
      outportb (0x43, 0x80);
      elapsed = inportb (0x42);
      elapsed |= (inportb (0x42) << 8);
      elapsed = ~elapsed;
    }
  while (elapsed < usec && failsafe--);
}

static int 
dspReset (void)
{
  int a;
  int success;

  outportb (sb_info.reset, 1);
  microWait (4);
  outportb (sb_info.reset, 0);

  success = 0;
  for (a = 0; a < 1000; a++)
    {
      if (inportb (sb_info.dataAvail) & 0x80)
	{
	  success = 1;
	  break;
	}
    }
  if (success)
    {
      for (a = 0; a < 1000; a++)
	{
	  if (inportb (sb_info.readData) == 0xAA)
	    {
	      success = 2;
	      break;
	    }
	}
    }
  if (success != 2)
    return 0;

  sb_dspWrite (0xE1);
  sb_info.dspVersion = sb_dspRead ();
  sb_info.dspVersion <<= 8;
  sb_info.dspVersion |= sb_dspRead ();
  return 1;
}

static sb_status 
findSoundBlaster (void)
{
  static WORD ioaddr[7] = {0x220, 0x240, 0x210, 0x230, 0x250, 0x260, 0x280};
  int a;
  sb_status stat = SB_FAILURE;

  for (a = 0; a < 7; a++)
    {
      sb_info.reset = ioaddr[a] + 0x06;
      sb_info.readData = ioaddr[a] + 0x0A;
      sb_info.writeData = ioaddr[a] + 0x0C;
      sb_info.dataAvail = ioaddr[a] + 0x0E;
      sb_info.dataAvail16 = ioaddr[a] + 0x0F;

      if (dspReset ())
	{			/* Found the right IO address! */
	  a = 7;
	  if ((sb_info.IRQ = findInterrupt ()))
	    {			/* ...grab the interrupt vector */
	      if (!dontSetDMA8)
		sb_info.DMA = 1;	/* Assume DMA channel 1 and */
#ifdef SB_SUPPORT_16_BIT
	      if (!dontSetDMA16)
		sb_info.DMA16 = 5;	/* 16-bit DMA channel 5     */
#endif
	      stat = SB_SUCCESS;
	    }
	}
    }
  return stat;
}

/*/////////////////////////////////////////////////////////////////////////////
/                              Global functions                               /
//////////////////////////////////////////////////////////////////////////// */

sb_status 
sb_is_present (void)
{
  static char *blaster;
  char *address;
  short sbIO;
  sb_status temp;

  waitInit ();
  blaster = getenv ("BLASTER");

  if (_go32_dpmi_lock_data (&sb_info, sizeof (sb_info)))
    RETURN_ERROR (SB_FAILURE);

  if (blaster != NULL)
    {
      strupr (blaster);
      address = strrchr (blaster, 'A');
      if (address == NULL)
	RETURN_ERROR (SB_BAD_BLASTER);

      ++address;
      sscanf (address, "%hx", &sbIO);
      if (sbIO != 0x210 && sbIO != 0x220 && sbIO != 0x230 && sbIO != 0x240
	  && sbIO != 0x250 && sbIO != 0x260 && sbIO != 0x280)
	RETURN_ERROR (SB_BAD_ADDRESS);

      sb_info.reset       = sbIO + 0x06;
      sb_info.readData    = sbIO + 0x0A;
      sb_info.writeData   = sbIO + 0x0C;
      sb_info.dataAvail   = sbIO + 0x0E;
      sb_info.dataAvail16 = sbIO + 0x0F;

      sb_info.IRQ=findInterrupt();      /* Forget what BLASTER says and find
                                           the IRQ myself.                  */
      if(sb_info.IRQ==0)
        RETURN_ERROR(SB_FAILURE);

      address = strrchr (blaster, 'D');
      if (address == NULL)
	RETURN_ERROR (SB_BAD_BLASTER);

      ++address;
      sscanf (address, "%d", &sb_info.DMA);
      if (sb_info.DMA != 0 && sb_info.DMA != 1 && sb_info.DMA != 3)
	RETURN_ERROR (SB_BAD_DMA);

#ifdef SB_SUPPORT_16_BIT
      address = strrchr (blaster, 'H');
      if (address == NULL)	/* No 16bit DMA in BLASTER variable, */
	sb_info.DMA16 = 5;	/* assume 5                          */
      else
	{
	  ++address;
	  sscanf (address, "%d", &sb_info.DMA16);
          if(sb_info.DMA16 < 5)
            sb_disable16bit = 1;
          else if(sb_info.DMA16 > 7)
            RETURN_ERROR (SB_BAD_DMA16);
	}
#endif

      if (!dspReset ())		/* Verify address */
	RETURN_ERROR (SB_BAD_ADDRESS);

      temp = SB_SUCCESS;
    }
  else
    {
      temp = findSoundBlaster ();
    }

end:
  return temp;
}

void 
sb_change_dma8_channel (int newChannel)
{
  sb_info.DMA = newChannel;
  dontSetDMA8 = 1;
}

#ifdef SB_SUPPORT_16_BIT
void 
sb_change_dma16_channel (int newChannel)
{
  sb_info.DMA16 = newChannel;
  dontSetDMA16 = 1;
}
#endif
