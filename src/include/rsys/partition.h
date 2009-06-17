
/*
 * These data structures are from IMV-579 and IMIV-292 (the SCSI manager
 * sections).  They're used for partitioned hard drives.
 *
 * NOTE:  I didn't use INTEGER below, because this .h file is included
 *	  by HFS_XFer.util.c, which doesn't know our Mac stuff
 */

#if !defined(PACKED)
#define PACKED __attribute__((packed))
typedef unsigned long ULONGINT;
#endif

typedef struct PACKED {
  unsigned short pmSig;	/* 0x504D == 'PM' */
  unsigned short pmSigPad;
  ULONGINT pmMapBlkCnt;
  ULONGINT pmPyPartStart;
  ULONGINT pmPartBlkCnt;
  unsigned char pmPartName[32];	/* NUL terminated */
  unsigned char pmPartType[32];	/* NUL terminated */
  ULONGINT pmLgDataStart;
  ULONGINT pmDataCnt;
  ULONGINT pmPartStatus;
  ULONGINT pmLgBootStart;
  ULONGINT pmBootSize;
  ULONGINT pmBootLoad;
  ULONGINT pmBootLoad2;
  ULONGINT pmBootEntry;
  ULONGINT pmBootEntry2;
  ULONGINT pmBootCksum;
  unsigned char pmProcessor[16];	/* NUL terminated */
  unsigned char bootargs[120];	/* IMV-579 says 128, but they probably
					   mean that the total should be 512 */
} partmapentry_t;

#define PARMAPSIG0	'P'
#define PARMAPSIG1	'M'

#define HFSPARTTYPE	"Apple_HFS"

typedef struct PACKED {
  ULONGINT pdStart;
  ULONGINT pdSize;
  ULONGINT pdFSID;
} oldmapentry_t;

#define NOLDENTRIES	42

typedef struct PACKED {
  unsigned short pdSig;	/* 0x5453 == 'TS' */
  oldmapentry_t oldmapentry[NOLDENTRIES];
} oldblock1_t;

#define OLDMAPSIG0	'T'
#define OLDMAPSIG1	'S'

#define PARTOFFSET	1
