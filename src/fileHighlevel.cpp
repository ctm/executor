/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* NOTE:  This is the quick hack version that uses old-style calls.
          Eventually we should extract the common functionality and
	  then use that for both.  That will be a little cleaner, a little
	  faster and a little easier to debug, but not enough of any to
	  make it sensible to do that initially.  */


#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_fileHighlevel[] =
	"$Id: fileHighlevel.c 86 2005-05-25 00:47:12Z ctm $";
#endif

#include "rsys/common.h"

#include "MemoryMgr.h"
#include "FileMgr.h"
#include "ProcessMgr.h"

#include "rsys/file.h"
#include "rsys/string.h"
#include "rsys/hfs.h"
#include "rsys/executor.h"

using namespace Executor;
using namespace ByteSwap;

/*
 * extract the last component of a name:
 *
 * aaa:bbb:ccc  -> ccc
 * aaa:bbb:ccc: -> ccc
 */

PRIVATE void
extract_name (Str255 dest, StringPtr source)
{
  int len, new_len;
  Byte *p;
  
  len = source[0];
  if (source[len] == ':') /* ignore trailing ':' */
    --len;
  for (p = source + len; p >= source+1 && *p != ':'; --p)
    ;
  new_len = len - (p - source);
  dest[0] = new_len;
  memmove (dest+1, p+1, new_len);
}

P4 (PUBLIC pascal trap, OSErr, FSMakeFSSpec,
    int16, vRefNum, int32, dir_id,
    Str255, file_name, FSSpecPtr, spec)
{
  Str255 local_file_name;
  OSErr retval;
  HParamBlockRec hpb;

  if (!file_name)
    warning_unexpected("!file_name");

/*
 * Colons make things tricky because they are used both to identify volumes
 * and to delimit directories and sometimes to identify partial paths.  Right
 * now, most of these uses of colon shouldn't cause problems, but there are
 * some issues that need to be checked on a real Mac.  What do all these
 * mean?
 *
 *     dir_id      file_name
 *     310         foo:
 *     310         :foo:
 *     310         foo:bar
 *     310         :foo:bar
 *     310         :foo:bar:
 *
 * what about all those file_names, but with dir_id = 0, dir_id = 1, dir_id = 2
 * and dir_id = a number that isn't a directory id?
 */

  if (pstr_index_after (file_name, ':', 0))
    warning_unexpected ("colon found");

  str255assign (local_file_name, file_name);
  hpb.volumeParam.ioNamePtr = (StringPtr) RM ((Ptr) local_file_name);
  hpb.volumeParam.ioVRefNum = BigEndianValue (vRefNum);
  if (file_name[0])
    hpb.volumeParam.ioVolIndex = CLC (-1);
  else
    hpb.volumeParam.ioVolIndex = CLC (0);

  retval = PBHGetVInfo (&hpb, FALSE);

  if (retval == noErr)
    {
      CInfoPBRec cpb;
      
      str255assign (local_file_name, file_name);
      cpb.hFileInfo.ioNamePtr = (StringPtr) RM ((Ptr) local_file_name);
      cpb.hFileInfo.ioVRefNum = BigEndianValue (vRefNum);
      if (file_name[0])
	cpb.hFileInfo.ioFDirIndex = CWC (0);
      else
	cpb.hFileInfo.ioFDirIndex = CWC (-1);
      cpb.hFileInfo.ioDirID = BigEndianValue (dir_id);
      retval = PBGetCatInfo (&cpb, FALSE);
      if (retval == noErr)
	{
	  spec->vRefNum = hpb.volumeParam.ioVRefNum;
	  spec->parID = cpb.hFileInfo.ioFlParID;
	  extract_name ((StringPtr) spec->name, MR (cpb.hFileInfo.ioNamePtr));
	}
      else if (retval == fnfErr)
	{
	  OSErr err;
	  
	  cpb.hFileInfo.ioNamePtr = (StringPtr)CLC (0);
	  cpb.hFileInfo.ioVRefNum = BigEndianValue (vRefNum);
	  cpb.hFileInfo.ioFDirIndex = CWC (-1);
	  cpb.hFileInfo.ioDirID = BigEndianValue (dir_id);
	  err = PBGetCatInfo (&cpb, FALSE);
	  if (err == noErr)
	    {
	      if (cpb.hFileInfo.ioFlAttrib & ATTRIB_ISADIR)
		{
		  spec->vRefNum = hpb.volumeParam.ioVRefNum;
		  spec->parID = BigEndianValue (dir_id);
		  extract_name ((StringPtr) spec->name, file_name);
		}
	      else
		retval = dupFNErr;
	    }
	}
    }
  return retval;
}

