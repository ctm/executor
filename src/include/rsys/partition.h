
/*
 * These data structures are from IMV-579 and IMIV-292 (the SCSI manager
 * sections).  They're used for partitioned hard drives.
 *
 * NOTE:  I didn't use INTEGER below, because this .h file is included
 *	  by HFS_XFer.util.c, which doesn't know our Mac stuff
 */

namespace Executor
{
#if !defined(PACKED)
#define PACKED __attribute__((packed))
typedef unsigned long ULONGINT;
#endif

struct partmapentry_t
{
    GUEST_STRUCT;
    GUEST<unsigned short> pmSig; /* 0x504D == 'PM' */
    GUEST<unsigned short> pmSigPad;
    GUEST<ULONGINT> pmMapBlkCnt;
    GUEST<ULONGINT> pmPyPartStart;
    GUEST<ULONGINT> pmPartBlkCnt;
    GUEST<unsigned char[32]> pmPartName; /* NUL terminated */
    GUEST<unsigned char[32]> pmPartType; /* NUL terminated */
    GUEST<ULONGINT> pmLgDataStart;
    GUEST<ULONGINT> pmDataCnt;
    GUEST<ULONGINT> pmPartStatus;
    GUEST<ULONGINT> pmLgBootStart;
    GUEST<ULONGINT> pmBootSize;
    GUEST<ULONGINT> pmBootLoad;
    GUEST<ULONGINT> pmBootLoad2;
    GUEST<ULONGINT> pmBootEntry;
    GUEST<ULONGINT> pmBootEntry2;
    GUEST<ULONGINT> pmBootCksum;
    GUEST<unsigned char[16]> pmProcessor; /* NUL terminated */
    GUEST<unsigned char[120]> bootargs; /* IMV-579 says 128, but they probably
					   mean that the total should be 512 */
};

#define PARMAPSIG0 'P'
#define PARMAPSIG1 'M'

#define HFSPARTTYPE "Apple_HFS"

struct oldmapentry_t
{
    GUEST_STRUCT;
    GUEST<ULONGINT> pdStart;
    GUEST<ULONGINT> pdSize;
    GUEST<ULONGINT> pdFSID;
};

#define NOLDENTRIES 42

struct oldblock1_t
{
    GUEST_STRUCT;
    GUEST<unsigned short> pdSig; /* 0x5453 == 'TS' */
    GUEST<oldmapentry_t[NOLDENTRIES]> oldmapentry;
};

#define OLDMAPSIG0 'T'
#define OLDMAPSIG1 'S'

#define PARTOFFSET 1
}
