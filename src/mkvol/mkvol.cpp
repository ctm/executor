/* Copyright 1994, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_mkvol[] =
"$Id: mkvol.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#undef _DARWIN_NO_64_BIT_INODE

/* #include "rsys/common.h" */
#if defined (__MINGW32__)
#define CYGWIN32
#endif

#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#if defined(MACOSX_)
#include <libc.h>
#endif

#if defined(__BORLANDC__)
#include <io.h>
#define write _write		/* Yahoo! "_" needed for "binary" output */
#endif

#include "mkvol_internal.h"
#include "mkvol.h"

#if !defined(PRIVATE)
#define PRIVATE static
#endif

#if !defined(noErr)
#define noErr 0
#endif

#if !defined(ioErr)
#define ioErr (-36)
#endif

typedef short int INTEGER;

typedef int LONGINT;
typedef int int32;

#ifndef TRUE
typedef enum { FALSE, TRUE } boolean_t;
#endif

#include "rsys/parsenum.h"

using namespace Executor;


PRIVATE bool write_zeros = false;

static inline void
my_bzero (void *ptr, size_t nbytes)
{
  memset (ptr, 0, nbytes);
}

/*
 * NOTE: we can't inherit PACKED from our mac stuff because we actually
 *   have to let something sit on an odd address, which means we
 *   need to use __attribute__((packed)) even on the 68k, but
 *   normally we have PACKED mean nothing special on the 68k
 */

#if defined(__GNUC__)
#undef PACKED
#define PACKED  __attribute__((packed))
#endif /* defined(__GNUC__) */

#define SECSIZE 512

enum
  {
    EX_SUCCESS,
    EX_NARGS,
    EX_NEGARG,
    EX_LONGNAME,
    EX_CANTOPEN,
    EX_BADSIZE,
    EX_EXISTS,
    EX_WRITE_ERROR,
    EX_WE_ARE_BROKEN,
    EX_BAD_USER_INPUT,
    EX_BAD_HFV_NAME,
  };

typedef struct
  {
    write_funcp_t writefuncp;
    int user_arg;
    unsigned long time;
    const char *volume_name;
    int32 nsecs_left;
    int32 nsecs_in_map;
    int32 nsecs_in_alblock;
    int32 nalblocks_in_tree;
    int32 nalblocks_in_desktop;
  }
info_t;

/*
 * write_startup writes two sectors of zeros since we're not creating a
 * bootable volume.
 */

#define WRITE_AND_RETURN_ERROR(infop, buf, length)			\
if ((infop)->writefuncp ((infop)->user_arg, (buf), length) != length)	\
  return ioErr

PRIVATE int
write_startup (info_t * infop)	/* 0 - 1023 */
{
  char buf[2 * SECSIZE];

  assert (infop->nsecs_left >= 2);
  my_bzero (buf, sizeof (buf));
  WRITE_AND_RETURN_ERROR (infop, buf, sizeof (buf));
  infop->nsecs_left -= 2;
  return noErr;
}

#define NMAPBITS  (SECSIZE * 8)

static union
  {
    char buf[SECSIZE];
    volumeinfo vi;
  }
bufu;

