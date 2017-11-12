/* Copyright 1994 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include <stdio.h>
#include <time.h>
#include <assert.h>
/*
 * NOTE: All the info here has been grabbed from rsys/hfs.h.
 *   I don't expect this stuff to change, but you never know ...
 */

#if !defined(_MKVOL_INTERNAL_H_)
#define _MKVOL_INTERNAL_H_

#if defined(__alpha) || defined(i386) || defined(__BORLANDC__) || defined(__x86_64__) || defined(__LITTLE_ENDIAN__)
#define LITTLEENDIAN
#elif defined(mc68000) || defined(powerpc) || defined(__ppc__) || defined(__BIG_ENDIAN__)
#else
#error "I don't know enough about this machine"
#endif

#define TICK(str) (((LONGINT)(unsigned char)str[0] << 24) | ((LONGINT)(unsigned char)str[1] << 16) | ((LONGINT)(unsigned char)str[2] << 8) | ((LONGINT)(unsigned char)str[3] << 0))

#if !defined(LITTLEENDIAN)

#define CW(rhs) (rhs)
#define CWC(rhs) (rhs)
#define CL(rhs) (rhs)
#define CLC(rhs) (rhs)
#define CWC(rhs) (rhs)
#define CB(rhs) (rhs)
#define CBC(rhs) (rhs)

#else /* defined(LITTLEENDIAN) */

#define CWCreal(rhs) \
    ((signed short)((((rhs) << 8) & 0xFF00) | (((rhs) >> 8) & 0x00FF)))

#define CLCreal(rhs) (((((unsigned int)(rhs)&0x000000FF) << 24) | (((unsigned int)(rhs)&0x0000FF00) << 8) | (((unsigned int)(rhs)&0x00FF0000) >> 8) | (((unsigned int)(rhs)&0xFF000000) >> 24)))

#define CW(rhs) CWCreal(rhs)
#define CWC(rhs) CWCreal(rhs)
#define CL(rhs) CLCreal(rhs)
#define CLC(rhs) CLCreal(rhs)
#define CB(rhs) (rhs)
#define CBC(rhs) (rhs)

#endif /* defined(LITTLEENDIAN) */

#define fInvisible 16384

#pragma pack(push, 2)

typedef unsigned int OSType;

typedef struct
{
    short v;
    short h;
} Point;

typedef struct
{
    OSType fdType;
    OSType fdCreator;
    unsigned short fdFlags;
    Point fdLocation;
    unsigned short fdFldr;
} FInfo;

typedef struct
{
    unsigned short blockstart;
    unsigned short blockcount;
} xtntdesc, xtntrec[3]; /* WILL NEED Cx() */

typedef struct
{
    unsigned short drSigWord; /* 0 */
    int drCrDate; /* 2 */
    int drLsMod; /* 6 */
    unsigned short drAtrb; /* 10 */
    unsigned short drNmFls; /* 12 */
    unsigned short drVBMSt; /* 14 */
    unsigned short drAllocPtr; /* 16 */
    unsigned short drNmAlBlks; /* 18 */
    int drAlBlkSiz; /* 20 */
    int drClpSiz;
    unsigned short drAlBlSt;
    int drNxtCNID;
    unsigned short drFreeBks;
    unsigned char drVN[28];
    int drVolBkUp;
    unsigned short drVSeqNum;
    int drWrCnt;
    int drXTClpSiz;
    int drCTClpSiz;
    unsigned short drNmRtDirs;
    int drFilCnt;
    int drDirCnt;
    int drFndrInfo[8];
    unsigned short drVCSize;
    unsigned short drVCBMSize;
    unsigned short drCtlCSize;
    int drXTFlSize;
    xtntrec drXTExtRec;
    int drCTFlSize;
    xtntrec drCTExtRec;
} volumeinfo, *volumeinfoPtr, **volumeinfoHandle;

typedef struct
{
    int ndFLink;
    int ndBLink;
    unsigned char ndType;
    char ndLevel;
    short ndNRecs;
    unsigned short idunno;
} btnode;

typedef enum { indexnode,
               leafnode = 0xFF } btnodetype;

typedef struct
{
    unsigned char ckrKeyLen;
    char ckrResrv1;
    int ckrParID;
    unsigned char ckrCName[32];
} catkey;

typedef struct
{
    unsigned char xkrKeyLen;
    unsigned char xkrFkType;
    int xkrFNum;
    unsigned short xkrFABN;
} xtntkey;

typedef union {
    unsigned char keylen;
    catkey catk;
    xtntkey xtntk;
} anykey;

#define FILETYPE 2

typedef struct
{
    char cdrType;
    char cdrResrv2;
    char filFlags;
    char filTyp;
    FInfo filUsrWds; /* not sure what form */
    int filFlNum;
    unsigned short filStBlk; /* I don't think this is used */
    int filLgLen;
    int filPyLen;
    unsigned short filRStBlk; /* not used? */
    int filRLgLen;
    int filRPyLen;
    int filCrDat;
    int filMdDat;
    int filBkDat;
    int filFndrInfo[4];
    unsigned short filClpSize;
    xtntrec filExtRec;
    xtntrec filRExtRec;
    int filResrv;
} filerec;

#define DIRTYPE 1

typedef struct
{
    char cdrType;
    char cdrResrv2;
    unsigned short dirFlags;
    unsigned short dirVal;
    int dirDirID;
    int dirCrDat;
    int dirMdDat;
    int dirBkDat;
    int dirUsrInfo[4];
    int dirFndrInfo[4];
    int dirResrv[4];
} directoryrec;

#define THREADTYPE 3

typedef struct
{
    char cdrType;
    char cdrResrv2;
    char thdResrv[8];
    int thdParID;
    unsigned char thdCName[32];
} threadrec;

typedef enum { datafork,
               resourcefork = 0xFF } forktype;

typedef enum { databusy,
               resourcebusy,
               eitherbusy } busyconcern_t;

typedef enum { reading,
               writing } accesstype;

typedef struct
{
    int flink; /* 0 */
    int blink; /* 4 */
    unsigned char type; /* 8 */
    unsigned char dummy; /* 9 */
    unsigned short hesthreejim; /* 10 */
    int height; /* 12 */
    int root; /* 16 */
    int numentries; /* 20 */
    int firstleaf; /* 24 */
    int lastleaf; /* 28 */
    unsigned short btnodesize; /* 32 */
    unsigned short indexkeylen; /* 34 */
    int nnodes; /* 36 */
    int nfreenodes; /* 40 */
    unsigned char reserved[72]; /* 44 */
    unsigned char dummy2[132]; /* 116 */
    unsigned char map[256]; /* 248 */
    int unknown2[2]; /* 504 */
} btblock0;

#pragma pack(pop)

#endif