#if 0

typedef struct save_fcb_info_str
{
  filecontrolblock fcb;
  HVCB *vptr;
  struct save_fcb_info_str *next;
  INTEGER refnum; /* zero if this is just info gathered for switching
		     purposes */
}
save_fcb_info_t;

PRIVATE LONGINT
get_file_num (FSSpecPtr fsp)
{
  return 0; /* TODO */
}

/*
 * get_fcb_info squirrels away all the FCB entries that match a particular
 * file.  It then alters them so that we can open files that are already
 * open without trouble.
 */

PRIVATE save_fcb_info_t *
get_fcb_info (FSSpecPtr fsp)
{
  filecontrolblock *fcbp, *efcbp;
  INTEGER total_length, fcb_size;
  save_fcb_info_t *retval;
  char *fcbsptr;
  INTEGER swapped_vrefnum;
  LONGINT swapped_fnum;


  retval = 0;

  swapped_vrefnum = fsp->vRefNum;
  swapped_fnum = BigEndianValue (get_file_num (fsp));

  fcbsptr = (char *) BigEndianValue (FCBSPtr);
  total_length = BigEndianValue(*(short *)fcbsptr);
  fcbp = (filecontrolblock *) ((short *)BigEndianValue(FCBSPtr)+1);
  efcbp = (filecontrolblock *) ((char *)BigEndianValue(FCBSPtr) + total_length);
  fcb_size = BigEndianValue (FSFCBLen);
  for (;fcbp < efcbp; fcbp = (filecontrolblock *) ((char *)fcbp + fcb_size))
    {
      HVCB *vptr;

      vptr = BigEndianValue (fcbp->fcbVPtr);
      if (vptr && vptr->vcbVRefNum == swapped_vrefnum
	  && fcbp->fcbFlNum == swapped_fnum)
	{
	  save_fcb_info_t *newp;

	  newp = malloc (sizeof *newp);
	  newp->refnum = (char *) fcbp - fcbsptr;
	  newp->fcb = *fcbp;
	  newp->next = retval;
	  newp->vptr = fcbp->fcbVPtr;
	  fcbp->fcbVPtr = 0; /* hide this open file */
	  retval = newp;
	}
    }
  return retval;
}

PRIVATE OSErr
exchange_forks (FSSpecPtr src, FSSpecPtr dst, forktype type)
{
  /* TODO */
  return paramErr;
}

PRIVATE void
exchange_fcb (const save_fcb_info_t *dstp, const save_fcb_info_t *srcp)
{
  /* TODO */
}

/*
 * make_fcb_info looks up the info that we need in order to swap fcb info
 * when we only have information about one side
 */

PRIVATE save_fcb_info_t *
make_fcb_info (FSSpecPtr fsp)
{
  save_fcb_info_t *retval;

  retval = malloc (sizeof *retval);
  retval->next = 0;
  retval->vptr = 0; /* not needed */
  retval->refnum = 0; /* means we're not associated with a real FCB */
  return retval;
}

PRIVATE void
exchange_fcbs (FSSpecPtr fs1, const save_fcb_info_t *savefcbp1,
	       FSSpecPtr fs2, const save_fcb_info_t *savefcbp2)
{
  if (savefcbp1 || savefcbp2)
    {
      if (!savefcbp1)
	savefcbp1 = make_fcb_info (fs1);
      else if (!savefcbp2)
	savefcbp2 = make_fcb_info (fs2);
    }
  exchange_fcb (savefcbp1, savefcbp2);
  exchange_fcb (savefcbp2, savefcbp1);
}