PRIVATE int
write_volume_info (info_t * infop)
{
  unsigned short nalblks;

  assert (infop->nsecs_left >= 1);
  assert (sizeof (bufu) == SECSIZE);
  my_bzero (&bufu, sizeof (bufu));

  infop->nsecs_in_alblock = (infop->nsecs_left + (1L << 16) - 1) / (1L << 16);
/*
 * the -3 below is to take into account this sector and two reserved sectors
 * at the end of the medium.
 */
  nalblks = ((infop->nsecs_left - 3 + infop->nsecs_in_alblock - 1)
	     / infop->nsecs_in_alblock);
  infop->nsecs_in_map = (int32)(((long) nalblks + NMAPBITS - 1) / NMAPBITS);
  nalblks = (infop->nsecs_left - infop->nsecs_in_map - 3
	     + infop->nsecs_in_alblock - 1) / infop->nsecs_in_alblock;
  infop->nalblocks_in_tree = nalblks / 128;	/* TODO: find proper mapping */
  if (infop->nalblocks_in_tree < 12)
    infop->nalblocks_in_tree = 12;

/*
 * NOTE:  For a couple new filesystems I tested, the number of allocation
 *    blocks that were used for "Desktop" was the number in the b-tree
 *    plus one!
 */
  infop->nalblocks_in_desktop = infop->nalblocks_in_tree + 1;

  bufu.vi.drSigWord = CWC (0x4244);
  bufu.vi.drCrDate = CL (infop->time);
  bufu.vi.drLsMod = CL (infop->time);
  bufu.vi.drAtrb = CWC (0x100);	/* clean unmount */
  bufu.vi.drNmFls = CWC (1);
  bufu.vi.drVBMSt = CWC (3);
  bufu.vi.drAllocPtr = CWC (2 * infop->nalblocks_in_tree);	/* NOTE:
			It is goofy to not count infop->nalblocks_in_desktop,
			but that's the way it goes */
  bufu.vi.drNmAlBlks = CW (nalblks);
  bufu.vi.drAlBlkSiz = CL (infop->nsecs_in_alblock * SECSIZE);
  bufu.vi.drClpSiz = CL (4 * CL (bufu.vi.drAlBlkSiz));
  bufu.vi.drAlBlSt = CW (CW (bufu.vi.drVBMSt) + infop->nsecs_in_map);
  bufu.vi.drNxtCNID = CLC (17);
  bufu.vi.drFreeBks = CW (nalblks - 2 * infop->nalblocks_in_tree -
			  infop->nalblocks_in_desktop);
  bufu.vi.drVN[0] = strlen (infop->volume_name);
  memcpy (bufu.vi.drVN + 1, infop->volume_name,
	  (unsigned char) bufu.vi.drVN[0]);
  bufu.vi.drVolBkUp = CLC (0);
  bufu.vi.drVSeqNum = CWC (0);
  bufu.vi.drWrCnt = CLC (0);
  bufu.vi.drXTClpSiz = CL (infop->nalblocks_in_tree * infop->nsecs_in_alblock
			   * SECSIZE);
  bufu.vi.drCTClpSiz = CL (infop->nalblocks_in_tree * infop->nsecs_in_alblock
			   * SECSIZE);
  bufu.vi.drNmRtDirs = CWC (0);
  bufu.vi.drFilCnt = CLC (1);
  bufu.vi.drDirCnt = CLC (0);
/*  bufu.vi.drFndrInfo = zero */
  bufu.vi.drVCSize = CWC (0);
  bufu.vi.drVCBMSize = CWC (0);
  bufu.vi.drCtlCSize = CWC (0);
  bufu.vi.drXTFlSize = bufu.vi.drXTClpSiz;
  bufu.vi.drXTExtRec[0].blockstart = CWC (0);
  bufu.vi.drXTExtRec[0].blockcount = CW (infop->nalblocks_in_tree);
  bufu.vi.drCTFlSize = bufu.vi.drCTClpSiz;
  bufu.vi.drCTExtRec[0].blockstart = CW (infop->nalblocks_in_tree);
  bufu.vi.drCTExtRec[0].blockcount = CW (infop->nalblocks_in_tree);

  WRITE_AND_RETURN_ERROR (infop, &bufu, sizeof (bufu));
  infop->nsecs_left -= 1;
  return noErr;
}

PRIVATE int
write_volume_bitmap (info_t * infop)
{
  int32 nalblocks_in_use;
  unsigned char buf[SECSIZE], *p;
  int32 i;

  assert (infop->nsecs_left >= infop->nsecs_in_map);
  nalblocks_in_use = infop->nalblocks_in_tree * 2 +
    infop->nalblocks_in_desktop;
  my_bzero (buf, sizeof (buf));
  for (i = nalblocks_in_use / 8, p = buf; --i >= 0;)
    *p++ = 0xff;
  *p = 0xff << (8 - nalblocks_in_use % 8);
  WRITE_AND_RETURN_ERROR (infop, buf, sizeof (buf));

  if ((i = infop->nsecs_in_map - 1) > 0)
    {
      my_bzero (buf, sizeof (buf));
      while (--i >= 0)
	WRITE_AND_RETURN_ERROR (infop, buf, sizeof (buf));
    }
  infop->nsecs_left -= infop->nsecs_in_map;
  return noErr;
}

typedef enum
  {
    extent, catalog
  } btree_enum_t;

