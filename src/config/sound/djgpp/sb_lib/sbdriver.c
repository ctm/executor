#include <stdio.h>
#include <string.h>
#include <dpmi.h>
#include <go32.h>
#include <inlines/pc.h>

#include "sb_defs.h"
#include "sbdetect.h"
#include "sb_dma.h"

#define SB_MAX_QUEUE_SIZE    4
#define SB_SAMPLE_FREQUENCY  22255
#define TIME_CONSTANT        (256-(1000000/SB_SAMPLE_FREQUENCY))
#define PRO_TIME_CONSTANT    ((65536-(256000000/(SB_SAMPLE_FREQUENCY*2)))/256)

char sb_driver_error[80];

static void (*hisCallback) (void);

static WORD dmaBufferSelector[SB_MAX_QUEUE_SIZE];
static int samplesInBuffer[SB_MAX_QUEUE_SIZE];
static volatile int pointer[SB_MAX_QUEUE_SIZE] = {0, 1, 2, 3};
static DWORD sb_dmaBufferLinearAddress[SB_MAX_QUEUE_SIZE];

static int endOfDMAInterruptVector;
static BYTE pic1Default, pic2Default;
static __dpmi_paddr oldHandler, newHandler;
static _go32_dpmi_seginfo wrapper;
static int driverInstalled = 0;

volatile int sb_numInQueue = 0;
static int soundEnabled = 1;
static int backedUp = 0;
static int bufferFormat = SB_8_BIT | SB_MONO;
static unsigned char mixerDefault;

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
startSample (DWORD address, DWORD length)
{
  if (bufferFormat & SB_8_BIT)
    {
      sb_dma8bitReadSC (address, length);
      if (bufferFormat & SB_MONO)
	{
	  if (sb_info.dspVersion >= 0x0400)
	    {			/* SB-16        */
	      sb_dspWrite (0xC0);	/*   8-Bit DAC, Single-Cycle    */
	      sb_dspWrite (0x00);	/*   Monaural, Unsigned samples */
	      sb_dspWrite (length & 0xFF);
	      sb_dspWrite (length >> 8);
	    }
	  else
	    {			/* Vanilla SB   */
	      sb_dspWrite (0x14);	/* 8-bit DAC, Single-Cycle, Monaural */
	      sb_dspWrite (length & 0xFF);
	      sb_dspWrite (length >> 8);
	    }
	}
      else
	{
	  if (sb_info.dspVersion < 0x0400)
	    {			/* SB-Pro       */
	      sb_dspWrite (0x48);
	      sb_dspWrite (length & 0xFF);
	      sb_dspWrite (length >> 8);
	      sb_dspWrite (0x91);
	    }
	  else
	    {			/* SB-16        */
	      sb_dspWrite (0xC0);	/*   8-bit DAC, Single-Cycle */
	      sb_dspWrite (0x20);	/*   Stereo, Unsigned samples */
	      sb_dspWrite (length & 0xFF);
	      sb_dspWrite (length >> 8);
	    }
	}
    }
  else
    {
#ifndef SB_SUPPORT_16_BIT
      abort ();		/* Shouldn't get here */
#else /* SB_SUPPORT_16_BIT */
      sb_dma16bitReadSC (address, length);
      sb_dspWrite (0xB0);	/* 16-bit DAC, Single-Cycle */
      if (bufferFormat & SB_MONO)
	sb_dspWrite (0x00);	/* Monaural, Unsigned samples */
      else
	sb_dspWrite (0x20);	/* Stereo, Unsigned samples */
      sb_dspWrite (length & 0xFF);
      sb_dspWrite (length >> 8);
#endif /* SB_SUPPORT_16_BIT */
    }
}

static void 
sb_interrupt_handler (void)
{
  int i, temp;

  if (bufferFormat & SB_8_BIT)
    inportb (sb_info.dataAvail);	/* Acknowledge the SB */
  else
    {
#ifdef SB_SUPPORT_16_BIT
      inportb (sb_info.dataAvail16);
#else
      abort ();
#endif
    }

  if (sb_numInQueue > 1)
    {
      startSample (sb_dmaBufferLinearAddress[pointer[1]],
		   samplesInBuffer[pointer[1]] - 1);
      temp = pointer[0];
      for (i = 0; i < SB_MAX_QUEUE_SIZE - 1; i++)
	pointer[i] = pointer[i + 1];
      pointer[SB_MAX_QUEUE_SIZE - 1] = temp;
    }
  --sb_numInQueue;

  outportb (0x20, 0x20);	/* Acknowledge PIC1 */
  outportb (0xA0, 0x20);	/* Acknowledge PIC2 */
  if (hisCallback != NULL)	/* Informing user has least priority */
    (*hisCallback) ();
}

