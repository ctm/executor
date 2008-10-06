
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

typedef struct {
    unsigned short pmSig	PACKED;	/* 0x504D == 'PM' */
    unsigned short pmSigPad	PACKED;
    ULONGINT pmMapBlkCnt	PACKED;
    ULONGINT pmPyPartStart	PACKED;
    ULONGINT pmPartBlkCnt	PACKED;
    unsigned char pmPartName[32]	PACKED;	/* NUL terminated */
    unsigned char pmPartType[32]	PACKED;	/* NUL terminated */
    ULONGINT pmLgDataStart	PACKED;
    ULONGINT pmDataCnt	PACKED;
    ULONGINT pmPartStatus	PACKED;
    ULONGINT pmLgBootStart	PACKED;
    ULONGINT pmBootSize	PACKED;
    ULONGINT pmBootLoad	PACKED;
    ULONGINT pmBootLoad2	PACKED;
    ULONGINT pmBootEntry	PACKED;
    ULONGINT pmBootEntry2	PACKED;
    ULONGINT pmBootCksum	PACKED;
    unsigned char pmProcessor[16]	PACKED;	/* NUL terminated */
    unsigned char bootargs[120]	PACKED;	/* IMV-579 says 128	PACKED, but they probably
					   mean that the total should be 512 */
} partmapentry_t;

#define PARMAPSIG0	'P'
#define PARMAPSIG1	'M'

#define HFSPARTTYPE	"Apple_HFS"

typedef struct {
    ULONGINT pdStart	PACKED;
    ULONGINT pdSize	PACKED;
    ULONGINT pdFSID	PACKED;
} oldmapentry_t;

#define NOLDENTRIES	42

typedef struct {
    unsigned short pdSig	PACKED;	/* 0x5453 == 'TS' */
    oldmapentry_t oldmapentry[NOLDENTRIES]	PACKED;
} oldblock1_t;

#define OLDMAPSIG0	'T'
#define OLDMAPSIG1	'S'

#define PARTOFFSET	1