PRIVATE int
fill_btblock0 (info_t * infop, btree_enum_t btree_enum)
{
  btblock0 bt;

  assert (infop->nsecs_left >= 1);
  assert (sizeof (bt) == SECSIZE);
  my_bzero (&bt, sizeof (bt));
  bt.type = CBC (01);
  bt.dummy = CBC (0);
  bt.hesthreejim = CWC (3);
  bt.btnodesize = CWC (512);
  bt.nnodes = CL (infop->nalblocks_in_tree * infop->nsecs_in_alblock);

  if (CL (bt.nnodes) > 2048)
    {
#if defined (MKVOL_PROGRAM)
      fprintf (stderr, "\n"
	       "makehfv is broken and can't handle an HFV this large.\n"
	       "makehfv should be fixed soon (128m max for now).\n");
      exit (-EX_WE_ARE_BROKEN);
#else
      abort ();
#endif
    }

  bt.nfreenodes = CL (CL (bt.nnodes) - 1);
  bt.unknown2[0] = CLC (0x01f800f8);
  bt.unknown2[1] = CLC (0x0078000e);

  switch (btree_enum)
    {
    case extent:
      bt.flink = CLC (0);
      bt.blink = CLC (0);
      bt.height = CLC (0);
      bt.root = CLC (0);
      bt.numentries = CLC (0);
      bt.firstleaf = CLC (0);
      bt.lastleaf = CLC (0);
      bt.indexkeylen = CWC (7);
      bt.map[0] = 0x80;
      break;
    case catalog:
      bt.flink = CLC (0);
      bt.blink = CLC (0);
      bt.height = CLC (1);
      bt.root = CLC (1);
      bt.numentries = CLC (3);
      bt.firstleaf = CLC (1);
      bt.lastleaf = CLC (1);
      bt.indexkeylen = CWC (37);
      bt.map[0] = 0xC0;
      bt.nfreenodes = CL (CL (bt.nfreenodes) - 1);
      break;
    default:
      assert (0);
      break;
    }
  WRITE_AND_RETURN_ERROR (infop, &bt, sizeof (bt));
  return noErr;
}

PRIVATE int
write_extents (info_t * infop)
{
  char buf[SECSIZE];
  int32 i, j;

  assert (infop->nsecs_left >=
	  infop->nalblocks_in_tree * infop->nsecs_in_alblock);
  fill_btblock0 (infop, extent);
  my_bzero (buf, sizeof (buf));
  for (i = infop->nsecs_in_alblock - 1; --i >= 0;)
    WRITE_AND_RETURN_ERROR (infop, buf, sizeof (buf));
  for (i = infop->nalblocks_in_tree - 1; --i >= 0;)
    for (j = infop->nsecs_in_alblock; --j >= 0;)
      WRITE_AND_RETURN_ERROR (infop, buf, sizeof (buf));
  infop->nsecs_left -= infop->nalblocks_in_tree * infop->nsecs_in_alblock;
  return noErr;
}

#define DESKTOP "Desktop"

PRIVATE void 
set_key_len (catkey * keyp)
{
  keyp->ckrKeyLen = (1 + sizeof (LONGINT) + 1 + keyp->ckrCName[0]) | 1;
}

#if !defined(EVENUP)
#define EVENUP(x) ((void *) (((long) (x) + 1) & ~1))
#endif /* !defined(EVENUP) */