static void 
bottomOfFunctions (void)
{
}

static sb_status 
getSBInfo (void)
{
  sb_status stat;

  if (sb_info.dspVersion == 0)
    {
      stat = sb_is_present ();
      switch (stat)
	{
	case SB_BAD_BLASTER:
	  strcpy (sb_driver_error, "BLASTER environment variable incomplete!");
	  break;
	case SB_BAD_ADDRESS:
	  strcpy (sb_driver_error, "Incorrect address specified in BLASTER environment variable!");
	  break;
	case SB_BAD_IRQ:
	  strcpy (sb_driver_error, "Incorrect IRQ specified in BLASTER environment variable!");
	  break;
	case SB_BAD_DMA:
	  strcpy (sb_driver_error, "Incorrect DMA channel specified in BLASTER environment variable!");
	  break;
	case SB_BAD_DMA16:
	  strcpy (sb_driver_error, "Incorrect 16-bit DMA channel specified in BLASTER environment variable!");
	  break;
	case SB_FAILURE:
	  strcpy (sb_driver_error, "General failure/No sound card detected.");
	  break;
	default:
	  strcpy (sb_driver_error, "Unknown error condition.");
	  break;
	}
      return stat;
    }
  return SB_SUCCESS;
}

static int 
allocateDMAMemory (int bytes, WORD *dosSeg, WORD *dosSel)
{
  int firstPage, lastPage;
  int linearAddress;
  int dosSegs[16];
  int dosSels[16];
  int paragraphs = (bytes + 15) >> 4;
  int currentTry = -1;

  do
    {
      ++currentTry;

      if (currentTry > 15)
	{
	  strcpy (sb_driver_error, "Tried 16 times -- all failed");
	  return 0;
	}

      dosSegs[currentTry] = __dpmi_allocate_dos_memory (paragraphs,
							&dosSels[currentTry]);

      if (dosSegs[currentTry] == -1)
	{
	  sprintf (sb_driver_error, "After %d attempts, DOS allocate failed",
		   currentTry);
	  return 0;
	}

      linearAddress = dosSegs[currentTry] << 4;
      firstPage = linearAddress >> 16;
      lastPage = (linearAddress + bytes - 1) >> 16;

    }
  while (firstPage != lastPage);

  *dosSeg = (WORD) dosSegs[currentTry];
  *dosSel = (WORD) dosSels[currentTry];

  for (currentTry -= 1; currentTry >= 0; currentTry--)
    __dpmi_free_dos_memory (dosSels[currentTry]);

  return 1;
}

/*****************************************************************************
 *                                                                           *
 *                          HERE STARTS THE API                              *
 *                                                                           *
 *****************************************************************************/

/*
   For sb_enqueue_sample(), 'length' should ALWAYS be samples-per-channel,
   NOT the size of the buffer in bytes:

   FORMAT                    SIZE OF BUFFER(BYTES)
   ------                    ---------------------
   SB_8_BIT|SB_MONO                      length
   SB_8_BIT|SB_STEREO                    length*2
   SB_16_BIT|SB_MONO                     length*2
   SB_16_BIT|SB_STEREO                   length*4
 */
int 
sb_enqueue_sample (const void *data, int length)
{
  if (sb_numInQueue < SB_MAX_QUEUE_SIZE)
    {
      if (bufferFormat & SB_8_BIT)
	{
	  if (bufferFormat & SB_MONO)
	    {
	      if (length <= SB_MAX_8_BIT_MONO_SAMPLES)
		dosmemput (data, length,
			   sb_dmaBufferLinearAddress[pointer[sb_numInQueue]]);
	      else
		return 0;
	    }
	  else
	    {
	      if (length <= SB_MAX_8_BIT_STEREO_SAMPLES)
		dosmemput (data, length * 2,
			   sb_dmaBufferLinearAddress[pointer[sb_numInQueue]]);
	      else
		return 0;
	    }
	}
      else
	{
#ifndef SB_SUPPORT_16_BIT
	  abort ();  /* shouldn't get here */
#else
	  if (bufferFormat & SB_MONO)
	    {
	      if (length <= SB_MAX_16_BIT_MONO_SAMPLES)
		dosmemput (data, length * 2,
			   sb_dmaBufferLinearAddress[pointer[sb_numInQueue]]);
	      else
		return 0;
	    }
	  else
	    {
	      if (length <= SB_MAX_16_BIT_STEREO_SAMPLES)
		dosmemput (data, length * 4,
			   sb_dmaBufferLinearAddress[pointer[sb_numInQueue]]);
	      else
		return 0;
	    }
#endif
	}
      if (bufferFormat & SB_MONO)
	samplesInBuffer[pointer[sb_numInQueue]] = length;
      else
	samplesInBuffer[pointer[sb_numInQueue]] = length * 2;
      if (sb_numInQueue == 0)
	{
	  if (soundEnabled)
	    startSample (sb_dmaBufferLinearAddress[pointer[sb_numInQueue]],
			 samplesInBuffer[pointer[sb_numInQueue]] - 1);
	  else
	    backedUp = 1;
	}
      ++sb_numInQueue;
      return 1;
    }
  return 0;
}

