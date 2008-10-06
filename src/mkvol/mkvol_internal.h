/* Copyright 1994 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* #if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_mkvol_h[] =
		      "$Id: mkvol_internal.h 63 2004-12-24 18:19:43Z ctm $";
#endif */

#include <stdio.h>
#include <time.h>
#include <assert.h>
/*
 * NOTE: All the info here has been grabbed from rsys/hfs.h.
 *   I don't expect this stuff to change, but you never know ...
 */

#if !defined(_MKVOL_INTERNAL_H_)
#define _MKVOL_INTERNAL_H_

#if defined(__alpha) || defined(i386) || defined(__BORLANDC__)
#define LITTLEENDIAN
#elif defined(mc68000) || defined (powerpc)
#else
#error "I don't know enough about this machine"
#endif

#if defined(__GNUC__)

#if !defined(mc68000)
#define PACKED  __attribute__((packed))
#else /* defined(mc68000) */
#define PACKED
#endif /* defined(mc68000) */

#else /* defined(__GNUC__) */

#if defined(__BORLANDC__)
#define PACKED
#else
#error "Don't know how to handle PACKED"
#endif

#endif /* !defined(__GNUC__) */

#define TICK(str)       (((LONGINT) (unsigned char) str[0] << 24) | \
												 ((LONGINT) (unsigned char) str[1] << 16) | \
												 ((LONGINT) (unsigned char) str[2] <<  8) | \
												 ((LONGINT) (unsigned char) str[3] <<  0))

#if !defined(LITTLEENDIAN)

#define CW(rhs)                 (rhs)
#define CWC(rhs)                (rhs)
#define CL(rhs)                 (rhs)
#define CLC(rhs)                (rhs)
#define CWC(rhs)                (rhs)
#define CB(rhs)     (rhs)
#define CBC(rhs)    (rhs)

#else /* defined(LITTLEENDIAN) */

#define CWCreal(rhs)  \
				( (signed short) ((((rhs) << 8) & 0xFF00) | \
				(((rhs) >> 8) & 0x00FF)))

#define CLCreal(rhs) ( ((((unsigned long) (rhs) & 0x000000FF) << 24) |\
						(((unsigned long) (rhs) & 0x0000FF00) <<  8) |\
			(((unsigned long) (rhs) & 0x00FF0000) >>  8) |\
			(((unsigned long) (rhs) & 0xFF000000) >> 24)) )

#define CW(rhs)  CWCreal(rhs)
#define CWC(rhs)  CWCreal(rhs)
#define CL(rhs)  CLCreal(rhs)
#define CLC(rhs)  CLCreal(rhs)
#define CB(rhs)     (rhs)
#define CBC(rhs)    (rhs)

#endif /* defined(LITTLEENDIAN) */

#define fInvisible      16384

typedef long OSType;

typedef struct {
		short v PACKED;
		short h PACKED;
} Point;


typedef struct {
		OSType fdType       PACKED;
		OSType fdCreator    PACKED;
		unsigned short fdFlags      PACKED;
		Point fdLocation    PACKED;
		unsigned short fdFldr       PACKED;
} FInfo;


typedef struct {
		unsigned short blockstart PACKED;
		unsigned short blockcount PACKED;
} xtntdesc, xtntrec[3]; /* WILL NEED Cx() */

typedef struct {
		unsigned short drSigWord  PACKED; /* 0 */
		long drCrDate PACKED;   /* 2 */
		long drLsMod  PACKED;   /* 6 */
		unsigned short drAtrb PACKED; /* 10 */
		unsigned short drNmFls  PACKED; /* 12 */
		unsigned short drVBMSt  PACKED; /* 14 */
		unsigned short drAllocPtr PACKED; /* 16 */
		unsigned short drNmAlBlks PACKED; /* 18 */
		long drAlBlkSiz PACKED;   /* 20 */
		long drClpSiz PACKED;
		unsigned short drAlBlSt PACKED;
		long drNxtCNID  PACKED;
		unsigned short drFreeBks  PACKED;
		unsigned char drVN[28]  PACKED;
		long drVolBkUp  PACKED;
		unsigned short drVSeqNum  PACKED;
		long drWrCnt  PACKED;
		long drXTClpSiz PACKED;
		long drCTClpSiz PACKED;
		unsigned short drNmRtDirs PACKED;
		long drFilCnt PACKED;
		long drDirCnt PACKED;
		long drFndrInfo[8]  PACKED;
		unsigned short drVCSize PACKED;
		unsigned short drVCBMSize PACKED;
		unsigned short drCtlCSize PACKED;
		long drXTFlSize PACKED;
		xtntrec drXTExtRec  PACKED;
		long drCTFlSize PACKED;
		xtntrec drCTExtRec  PACKED;
} volumeinfo, *volumeinfoPtr, **volumeinfoHandle;