PRIVATE int
write_catalog (info_t * infop)
{
  char buf[SECSIZE];
  btnode *btp;
  catkey *catkeyp;
  unsigned short *offsets;
  directoryrec *dirp;
  threadrec *threadp;
  filerec *filep;
  int32 j, remaining_alblocks_in_tree;

  assert (infop->nsecs_left >=
	  infop->nalblocks_in_tree * infop->nsecs_in_alblock);
  fill_btblock0 (infop, catalog);

  /* we use a negative offset */
  offsets = (unsigned short *) (buf + sizeof (buf));

  btp = (btnode *) buf;
  btp->ndFLink = CLC (0);
  btp->ndBLink = CLC (0);
  btp->ndType = CBC (leafnode);
  btp->ndLevel = CBC (1);
  btp->ndNRecs = CWC (3);

  catkeyp = (catkey *) (btp + 1);
  catkeyp->ckrResrv1 = CBC (0);
  catkeyp->ckrParID = CLC (1);
  catkeyp->ckrCName[0] = strlen (infop->volume_name);
  memcpy (catkeyp->ckrCName + 1, infop->volume_name,
	  (unsigned char) catkeyp->ckrCName[0]);
  set_key_len (catkeyp);

  dirp = (directoryrec*)EVENUP ((char *) catkeyp + catkeyp->ckrKeyLen);
  dirp->cdrType = CBC (DIRTYPE);
  dirp->cdrResrv2 = CBC (0);
  dirp->dirFlags = CWC (0);
  dirp->dirVal = CWC (1);
  dirp->dirDirID = CLC (2);
  dirp->dirCrDat = CL (infop->time);
  dirp->dirMdDat = CL (infop->time);
  dirp->dirBkDat = CLC (0);
/*
 * Leave dirUsrInfo, dirFndrInfo and dirResrv zero for now
 */
  offsets[-1] = CW ((char *) catkeyp - buf);

  catkeyp = (catkey *) (dirp + 1);
  catkeyp->ckrResrv1 = CBC (0);
  catkeyp->ckrParID = CLC (2);
  catkeyp->ckrCName[0] = 0;
  set_key_len (catkeyp);

  threadp = (threadrec*)EVENUP ((char *) catkeyp + catkeyp->ckrKeyLen);
  threadp->cdrType = CBC (THREADTYPE);
  threadp->cdrResrv2 = CBC (0);
  threadp->thdParID = CLC (1);
  threadp->thdCName[0] = strlen (infop->volume_name);
  memcpy (threadp->thdCName + 1, infop->volume_name,
	  (unsigned char) threadp->thdCName[0]);

  offsets[-2] = CW ((char *) catkeyp - buf);

  catkeyp = (catkey *) (threadp + 1);
  catkeyp->ckrResrv1 = CBC (0);
  catkeyp->ckrParID = CLC (2);
  catkeyp->ckrCName[0] = strlen (DESKTOP);
  memcpy (catkeyp->ckrCName + 1, DESKTOP,
	  (unsigned char) catkeyp->ckrCName[0]);
  set_key_len (catkeyp);

  filep = (filerec*)EVENUP ((char *) catkeyp + catkeyp->ckrKeyLen);
  filep->cdrType = CBC (FILETYPE);
  filep->cdrResrv2 = CB (0);
  filep->filFlags = CBC (0);
  filep->filTyp = CBC (0);
  filep->filUsrWds.fdType = CL (TICK ("FNDR"));
  filep->filUsrWds.fdCreator = CL (TICK ("ERIK"));
  filep->filUsrWds.fdFlags = CWC (fInvisible);
  filep->filFlNum = CLC (16);
  filep->filStBlk = CWC (0);
  filep->filLgLen = CLC (0);
  filep->filPyLen = CLC (0);
  filep->filRStBlk = CWC (0);
  filep->filRLgLen = CLC (321);
  filep->filRPyLen = CLC (infop->nalblocks_in_desktop
			  * infop->nsecs_in_alblock * SECSIZE);
  filep->filCrDat = CL (infop->time);
  filep->filMdDat = CL (infop->time);
  filep->filBkDat = CLC (0);
/*  filep->filFndrInfo is not set up */
  filep->filClpSize = CLC (0);
  filep->filExtRec[0].blockstart = CWC (0);
  filep->filExtRec[0].blockcount = CWC (0);
  filep->filRExtRec[0].blockstart = CW (infop->nalblocks_in_tree * 2);
  filep->filRExtRec[0].blockcount = CW (infop->nalblocks_in_desktop);

  offsets[-3] = CW ((char *) catkeyp - buf);
  offsets[-4] = CW ((char *) (filep + 1) - buf);
  WRITE_AND_RETURN_ERROR (infop, buf, sizeof (buf));

  my_bzero (buf, sizeof (buf));
  if (infop->nsecs_in_alblock > 1)
    {
      for (j = infop->nsecs_in_alblock - 2; --j >= 0;)
	WRITE_AND_RETURN_ERROR (infop, buf, sizeof (buf));
      remaining_alblocks_in_tree = infop->nalblocks_in_tree - 1;
    }
  else
    remaining_alblocks_in_tree = infop->nalblocks_in_tree - 2;

  while (--remaining_alblocks_in_tree >= 0)
    for (j = infop->nsecs_in_alblock; --j >= 0;)
      WRITE_AND_RETURN_ERROR (infop, buf, sizeof (buf));
  infop->nsecs_left -= infop->nalblocks_in_tree * infop->nsecs_in_alblock;
  return noErr;
}