int 
sb_get_capabilities (void)
{
  int caps;

  if (getSBInfo () == SB_SUCCESS)
    {
      caps = SB_8_BIT | SB_MONO;

      if (sb_info.dspVersion >= 0x0300)
	caps |= SB_STEREO;

#ifdef SB_SUPPORT_16_BIT
      if (sb_info.dspVersion >= 0x0400 && !sb_disable16bit)
	caps |= SB_16_BIT;
#endif

      return caps;
    }
  return 0;
}

int 
sb_set_format (int caps)
{
  if (!sb_numInQueue)
    {
      int hardware_caps;

      /* Make sure the hardware really supports this. */
      hardware_caps = sb_get_capabilities ();
      if ((caps & hardware_caps) != caps)
	return 0;

      if ((bufferFormat & SB_STEREO) && (sb_info.dspVersion < 0x0400))
	{
	  outportb (sb_info.reset - 2, 0x0E);
	  outportb (sb_info.reset - 1, mixerDefault);
	}

      bufferFormat = caps;
      if (sb_info.dspVersion < 0x0400)
	{			/* Must be less than SB16 */
	  if (bufferFormat & SB_8_BIT)
	    {
	      if (bufferFormat & SB_MONO)
		{
		  sb_dspWrite (0x40);
		  sb_dspWrite (TIME_CONSTANT);
		}
	      else
		{			   /* If stereo and not SB16 then  */
		  outportb (sb_info.reset - 2, 0x0E);	/* must be SBPro   */
		  mixerDefault = inportb (sb_info.reset - 1);
		  outportb (sb_info.reset - 2, 0x0E);
		  outportb (sb_info.reset - 1, mixerDefault | 0x22);
		  sb_dspWrite (0x40);
		  sb_dspWrite (PRO_TIME_CONSTANT);
		}
	    }
	}
      return 1;
    }
  return 0;
}

int 
sb_set_playback_enabled (int shouldEnable)
{
  int oldStatus = soundEnabled;

  soundEnabled = shouldEnable;
  if (oldStatus != soundEnabled)
    {
      if (!soundEnabled)
	{
	  if (sb_numInQueue > 0)
	    {
	      if (bufferFormat & SB_8_BIT)
		sb_dspWrite (0xD0);	/* 8-bit Halt */
	      else
		{
#ifdef SB_SUPPORT_16_BIT
		  sb_dspWrite (0xD5);	/* 16-bit Halt */
#else
		  abort ();
#endif
		}
	    }
	}
      else
	{
	  if (sb_numInQueue > 0)
	    {
	      if (backedUp)
		{
		  startSample (sb_dmaBufferLinearAddress[pointer[sb_numInQueue]],
			       samplesInBuffer[pointer[sb_numInQueue]] - 1);
		  backedUp = 0;
		}
	      else
		{
		  if (bufferFormat & SB_8_BIT)
		    sb_dspWrite (0xD4);		/* 8-bit Resume */
		  else
		    {
#ifdef SB_SUPPORT_16_BIT
		      sb_dspWrite (0xD6);		/* 16-bit Resume */
#else
		      abort ();
#endif
		    }
		}
	    }
	}
    }
  return oldStatus;
}