typedef struct {
		long ndFLink  PACKED;
		long ndBLink  PACKED;
		unsigned char ndType  PACKED;
		char ndLevel  PACKED;
		short ndNRecs PACKED;
		unsigned short idunno PACKED;
} btnode;

typedef enum { indexnode, leafnode = 0xFF } btnodetype;

typedef struct {
		unsigned char ckrKeyLen PACKED;
		char ckrResrv1  PACKED;
		long ckrParID PACKED;
		unsigned char ckrCName[32]  PACKED;
} catkey;

typedef struct {
		unsigned char xkrKeyLen PACKED;
		unsigned char xkrFkType PACKED;
		long xkrFNum  PACKED;
		unsigned short xkrFABN  PACKED;
} xtntkey;

typedef union {
		unsigned char keylen;
		catkey catk;
		xtntkey xtntk;
} anykey;

#define FILETYPE    2

typedef struct {
		char cdrType  PACKED;
		char cdrResrv2  PACKED;
		char filFlags PACKED;
		char filTyp PACKED;
		FInfo filUsrWds PACKED;    /* not sure what form */
		long filFlNum PACKED;
		unsigned short filStBlk PACKED; /* I don't think this is used */
		long filLgLen PACKED;
		long filPyLen PACKED;
		unsigned short filRStBlk  PACKED;    /* not used? */
		long filRLgLen  PACKED;
		long filRPyLen  PACKED;
		long filCrDat PACKED;
		long filMdDat PACKED;
		long filBkDat PACKED;
		long filFndrInfo[4] PACKED;
		unsigned short filClpSize PACKED;
		xtntrec filExtRec PACKED;
		xtntrec filRExtRec  PACKED;
		long filResrv PACKED;
} filerec;

#define DIRTYPE 1

typedef struct {
		char cdrType  PACKED;
		char cdrResrv2  PACKED;
		unsigned short dirFlags PACKED;
		unsigned short dirVal PACKED;
		long dirDirID PACKED;
		long dirCrDat PACKED;
		long dirMdDat PACKED;
		long dirBkDat PACKED;
		long dirUsrInfo[4]  PACKED;
		long dirFndrInfo[4] PACKED;
		long dirResrv[4]  PACKED;
} directoryrec;

#define THREADTYPE  3

typedef struct {
		char cdrType  PACKED;
		char cdrResrv2  PACKED;
		char thdResrv[8]  PACKED;
		long thdParID PACKED;
		unsigned char thdCName[32]  PACKED;
} threadrec;

typedef enum { datafork, resourcefork = 0xFF } forktype;

typedef enum { databusy, resourcebusy, eitherbusy } busyconcern_t;

typedef enum { reading, writing } accesstype;

typedef struct {
		long    flink PACKED;              /* 0 */
		long    blink PACKED;              /* 4 */
		unsigned char type  PACKED;         /* 8 */
		unsigned char dummy PACKED;        /* 9 */
		unsigned short   hesthreejim  PACKED;        /* 10 */
		long    height  PACKED;             /* 12 */
		long    root  PACKED;               /* 16 */
		long    numentries  PACKED;         /* 20 */
		long    firstleaf PACKED;          /* 24 */
		long    lastleaf  PACKED;           /* 28 */
		unsigned short   btnodesize PACKED;         /* 32 */
		unsigned short   indexkeylen  PACKED;        /* 34 */
		long    nnodes  PACKED;             /* 36 */
		long    nfreenodes  PACKED;         /* 40 */
		unsigned char reserved[72]  PACKED; /* 44 */
		unsigned char dummy2[132] PACKED;  /* 116 */
		unsigned char map[256]  PACKED;     /* 248 */
		long unknown2[2]  PACKED;           /* 504 */
} btblock0;

#endif