#define STR_ID    0
#define STR_NAME  "\012Finder 1.0"

/*
 * NOTE: the "-1" below is because both the byte count and the null zero
 *   are included in the sizeof() value, so we have to decrement to
 *   discount the null.  The null won't actually be in the buffer
 *   because we won't give enough room.
 */

#define DATLEN    (sizeof(LONGINT) + sizeof(STR_NAME) - 1)
#define MAPLEN    (sizeof(map_t))

PRIVATE int
write_desktop (info_t * infop)
{
#pragma pack(push, 2)
  typedef struct
  {
    char zeros[22];
    INTEGER fileattrs;
    INTEGER typeoff;
    INTEGER nameoff;
    INTEGER ntypesminus1;
    char restype1[4];
    INTEGER ntype1minus1;
    INTEGER reflistoff1;
    INTEGER resid1;
    INTEGER resnamoff1;
    char resattr1;
    unsigned char resdoff1[3];
    LONGINT reszero1;
  } map_t;
  
  typedef struct
  {
    LONGINT datoff;
    LONGINT mapoff;
    LONGINT datlen;
    LONGINT maplen;
    char sysuse[112];
    char appluse[128];
    LONGINT datalen1;
    char data1[DATLEN - sizeof (LONGINT)];
    map_t map;
  }
  res_data_t;
  
  const static struct {
    res_data_t data;
    char filler[SECSIZE - sizeof (res_data_t)];
  } buf = {
    {
      CLC (256),
      CLC (256 + DATLEN),
      CLC (DATLEN),
      CLC (MAPLEN),
      {
        0, 0, 0, /* ... */
      }
      ,
      {
        0, 0, 0, /* ... */
      }
      ,
      CLC (DATLEN - sizeof (LONGINT)),
      //STR_NAME,
      { 10, 'F','i','n','d','e','r',' ','1','.','0'},
      {
        {
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        }
        ,
        0x0000,
        CWC (22 + 2 + 2 + 2),	/* zeros ... nameoff */
        CWC (MAPLEN),
        CWC (1 - 1),
        { 'S', 'T', 'R', ' ' },
        CWC (1 - 1),
        CWC (2 + 4 + 2 + 2),	/* ntypesminus1 ... reflistoff1 */
        CWC (STR_ID),
        CWC (-1),		/* no name */
#if !defined(resPreload)
#define resPreload  4
#endif
        CBC (resPreload),
        { 0, 0, 0 },
        0,
      }
      ,
    }
    ,
  };
  assert (infop->nsecs_left >= 1);
  WRITE_AND_RETURN_ERROR (infop, &buf, sizeof (buf));
  --infop->nsecs_left;
  return noErr;
#pragma pack(pop)
}

PRIVATE int
write_rest (info_t * infop)
{
  char buf[SECSIZE];

  assert (infop->nsecs_left >= 2);
  my_bzero (buf, sizeof (buf));

  if (!write_zeros && infop->writefuncp == (write_funcp_t) write)
    {
      int32 blocks_to_skip;

      blocks_to_skip = infop->nsecs_left - 2;
      if (blocks_to_skip > 0)
	if (lseek (infop->user_arg, blocks_to_skip * sizeof (buf), SEEK_CUR) 
	    == -1)
	  return ioErr;
    }
  else
    {
      while (--infop->nsecs_left >= 2)
	WRITE_AND_RETURN_ERROR (infop, buf, sizeof (buf));
    }
  WRITE_AND_RETURN_ERROR (infop, &bufu, sizeof (bufu));
  WRITE_AND_RETURN_ERROR (infop, buf, sizeof (buf));
  return noErr;
}

#if !defined(U70MINUSM04)
#define U70MINUSM04     2082844800L
#endif /* !defined U70MINUSM04 */

#define SECSINMINUTE  60L
#define SECSINHOUR  (60L * SECSINMINUTE)
#define SECSINDAY (24L * SECSINHOUR)
#define SECSINYEAR  (365L * SECSINDAY)