sb_status 
sb_install_driver (void (*callback) ())
{
  sb_status stat = SB_FAILURE;
  WORD dmaBufferSegment;
  BYTE picMask;
  int i;

  if (!driverInstalled)
    {
      if ((stat = getSBInfo ()) == SB_SUCCESS)
	{
	  for (i = 0; i < SB_MAX_QUEUE_SIZE; i++)
	    {
	      if (!allocateDMAMemory (SB_BUFFER_SIZE, &dmaBufferSegment,
				      &dmaBufferSelector[i]))
		{
		  stat = SB_FAILURE;
		  strcat (sb_driver_error,
			  "\nUnable to allocate DOS memory buffer!");
		  return stat;
		}
	      sb_dmaBufferLinearAddress[i] = (DWORD) dmaBufferSegment << 4;
	    }

	  hisCallback = callback;
	  pic1Default = inportb (0x21);
	  pic2Default = inportb (0xA1);

	  if (sb_info.IRQ < 8)
	    {
	      endOfDMAInterruptVector = sb_info.IRQ + 0x08;
	      picMask = 1 << sb_info.IRQ;
	      picMask = ~picMask;
	      outportb (0x21, pic1Default & picMask);  /* Enable PIC-1's IRQ */
	    }
	  else
	    {
	      endOfDMAInterruptVector = sb_info.IRQ + 0x68;
	      picMask = 1 << (sb_info.IRQ - 8);
	      picMask = ~picMask;
	      outportb (0x21, pic1Default & 0xFB);	/* Enable IRQ2 */
	      outportb (0xA1, pic2Default & picMask);   /* As well as PIC-2's IRQ */
	    }

	  wrapper.pm_offset = (int) sb_interrupt_handler;
	  wrapper.pm_selector = _my_cs ();
	  _go32_dpmi_allocate_iret_wrapper (&wrapper);
	  newHandler.offset32 = wrapper.pm_offset;
	  newHandler.selector = wrapper.pm_selector;

	  if (_go32_dpmi_lock_code (topOfFunctions,
				    ((char *) bottomOfFunctions
				     - (char *) topOfFunctions))
	      || _go32_dpmi_lock_data (&wrapper, sizeof wrapper)
	      || _go32_dpmi_lock_data (sb_dmaBufferLinearAddress,
				       sizeof sb_dmaBufferLinearAddress)
	      || _go32_dpmi_lock_data (samplesInBuffer, sizeof samplesInBuffer)
	      || _go32_dpmi_lock_data ((void *) pointer, sizeof pointer)
	      || _go32_dpmi_lock_data ((void *) &sb_numInQueue,
					sizeof sb_numInQueue)
	      || _go32_dpmi_lock_data (&bufferFormat, sizeof bufferFormat)
	      || _go32_dpmi_lock_data (&hisCallback, sizeof hisCallback)
	      || !sb_dma_module_lock_your_memory ())
	    {
	      strcpy (sb_driver_error, "Unable to lock appropriate memory.");
	      return SB_FAILURE;
	    }

	  __dpmi_get_protected_mode_interrupt_vector (endOfDMAInterruptVector,
						      &oldHandler);
	  __dpmi_set_protected_mode_interrupt_vector (endOfDMAInterruptVector,
						      &newHandler);

	  sb_dspWrite (0xD1);	/* Turn the speaker on */
	  sb_set_format (SB_8_BIT | SB_MONO);
	  if (sb_info.dspVersion >= 0x0400)
	    {				       	     /* The SB16 needs to be */
	      sb_dspWrite (0x41);		     /* programmed with the  */
	      sb_dspWrite (SB_SAMPLE_FREQUENCY >> 8);	/* sample frequency  */
	      sb_dspWrite (SB_SAMPLE_FREQUENCY & 0xFF);	 /* only once.       */
	    }
	  driverInstalled = 1;
	}
    }
  return stat;
}

void 
sb_uninstall_driver (void)
{
  if (driverInstalled)
    {
      sb_set_playback_enabled (0);  /* stop sound playing */

      sb_dspWrite (0xD3);	/* Turn speaker-output off */

      __dpmi_set_protected_mode_interrupt_vector (endOfDMAInterruptVector,
						  &oldHandler);

      outportb (0x21, pic1Default);
      outportb (0xA1, pic2Default);

      if ((bufferFormat & SB_STEREO) && (sb_info.dspVersion < 0x0400))
	{
	  outportb (sb_info.reset - 2, 0x0E);
	  outportb (sb_info.reset - 1, mixerDefault);
	}

      _go32_dpmi_free_iret_wrapper (&wrapper);
      __dpmi_free_dos_memory (dmaBufferSelector[0]);
      __dpmi_free_dos_memory (dmaBufferSelector[1]);
      __dpmi_free_dos_memory (dmaBufferSelector[2]);
      __dpmi_free_dos_memory (dmaBufferSelector[3]);
      driverInstalled = 0;
    }
}