PRIVATE void
restore_fcb (const save_fcb_info_t *infop)
{
  char *fcbsptr;

  fcbsptr = (char *) BigEndianValue (FCBSPtr);
  if (infop)
    {
      if (infop->refnum)
	{
	  filecontrolblock *fcbp;

	  fcbp = (filecontrolblock *) (fcbsptr + infop->refnum);
	  fcbp->fcbVPtr = infop->vptr;
	}
      restore_fcb (infop->next);
    }
}

PRIVATE void
release_fcb_info (save_fcb_info_t *infop)
{
  if (infop)
    {
      release_fcb_info (infop->next);
      free (infop);
    }
}

#endif

/*
 * Part of ugly PAUP-specific hack below
 */

PRIVATE void
create_temp_name (Str63 name, int i)
{
  OSErr err;
  ProcessSerialNumber psn;

  err = GetCurrentProcess (&psn);
  if (err == noErr)
    sprintf ((char *) name+1,
	     "%x%x.%x", BigEndianValue (psn.highLongOfPSN), BigEndianValue (psn.lowLongOfPSN), i);
  else
    sprintf ((char *) name+1, "%d.%x", err, i);
  name[0] = strlen ((char *) name+1);
}

/*
 * On real HFS volumes we could do some B-tree manipulation to achieve
 * the correct results, *but* we'd need this code for ufs volumes anyway,
 * so for now we do the same thing on both.
 */

P2 (PUBLIC pascal trap, OSErr, FSpExchangeFiles,
    FSSpecPtr, src, FSSpecPtr, dst)
{
#if 0
  save_fcb_info_t *src_fcb_info, *dst_fcb_info;
  OSErr retval;

  src_fcb_info = get_fcb_info (src);
  dst_fcb_info = get_fcb_info (dst);

  retval = exchange_forks (src, dst, datafork);
  if (retval == noErr)
    {
      retval = exchange_forks (src, dst, resourcefork);
      if (retval != noErr)
	exchange_forks (src, dst, datafork); /* try to put things back
						together */
    }

  if (retval == noErr)
    exchange_fcbs (src, src_fcb_info, dst, dst_fcb_info);
  else
    {
      restore_fcb (src_fcb_info);
      restore_fcb (dst_fcb_info);
    }

  release_fcb_info (src_fcb_info);
  release_fcb_info (dst_fcb_info);

  return retval;
#else
  OSErr retval;

  warning_unimplemented ("poorly implemented");
  if (src->vRefNum != dst->vRefNum)
    retval = diffVolErr;
  else if (ROMlib_creator != TICK("PAUP") || src->parID != dst->parID)
    retval = wrgVolTypeErr;
  else
    {
      /* Evil hack to get PAUP to work -- doesn't bother adjusting FCBs */
      FSSpec tmp_spec;
      int i;
      
      i = 0;
      tmp_spec = *dst;
      do
	{
	  create_temp_name (tmp_spec.name, i++);
	  retval = FSpRename (dst, tmp_spec.name);
	}
      while (retval == dupFNErr);
      if (retval == noErr)
	{
	  retval = FSpRename (src, dst->name);
	  if (retval != noErr)
	    FSpRename (&tmp_spec, dst->name);
	  else
	    {
	      retval = FSpRename (&tmp_spec, src->name);
	      if (retval != noErr)
		{
		  FSpRename (dst, src->name);
		  FSpRename (&tmp_spec, dst->name);
		}
	    }
	} 
    }
  return retval;
#endif
}

typedef OSErrRET (*open_procp) (HParmBlkPtr pb, BOOLEAN sync);

PRIVATE OSErr
open_helper (FSSpecPtr spec, SignedByte perms, int16 *refoutp,
	     open_procp procp)
{
  OSErr retval;
  HParamBlockRec hpb;

  hpb.ioParam.ioVRefNum = spec->vRefNum;
  hpb.fileParam.ioDirID = spec->parID;
  hpb.ioParam.ioNamePtr = (StringPtr) RM ((Ptr) spec->name);
  if (perms == fsWrPerm)
    {
      warning_unexpected (NULL_STRING);
      perms = fsRdWrPerm;
    }
  hpb.ioParam.ioPermssn = perms;
  warning_unimplemented ("poorly implemented ... will try to open drivers");
  retval = procp (&hpb, FALSE);
  if (retval == noErr)
    *refoutp = hpb.ioParam.ioRefNum;
  return retval;
}