int
format_disk(unsigned long timevar, const char *volumename, int nsecs,
	    write_funcp_t writefuncp, int user_arg)
{
  info_t info;
  int retval;

  info.writefuncp = writefuncp;
  info.user_arg = user_arg;
  info.time = timevar;
  info.volume_name = volumename;
  info.nsecs_left = nsecs;

#if !defined (MKVOL_PROGRAM)
#define NOTE(x)
#else

#define NOTE(x)					\
  do						\
    {						\
      fprintf (stderr, "%s", (x));		\
      fflush (stderr);				\
    }						\
  while (0)

#endif

#define GOTO_DONE_IF_ERROR(var, val)  if (((var) = (val))) goto DONE

  NOTE ("Writing startup");
  GOTO_DONE_IF_ERROR (retval, write_startup (&info));
  NOTE (" info");
  GOTO_DONE_IF_ERROR (retval, write_volume_info (&info));
  NOTE (" bitmap");
  GOTO_DONE_IF_ERROR (retval, write_volume_bitmap (&info));
  NOTE (" extents");
  GOTO_DONE_IF_ERROR (retval, write_extents (&info));
  NOTE (" catalog");
  GOTO_DONE_IF_ERROR (retval, write_catalog (&info));
  NOTE (" desktop");
  GOTO_DONE_IF_ERROR (retval, write_desktop (&info));
  NOTE (" the rest");
  GOTO_DONE_IF_ERROR (retval, write_rest (&info));
  NOTE ("\nDone.\n");
 DONE:
  return retval;
}

#if defined(MKVOL_PROGRAM)

#include <ctype.h>

//#define DEFAULT_SUFFIX ".hfv"
#define DEFAULT_SUFFIX ".img"

PRIVATE int
mixed_case_match (const char *str1, const char *str2)
{
  int retval;

  while (*str1 && *str2 && tolower (*str1) == tolower (*str2))
    {
      ++str1;
      ++str2;
    }
  retval = !*str1 && !*str2;
  return retval;
}

PRIVATE void
adjust_hfv_name (char **namepp)
{
  char *namep = *namepp, *suffixp;
  size_t name_len = strlen(namep);
  FILE *fp;

  if (name_len >= 4) {
      suffixp = namep + name_len - 4;
      if (mixed_case_match (suffixp, DEFAULT_SUFFIX))
	memcpy (suffixp, DEFAULT_SUFFIX, strlen (DEFAULT_SUFFIX));
    }
#if !defined (LETGCCWAIL)
  else
    suffixp = 0;
#endif

  if (name_len < 4 || strcmp (suffixp, DEFAULT_SUFFIX) != 0)
    {
      char *new_namep;

      new_namep = (char*)malloc (name_len + 4 + 1);
      sprintf (new_namep, "%s%s", namep, DEFAULT_SUFFIX);
      fprintf (stderr, "Adding \"" DEFAULT_SUFFIX "\" to rename \"%s\" to \"%s\"\n",
	       namep, new_namep);
      namep = new_namep;
    }
  fp = fopen (namep, "rb");
  if (fp)
    {
      fclose (fp);
      fprintf (stderr, "File \"%s\" already exists and will not be modified\n",
	       namep);
      exit (-EX_EXISTS);
    }
  *namepp = namep;
}

PRIVATE const char *magic_file_to_delete_because_at_exit_is_flawed;

PRIVATE void
cleanup (void)
{
  if (magic_file_to_delete_because_at_exit_is_flawed)
    unlink (magic_file_to_delete_because_at_exit_is_flawed);
}

/*
 * Look for a switch.  If found, set a value and remove the switch from
 * the argument list.
 */

PRIVATE boolean_t
find_and_remove_switch_p (const char *switch_name, int *argcp, char *argv[])
{
  int in, out;
  int start_argc;
  boolean_t present_p;

  present_p = FALSE;
  start_argc = *argcp;
  for (out = in = 1; in < start_argc; in++)
    {
      argv[out] = argv[in];
      if (strcmp (argv[in], switch_name) == 0)
	{
	  present_p = TRUE;
	  --*argcp;
	}
      else
	++out;
    }
  argv[out] = argv[in];
  return present_p;
}

