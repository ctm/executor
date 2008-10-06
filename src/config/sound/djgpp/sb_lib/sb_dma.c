#include <dpmi.h>
#include <inlines/pc.h>

#include "sbdetect.h"

static int pagePort[8] = {0x87, 0x83, 0x81, 0x82, 0x8F, 0x8B, 0x89, 0x8A};

static void 
topOfFunctions (void)
{
}

void 
sb_dma8bitReadSC (DWORD linearAddressOfBuffer, int length)
{
  int offset, page;
  int port;

  page = linearAddressOfBuffer >> 16;
  offset = linearAddressOfBuffer & 0xFFFF;

  outportb (0x0A, sb_info.DMA | 0x04);	/* Mask DMA channel     */
  outportb (0x0C, 0);		/* Clear byte flip-flop */
  outportb (0x0B, sb_info.DMA | 0x48);	/* Set mode             */

  port = sb_info.DMA << 1;
  outportb (port, offset & 0xFF);	/* Program offset       */
  outportb (port, offset >> 8);

  port += 1;
  outportb (port, length & 0xFF);	/* Program length       */
  outportb (port, length >> 8);

  outportb (pagePort[sb_info.DMA], page);	/* Program page         */

  outportb (0x0A, sb_info.DMA);	/* Clear channel mask   */
}

#ifdef SB_SUPPORT_16_BIT

void 
sb_dma16bitReadSC (DWORD linearAddressOfBuffer, unsigned length)
{
  unsigned offset, page;
  int port;

  page = linearAddressOfBuffer >> 16;
  offset = (linearAddressOfBuffer >> 1) & 0xFFFF;  /* Measured in words    */

  outportb (0xD4, sb_info.DMA16);		/* Mask DMA channel     */
  outportb (0xD8, 0);				/* Clear byte flip-flop */
  outportb (0xD6, (sb_info.DMA16 - 4) | 0x48);	/* Set mode             */

  port = 0xC0 + ((sb_info.DMA16 - 4) * 4);
  outportb (port, offset & 0xFF);		/* Program offset       */
  outportb (port, offset >> 8);

  port += 2;
  outportb (port, length & 0xFF);		/* Program length       */
  outportb (port, length >> 8);

  outportb (pagePort[sb_info.DMA16], page);	/* Program page         */

  outportb (0xD4, sb_info.DMA16 & 0x03);	/* Clear mask           */
}

#endif /* SB_SUPPORT_16_BIT */

static void 
bottomOfFunctions (void)
{
}

int 
sb_dma_module_lock_your_memory (void)
{
  if (!_go32_dpmi_lock_code (topOfFunctions,
			     ((char *) bottomOfFunctions
			      - (char *) topOfFunctions)))
    return !_go32_dpmi_lock_data (pagePort, sizeof pagePort);

  return 0;
}