P3 (PUBLIC pascal trap, OSErr, FSpOpenDF,
    FSSpecPtr, spec, SignedByte, perms, int16 *, refoutp)
{
  return open_helper (spec, perms, refoutp, PBHOpen);
}

P3 (PUBLIC pascal trap, OSErr, FSpOpenRF,
    FSSpecPtr, spec, SignedByte, perms, int16 *, refoutp)
{
  return open_helper (spec, perms, refoutp, PBHOpenRF);
}

P4 (PUBLIC pascal trap, OSErr, FSpCreate,
    FSSpecPtr, spec, OSType, creator, OSType, file_type,
    ScriptCode, script)
{
  OSErr retval;

  retval = HCreate (BigEndianValue (spec->vRefNum), BigEndianValue (spec->parID), spec->name,
		    creator, file_type);
  return retval;
}

P3 (PUBLIC pascal trap, OSErr, FSpDirCreate,
    FSSpecPtr, spec, ScriptCode, script,
    int32 *, created_dir_id)
{
  OSErr retval;
  HParamBlockRec hpb;

  hpb.ioParam.ioVRefNum = spec->vRefNum;
  hpb.fileParam.ioDirID = spec->parID;
  hpb.ioParam.ioNamePtr = (StringPtr) RM ((Ptr) spec->name);
  retval = PBDirCreate (&hpb, FALSE);
  if (retval == noErr)
    *created_dir_id = hpb.fileParam.ioDirID;
  return retval;
}

P1 (PUBLIC pascal trap, OSErr, FSpDelete,
    FSSpecPtr, spec)
{
  OSErr retval;
  HParamBlockRec hpb;

  hpb.ioParam.ioVRefNum = spec->vRefNum;
  hpb.fileParam.ioDirID = spec->parID;
  hpb.ioParam.ioNamePtr = (StringPtr) RM ((Ptr) spec->name);
  retval = PBHDelete (&hpb, FALSE);
  return retval;
}

P2 (PUBLIC pascal trap, OSErr, FSpGetFInfo,
    FSSpecPtr, spec, FInfo *, fndr_info)
{
  OSErr retval;
  HParamBlockRec hpb;

  hpb.fileParam.ioVRefNum = spec->vRefNum;
  hpb.fileParam.ioDirID = spec->parID;
  hpb.fileParam.ioNamePtr = (StringPtr) RM ((Ptr) spec->name);
  hpb.fileParam.ioFDirIndex = CWC (0);
  retval = PBHGetFInfo (&hpb, FALSE);
  if (retval == noErr)
    *fndr_info = hpb.fileParam.ioFlFndrInfo;
  return retval;
}

P2 (PUBLIC pascal trap, OSErr, FSpSetFInfo,
    FSSpecPtr, spec, FInfo *, fndr_info)
{
  OSErr retval;
  HParamBlockRec hpb;

  warning_unimplemented ("poorly implemented: call of PBHGetFInfo wasteful");
  hpb.fileParam.ioVRefNum = spec->vRefNum;
  hpb.fileParam.ioDirID = spec->parID;
  hpb.fileParam.ioNamePtr = (StringPtr) RM ((Ptr) spec->name);
  hpb.fileParam.ioFDirIndex = CWC (0);
  retval = PBHGetFInfo (&hpb, FALSE);
  if (retval == noErr)
    {
      hpb.fileParam.ioDirID = spec->parID;
      hpb.fileParam.ioFlFndrInfo = *fndr_info;
      retval = PBHSetFInfo (&hpb, FALSE);
    }
  return retval;
}

typedef OSErrRET (*lock_procp) (HParmBlkPtr pb, BOOLEAN async);

PRIVATE OSErr
lock_helper (FSSpecPtr spec, lock_procp procp)
{
  OSErr retval;
  HParamBlockRec hpb;

  hpb.fileParam.ioVRefNum = spec->vRefNum;
  hpb.fileParam.ioDirID = spec->parID;
  hpb.fileParam.ioNamePtr = (StringPtr) RM ((Ptr) spec->name);
  retval = procp (&hpb, FALSE);
  return retval;
}