PRIVATE boolean_t
check_hfv_name (const char *hfv_name)
{
  boolean_t success_p;
  if (strlen (hfv_name) == 0)
    {
      fprintf (stderr, "You can't have an empty file name!\n");
      success_p = FALSE;
    }
  else
    {
      success_p = TRUE;
    }
  return success_p;
}

PRIVATE boolean_t
check_volume_name (const char *volume_name)
{
  size_t len = strlen (volume_name);
  boolean_t success_p;

  if (len == 0)
    {
      fprintf (stderr, "You can't have an empty volume name!\n");
      success_p = FALSE;
    }
  else if (len > 27)
    {
      fprintf (stderr, "Volume names can be no longer than 27 characters.\n");
      success_p = FALSE;
    }
  else if (strchr (volume_name, ':'))
    {
      fprintf (stderr, "Volume names may not contain colons.\n");
      success_p = FALSE;
    }
  else
    {
      success_p = TRUE;
    }

  return success_p;
}

PRIVATE boolean_t
check_volume_size (const char *volume_size_string)
{
  boolean_t success_p;
  int32 nbytes;

  if (!parse_number (volume_size_string, &nbytes, SECSIZE))
    {
      fprintf (stderr, "Malformed volume size.\n");
      success_p = FALSE;
    }
  else if (nbytes / SECSIZE < 100)	/* need at least 100 sectors */
    {
      fprintf (stderr, "The specified volume is too small.");
      if (volume_size_string[0])
	{
	  char last_char;
	  last_char = volume_size_string[strlen (volume_size_string) - 1];
	  if (last_char != 'k' && last_char != 'K'
	      && last_char != 'm' && last_char != 'M')
	    fprintf (stderr,
		     "  Remember to add a K or M suffix for\n"
		     "kilobytes or megabytes, respectively.");
	}
      putc ('\n', stderr);

      success_p = FALSE;
    }
  else
    {
      success_p = TRUE;
    }

  return success_p;
}

static void
cleanup_string (char *s)
{
  size_t len;

  len = strlen (s);

  /* Nuke leading whitespace */
  while (isspace (s[0]))
    {
      memmove (s, s + 1, len);
      --len;
    }

  /* Nuke trailing whitespace */
  while (len > 0 && isspace (s[len - 1]))
    s[--len] = '\0';
}

PRIVATE boolean_t
read_parameter (const char *prompt, char **s,
		boolean_t (*verify_func) (const char *))
{
  char buf[2048];
  boolean_t success_p;

  success_p = FALSE;
  do
    {
      fputs (prompt, stdout);
      fflush (stdout);
      if (!fgets (buf, sizeof buf - 1, stdin))
	goto done;
      cleanup_string (buf);
    }
  while (!verify_func (buf));
  *s = strcpy ((char*)malloc (strlen (buf) + 1), buf);
  success_p = TRUE;

 done:
  return success_p;
}

#if defined (__MSDOS__)
#define OS_NAME "DOS"
#elif defined (__linux__)
#define OS_NAME "Linux"
#elif defined (__NeXT__)
#define OS_NAME "NEXTSTEP"
#elif defined (MACOSX_)
#define OS_NAME "Mac OS X"
#endif

PRIVATE boolean_t
prompt_for_parameters (char **hfv_name, char **volume_name, char **volume_size)
{
  boolean_t success_p;

  /* Default to an error string to avoid uninitialized memory bugs. */
  *hfv_name = *volume_name = *volume_size = "???error???";
  success_p = FALSE;

printf (
"Makehfv creates \"HFV\" files for use with Executor, the Macintosh emulator.\n"
"%s will see this HFV file as a single %s file, but Executor treats each\n"
"HFV file as a separate disk drive that you can use to store Mac files and\n"
"folders in exactly the same format that Macintosh programs expect.\n\n"
"Makehfv will prompt you for the %s file name that you want to assign to\n"
"the new HFV file.  Makehfv will automatically add \".hfv\" to the name that\n"
"you provide.  Makehfv will also prompt you for the volume name that you\n"
"want Executor to use for the HFV file.  Finally, makehfv will ask you how\n"
"many bytes of your hard disk space you want to devote to this HFV file.\n"
"Usually you'll want to answer with a number followed by \"m\" -- the \"m\"\n"
"stands for megabyte.\n\n"
"For example, to create a 20 megabyte HFV named \"test.hfv\" that will be\n"
"called \"Play\" from within Executor, answer \"test\", \"Play\" and \"20m\".\n\n",
OS_NAME, OS_NAME, OS_NAME);



  /* Read in the HFV name. */
  if (!read_parameter ("Name of the HFV file to create: ",
		       hfv_name, check_hfv_name))
    goto done;

  /* Read in the volume name. */
  if (!read_parameter ("Volume name as seen under Executor: ",
		       volume_name, check_volume_name))
    goto done;

  /* Read in the volume size. */
  if (!read_parameter ("Volume size: ",
		       volume_size, check_volume_size))
    goto done;

  success_p = TRUE;
  
 done:
  return success_p;
}


int
main (int argc, char *argv[])
{
  time_t now;
  struct tm *tmp;
  int32 years, leaps;
  int32 nsecs = 0, nbytes = 0;
  long timevar;
  boolean_t force_p, help_p;
  char *hfv_name, *volume_name, *volume_size;

  write_zeros = find_and_remove_switch_p ("-zeros", &argc, argv);
  force_p = find_and_remove_switch_p ("-force", &argc, argv);
  help_p = find_and_remove_switch_p ("-help", &argc, argv);

  if (help_p || (argc != 1 && argc != 4))
    {
      const char *name;

      name = strrchr (argv[0], '/');
      if (name)
	++name;
      else
	name = argv[0];
      fprintf (stderr,
"Usage: %s [-help] [-force] [-zeros] [hfvname volumename volumesize]\n"
"\tThe volume size is specified in bytes, and may contain an \"M\"\n"
"\tor \"K\" suffix for megabytes or kilobytes.  For example:\n"
"\t\t%s newvol.hfv NewVolume 4.5M\n"
"\twill create a 4.5 megabyte HFV named newvol.hfv.\n"
"\t   The \"-force\" option prevents a \".hfv\" suffix from being "
"appended\n"
"\tto the file name.  The \"-zeros\" option causes all unused disk blocks\n"
"\tto be filled in with zero bytes.  Ordinarily you never need these\n"
"\toptions, although \"-zeros\" can make the HFV take up less space\n"
"\ton compressed filesystems.\n",
	       name, name);
      exit (-EX_NARGS);
    }
  
  if (argc == 4)
    {
      hfv_name    = argv[1];
      volume_name = argv[2];
      volume_size = argv[3];
    }
  else if (!prompt_for_parameters (&hfv_name, &volume_name, &volume_size))
    exit (-EX_BAD_USER_INPUT);

  if (!force_p)
    adjust_hfv_name (&hfv_name);

  if (!check_hfv_name (hfv_name))
    exit (-EX_BAD_HFV_NAME);

  if (!check_volume_name (volume_name))
    exit (-EX_LONGNAME);

  /* Parse in the disk volume size. */
  if (!check_volume_size (volume_size)
      || !parse_number (volume_size, &nbytes, SECSIZE))
    exit (-EX_BADSIZE);

  nsecs = nbytes / SECSIZE;

  magic_file_to_delete_because_at_exit_is_flawed = hfv_name;
  atexit (cleanup);
  if (freopen (hfv_name, "wb", stdout) == NULL)
    {
      char errmsg[2048];
      sprintf (errmsg, "Could not open \"%s\"", hfv_name);
      perror (errmsg);
      exit (-EX_CANTOPEN);
    }
  time (&now);
  tmp = localtime (&now);
  years = tmp->tm_year - 70;
  leaps = (years + 1) / 4;	/* 1973 counts 1972, 1977 counts '76+'72 */
  timevar = years * SECSINYEAR +
    leaps * SECSINDAY +
    tmp->tm_yday * SECSINDAY +
    tmp->tm_hour * SECSINHOUR +
    tmp->tm_min * SECSINMINUTE +
    tmp->tm_sec + U70MINUSM04;
  if (format_disk(timevar, volume_name, nsecs, (write_funcp_t) write, 1)
      != noErr)
    {
      fprintf (stderr, "Error writing HFV.  Perhaps your disk is full.\n");
      exit (-EX_WRITE_ERROR);
    }
  magic_file_to_delete_because_at_exit_is_flawed = 0;
  return 0;
}
#endif /* defined(MKVOL_PROGRAM) */