P1 (PUBLIC pascal trap, OSErr, FSpSetFLock,
    FSSpecPtr, spec)
{
  return lock_helper (spec, PBHSetFLock);
}

P1 (PUBLIC pascal trap, OSErr, FSpRstFLock,
    FSSpecPtr, spec)
{
  return lock_helper (spec, PBHRstFLock);
}

P2 (PUBLIC pascal trap, OSErr, FSpRename,
    FSSpecPtr, spec, Str255, new_name)
{
  OSErr retval;
  HParamBlockRec hpb;

  hpb.fileParam.ioVRefNum = spec->vRefNum;
  hpb.fileParam.ioDirID = spec->parID;
  hpb.fileParam.ioNamePtr = (StringPtr) RM ((Ptr) spec->name);
  hpb.ioParam.ioMisc = RM ((LONGINT) new_name);
  retval = PBHRename (&hpb, FALSE);
  return retval;
}

P2 (PUBLIC pascal trap, OSErr, FSpCatMove,
    FSSpecPtr, src, FSSpecPtr, dst)
{
  OSErr retval;
  CMovePBRec cbr;

  if (src->vRefNum != dst->vRefNum)
    retval = paramErr;
  else
    {
      cbr.ioVRefNum = src->vRefNum;
      cbr.ioDirID = src->parID;
      cbr.ioNamePtr = (StringPtr) RM ((Ptr) src->name);
      cbr.ioNewName = (StringPtr) RM ((Ptr) dst->name);
      cbr.ioNewDirID = dst->parID;
      retval = PBCatMove (&cbr, FALSE);
    }
  return retval;
}

P4 (PUBLIC pascal trap, void, FSpCreateResFile,
    FSSpecPtr, spec, OSType, creator, OSType, file_type,
    ScriptCode, script)
{
  HCreateResFile_helper (BigEndianValue (spec->vRefNum), BigEndianValue (spec->parID),
			 spec->name, creator, file_type, script);
}


P2 (PUBLIC pascal trap, INTEGER, FSpOpenResFile,
    FSSpecPtr, spec, SignedByte, perms)
{
  INTEGER retval;

  retval = HOpenResFile (BigEndianValue (spec->vRefNum), BigEndianValue (spec->parID), spec->name,
			 perms);
  return retval;
}


/* NOTE: the HCreate and HOpenRF are not traps, they're just high level
   calls that are handy to use elsewhere, so they're included here. */

PUBLIC OSErr
Executor::HCreate (INTEGER vref, LONGINT dirid, Str255 name, OSType creator, OSType type)
{
  HParamBlockRec hpb;
  OSErr retval;

  hpb.fileParam.ioNamePtr = RM (name);
  hpb.fileParam.ioVRefNum = BigEndianValue (vref);
  hpb.fileParam.ioDirID = BigEndianValue (dirid);
  retval = PBHCreate (&hpb, FALSE);
  if (retval == noErr)
    {
      hpb.fileParam.ioFDirIndex = CWC (0);
      retval = PBHGetFInfo (&hpb, FALSE);
      if (retval == noErr)
	{
	  hpb.fileParam.ioFlFndrInfo.fdCreator = BigEndianValue (creator);
	  hpb.fileParam.ioFlFndrInfo.fdType = BigEndianValue (type);
	  hpb.fileParam.ioDirID = BigEndianValue (dirid);
	  retval = PBHSetFInfo (&hpb, FALSE);
	}
    }
  return retval;
}

PUBLIC OSErr
Executor::HOpenRF (INTEGER vref, LONGINT dirid, Str255 name, SignedByte perm,
	 INTEGER *refp)
{
  HParamBlockRec hpb;
  OSErr retval;

  hpb.fileParam.ioNamePtr = RM (name);
  hpb.fileParam.ioVRefNum = BigEndianValue (vref);
  hpb.ioParam.ioPermssn = CB (perm);
  hpb.ioParam.ioMisc = CLC (0);
  hpb.fileParam.ioDirID = BigEndianValue (dirid);
  retval = PBHOpenRF (&hpb, FALSE);
  if (retval == noErr)
    *refp = BigEndianValue (hpb.ioParam.ioRefNum);
  return retval;
}
