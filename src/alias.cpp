/* Copyright 1994, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_alias[] =
  "$Id: alias.c 88 2005-05-25 03:59:37Z ctm $";
#endif

#include "rsys/common.h"

#include <stdarg.h>

#include "FileMgr.h"
#include "AliasMgr.h"
#include "MemoryMgr.h"
#include "ToolboxUtil.h"

#include "rsys/file.h"
#include "rsys/hfs.h"
#include "rsys/string.h"
#if defined (CYGWIN32)
#include "win_temp.h"
#endif

#include "rsys/alias.h"

#define paramErr	(-50)

using namespace Executor;

/* NOTE: if we want to be more like the Mac, we should have a 'fld#',0
   resource that will have in it: type, four bytes of 0, pascal string,
   potential padding to even things up, type, four bytes of 0, ... */


PRIVATE const char *
find_sub_dir (OSType folderType)
{
  typedef struct
    {
      OSType type;
      const char *name;
    }
  sys_sub_dir_match_t;

  static sys_sub_dir_match_t matches[] =
    {
      { kPrintMonitorDocsFolderType,	"PrintMonitor Documents", },
      { kStartupFolderType,		"Startup Items", },
      { kAppleMenuFolderType,		"Apple Menu Items", },
      { kExtensionFolderType,		"Extensions", },
      { kPreferencesFolderType,		"Preferences", },
      { kControlPanelFolderType,	"Control Panels", },
      { kFontFolderType,		"Fonts", },
    };
  int i;
  const char *retval;
  
  for (i = 0; i < (int) NELEM (matches) && matches[i].type != folderType; ++i)
    ;
  if (i < (int) NELEM (matches))
    retval = matches[i].name;
  else
    retval = 0;
  return retval;
}

PRIVATE OSErr
get_sys_vref_and_dirid (INTEGER *sys_vrefp, LONGINT *sys_diridp)
{
  OSErr err;
  WDPBRec wdp;

  wdp.ioVRefNum = BootDrive;
  wdp.ioWDIndex = CWC (0);
  wdp.ioNamePtr = (StringPtr)CLC (0);
  err = PBGetWDInfo (&wdp, FALSE);
  if (err == noErr)
    {
      *sys_vrefp = CW (wdp.ioWDVRefNum);
      *sys_diridp = CL (wdp.ioWDDirID);
    }
  return err;
}

PRIVATE OSErr
try_to_find (INTEGER vref, const char *str, INTEGER *vrefp, LONGINT *diridp)
{
  OSErr err;
  HVCB *vcbp;

  warning_trace_info ("str = '%s'", str);
  vcbp = ROMlib_vcbbybiggestunixname (str);

  if (!vcbp)
    err = nsvErr;
  else
    {
      VCBExtra *vcbextrap;
      struct stat sbuf;

      vcbextrap = (VCBExtra *) vcbp;
      warning_trace_info ("unixname = '%s'", vcbextrap->unixname);
      if (Ustat (str, &sbuf) < 0)
	{
	  warning_trace_info ("stat failed, errno = %d", errno);
	  err = ROMlib_maperrno ();
	}
      else
	{
	  warning_trace_info ("ino = %ld, ufs.ino = %ld", (long) ST_INO (sbuf),
			      (long) vcbextrap->u.ufs.ino);
	  if (vref != CW (vcbp->vcbVRefNum))
	    err = fnfErr;
	  else
	    {
	      *vrefp = vref;
	      if (ST_INO(sbuf) == vcbextrap->u.ufs.ino)
		*diridp = 2;
	      else
		*diridp = ST_INO(sbuf);
	      err = noErr;
	    }
	}
    }
  warning_trace_info ("err = %d", err);
  return err;
}

PRIVATE OSErr
look_for_volume (const char *vol_name, INTEGER *vrefp, LONGINT *diridp)
{
  OSErr retval;
  ParamBlockRec pbr;
  Str255 pvol_name;

  str255_from_c_string (pvol_name, vol_name);
  pbr.volumeParam.ioNamePtr = RM (&pvol_name[0]);
  pbr.volumeParam.ioVolIndex = CLC (-1);
  pbr.volumeParam.ioVRefNum = CWC (0);
  retval = PBGetVInfo (&pbr, FALSE);
  if (retval == noErr)
    {
      *vrefp = CW (pbr.volumeParam.ioVRefNum);
      *diridp = 2;
    }
  return retval;
}

/*
 * We call this when we haven't been able to find what we want, so we
 * try to construct something ourselves.
 */

PRIVATE OSErr
last_chance_tmp_vref_and_dirid (INTEGER vref, INTEGER *tmp_vrefp,
				LONGINT *tmp_diridp)
{
  OSErr retval;
  HParamBlockRec pb = {0};
  
  pb.volumeParam.ioVRefNum = CW (vref);
  retval = PBHGetVInfo (&pb, FALSE);
  if (retval == noErr) {
	static char *top_level_names[] =
	{
	    "\3tmp",
	    "\4temp",
	};
	int i;
	OSErr err;
	
	*tmp_vrefp = vref;
	
	for (i = 0, err = fnfErr;
		 err != noErr && i < NELEM (top_level_names);
		 ++i)
	{
	  CInfoPBRec hpb;
	  
	  memset (&hpb, 0, sizeof hpb);
	  hpb.dirInfo.ioNamePtr = (StringPtr) RM (top_level_names[i]);
	  hpb.dirInfo.ioVRefNum = pb.volumeParam.ioVRefNum;
	  hpb.dirInfo.ioDrDirID = CLC (2);
	  err = PBGetCatInfo (&hpb, FALSE);
	  if (err == noErr && (hpb.hFileInfo.ioFlAttrib & ATTRIB_ISADIR))
		*tmp_diridp = CL (hpb.dirInfo.ioDrDirID);
	}
	if (err != noErr)
	  *tmp_diridp = CLC (2);
  }
  
  return retval;
}

PRIVATE OSErr
get_tmp_vref_and_dirid (INTEGER vref, INTEGER *tmp_vrefp, LONGINT *tmp_diridp)
{
  int i;
  OSErr retval;
  static const char *guesses[] =
    {

#if !defined (MSDOS) && !defined (CYGWIN32)
      "/tmp",
#else
#if defined (CYGWIN32)
      NULL,
#endif
      "c:/tmp",
      "c:/temp",
      "c:/"
#endif
    };


  {
    static boolean_t been_here_p = FALSE;

    if (!been_here_p)
      {
	int j;

#if defined (CYGWIN32)
	guesses[0] = win_temp ();
#endif	
	
	for (j = 0; j < (int) NELEM (guesses); ++j)
	  {
	    char *p;
	    
	    if (guesses[j])
	      {
		p = (char*)alloca (strlen (guesses[j]) + 1);
		strcpy (p, guesses[j]);
		ROMlib_automount (p);
	      }
	  }
	been_here_p = TRUE;
      }
  }

  retval = look_for_volume ("tmp:", tmp_vrefp, tmp_diridp);

  for (i = 0; retval != noErr && i < (int) NELEM (guesses); ++i)
    retval = try_to_find (vref, guesses[i], tmp_vrefp, tmp_diridp);

  if (retval != noErr)
    retval = last_chance_tmp_vref_and_dirid (vref, tmp_vrefp, tmp_diridp);

  return retval;
}

PRIVATE OSErr
test_directory (INTEGER vref, LONGINT dirid, const char *sub_dirp,
		LONGINT *new_idp)
{
  OSErr err;
  CInfoPBRec cpb;
  Str255 file_name;

  str255_from_c_string (file_name, sub_dirp);
  cpb.hFileInfo.ioNamePtr = (StringPtr) RM ((Ptr) file_name);
  cpb.hFileInfo.ioVRefNum = CW (vref);
  cpb.hFileInfo.ioFDirIndex = CWC (0);
  cpb.hFileInfo.ioDirID = CL (dirid);
  err = PBGetCatInfo (&cpb, FALSE);
  if (err == noErr && !(cpb.hFileInfo.ioFlAttrib & ATTRIB_ISADIR))
    err = dupFNErr;
  if (err == noErr)
    *new_idp = CL (cpb.dirInfo.ioDrDirID);
  return err;
}

PRIVATE OSErr
create_directory (INTEGER sys_vref, LONGINT sys_dirid, const char *sub_dirp,
		  LONGINT *new_idp)
{
  warning_unimplemented (NULL_STRING);
  return paramErr;
}

P5 (PUBLIC pascal trap, OSErr, FindFolder,
    int16, vRefNum, OSType, folderType,
    Boolean, createFolder, int16 *, foundVRefNum, int32 *, foundDirID)
{
  OSErr retval;
  const char *sub_dir;

  sub_dir = find_sub_dir (folderType);
  if (sub_dir)
    {
      INTEGER sys_vref;
      LONGINT sys_dirid, new_id;

      retval = get_sys_vref_and_dirid (&sys_vref, &sys_dirid);
      if (retval == noErr)
	{
	  retval = test_directory (sys_vref, sys_dirid, sub_dir, &new_id);
	  if (retval == fnfErr && createFolder)
	    retval = create_directory (sys_vref, sys_dirid, sub_dir, &new_id);
	  if (retval == noErr)
	    {
	      *foundVRefNum = CW (sys_vref);
	      *foundDirID = CL (new_id);
	    }
	}
    }
  else
    switch (folderType)
      {
      case kSystemFolderType:
	{
	  INTEGER sys_vref;
	  LONGINT sys_dirid;
	
	  retval = get_sys_vref_and_dirid (&sys_vref, &sys_dirid);
	  if (retval == noErr)
	    {
	    /* NOTE: IMVI 9-44 tells us to not create System Folder if it
	       doesn't already exist */
	      *foundVRefNum = CW (sys_vref);
	      *foundDirID = CL (sys_dirid);
	    }
	}
	break;
      case kDesktopFolderType:
      case kTrashFolderType:
      case kWhereToEmptyTrashFolderType:
      case kTemporaryFolderType:
/* These cases aren't properly handled, but they should allow some apps
   to get further */
	{
	  INTEGER tmp_vref;
	  LONGINT tmp_dirid;
	  
	  retval = get_tmp_vref_and_dirid (vRefNum, &tmp_vref, &tmp_dirid);
	  warning_unimplemented ("poorly implemented");
	  if (retval == fnfErr && createFolder)
	    warning_unimplemented ("Didn't attempt to create folder");
	  if (retval == noErr)
	    {
	      *foundVRefNum = CW (tmp_vref);
	      *foundDirID = CL (tmp_dirid);
	    }
	}
	break;
      default:
	warning_unexpected ("unknown folderType");
	retval = fnfErr;
	break;
      }
  return retval;
}

P3 (PUBLIC pascal trap, OSErr, NewAlias,
    FSSpecPtr, fromFile, FSSpecPtr, target,
    AliasHandle *, alias)
{
  OSErr retval;

  *alias = 0;
  warning_unimplemented ("poorly implemented");

  retval = NewAliasMinimal (target, alias);
  return retval;
}

P4 (PUBLIC pascal trap, OSErr, UpdateAlias,
    FSSpecPtr, fromFile, FSSpecPtr, target,
    AliasHandle, alias, Boolean *, wasChanged)
{
  warning_unimplemented (NULL_STRING);
  return paramErr;
}

enum
{
  FULL_PATH_TAG = 0x0002,
  TAIL_TAG = 0x0009,
};

/*
 * ResolveAlias is just a stub so we can recover the fsspecs that are stored
 * in the AppleEvent that is constructed as part of the process of launching
 * another application.  This stub doesn't look at fromFile, doesn't consider
 * the fact that the alias may point to an alias.  It won't work if a full-path
 * alias is supplied either.
 */

P4 (PUBLIC pascal trap, OSErr, ResolveAlias,
    FSSpecPtr, fromFile, AliasHandle, alias,
    FSSpecPtr, target, Boolean *, wasAliased)
{
  OSErr retval;
  alias_head_t *headp; 
  Str255 volname;
  FSSpec fs;
  HParamBlockRec pb;

  warning_unimplemented ("stub for Launch WON'T WORK WITH FULL PATH SPEC");
  retval = noErr;
  headp = (typeof (headp)) STARH (alias);
  str255assign (volname, headp->volumeName);
  fs.parID = headp->ioDirID; /* NOT VALID IF THIS IS A FULL PATH SPEC */
  str255assign (fs.name, headp->fileName); 

  pb.volumeParam.ioNamePtr = (StringPtr) RM ((Ptr) volname);
  pb.volumeParam.ioVolIndex = CLC (-1);
  pb.volumeParam.ioVRefNum = 0;
  retval = PBHGetVInfo (&pb, FALSE);
  if (retval == noErr)
    {
      fs.vRefNum = pb.volumeParam.ioVRefNum;
      *wasAliased = FALSE;
      *target = fs;
    }

  return retval;
}

P4 (PUBLIC pascal trap, OSErr, ResolveAliasFile,
    FSSpecPtr, theSpec, Boolean, resolveAliasChains,
    Boolean *, targetIsFolder, Boolean *, wasAliased)
{
  HParamBlockRec hpb;
  OSErr retval;


  memset (&hpb, 0, sizeof hpb);
  hpb.fileParam.ioNamePtr = (StringPtr) RM ((Ptr) theSpec->name);
  hpb.fileParam.ioDirID = theSpec->parID;
  hpb.fileParam.ioVRefNum = theSpec->vRefNum;
  retval = PBHGetFInfo (&hpb, FALSE);

  if (retval == noErr)
    {
      *targetIsFolder = !!(hpb.fileParam.ioFlAttrib & ATTRIB_ISADIR);
      *wasAliased = FALSE;
    }

  warning_unimplemented ("'%.*s' retval = %d, isFolder = %d", theSpec->name[0],
			 theSpec->name+1, retval, *targetIsFolder);

  return retval;
}

P8 (PUBLIC pascal trap, OSErr, MatchAlias,
    FSSpecPtr, fromFile, int32, rulesMask,
    AliasHandle, alias, int16 *, aliasCount,
    FSSpecArrayPtr, aliasList, Boolean *, needsUpdate,
    AliasFilterProcPtr, aliasFilter,
    Ptr, yourDataPtr)
{
  warning_unimplemented (NULL_STRING);
  return paramErr;
}

P3 (PUBLIC pascal trap, OSErr, GetAliasInfo,
    AliasHandle, alias, AliasTypeInfo, index,
    Str63, theString)
{
  warning_unimplemented (NULL_STRING);
  return paramErr;
}

PRIVATE int
EVENUP (int n)
{
  int retval = n;
	
  if (retval & 1)
    ++retval;
  return retval;
}

#if 0
PRIVATE OSErr
parse2 (AliasHandle ah, const void *addrs[], int count)
{
  OSErr retval;
  Size size;
	
  size = GetHandleSize ((Handle) ah);
  if (size < sizeof (alias_head_t) + sizeof (INTEGER))
    retval = paramErr;
  else if (count < 0)
    retval = paramErr;
  else
    {
      const alias_head_t *headp;
      const INTEGER *partp, *ep;
		
      headp = (alias_head_t *) STARH (ah);
      partp = (INTEGER *) (&headp[1]);
      ep = (INTEGER *) ((char *) headp + MIN (size, CW (headp->length)));
      memset (addrs, 0, count * sizeof addrs[0]);
      for (; partp < ep && *partp != CWC (-1);
	   partp = (INTEGER *) ((char *) partp + EVENUP (4 + CW (partp[1]))))
	{
	  int part;
			
	  part = CW (*partp);
	  if (part < count)
	    addrs[part] = partp + 1;
	}
      retval = *partp == CWC (-1) ? noErr : paramErr;
    }
	
  return retval;
}
#endif

PRIVATE OSErr
decompose_full_path (INTEGER path_len, Ptr fullPath, Str27 volumeName,
		     Str31 fileName)
{
  OSErr retval;
  char *first_colon;
  char *last_colon;
  char *p, *ep;
  int volume_len;
  int file_len;
	
  for (p = (char *) fullPath,
	 ep = p + path_len, first_colon = 0, last_colon = 0;
       p != ep;
       ++p)
    {
      if (*p == ':')
	{
	  if (!first_colon)
	    first_colon = p;
	  last_colon = p;
	}
    }
  if (!first_colon)
    retval = paramErr;
  else
    {
      volume_len = first_colon - (char *) fullPath;
      file_len = ep - last_colon - 1;
      if (volume_len > 27 || file_len > 31)
	retval = paramErr;
      else
	{
	  volumeName[0] = volume_len;
	  memcpy (volumeName+1, fullPath, volume_len);
	  fileName[0] = file_len;
	  memcpy (fileName+1, last_colon + 1, file_len);
	  retval = noErr;
	}
    }
  return retval;
}

PRIVATE void
init_head (alias_head_t *headp, Str27 volumeName, Str31 fileName)
{
  memset (headp, 0, sizeof *headp);
  headp->usually_2 = CWC (2);
  memcpy (headp->volumeName, volumeName, volumeName[0]+1);
  memcpy (headp->fileName, fileName, fileName[0]+1);
  headp->mystery_words[0] = CWC (-1);
  headp->mystery_words[1] = CWC (-1);
  headp->mystery_words[3] = CWC (17);
}

PRIVATE void
init_tail (alias_tail_t *tailp, Str32 zoneName, Str31 serverName,
	   Str27 volumeName)
{
  Handle h;

  memset (tailp, 0, sizeof *tailp);
  memcpy (tailp->zone, zoneName, zoneName[0]+1);
  memcpy (tailp->server, serverName, serverName[0]+1);
  memcpy (tailp->volumeName, volumeName, volumeName[0]+1);
  h = (Handle) GetString (-16096);
  if (!h)
    tailp->network_identity_owner_name[0] = 0;
  else
    {
      int name_len;
		
      name_len = MIN (GetHandleSize (h), 31);
      memcpy (tailp->network_identity_owner_name, STARH (h), name_len);
    }
  tailp->weird_info[ 0] = CWC (0x00A8);
  tailp->weird_info[ 1] = CWC (0x6166);
  tailp->weird_info[ 2] = CWC (0x706D);
  tailp->weird_info[ 5] = CWC (0x0003);
  tailp->weird_info[ 6] = CWC (0x0018);
  tailp->weird_info[ 7] = CWC (0x0039);
  tailp->weird_info[ 8] = CWC (0x0059);
  tailp->weird_info[ 9] = CWC (0x0075);
  tailp->weird_info[10] = CWC (0x0095);
  tailp->weird_info[11] = CWC (0x009E);
}

PRIVATE OSErr
assemble_pieces (AliasHandle *ahp, alias_head_t *headp, INTEGER n_pieces, ...)
{
  Size n_bytes_needed;
  va_list va;
  int i;
  Handle h;
  OSErr retval;
	
  n_bytes_needed = sizeof *headp;
  va_start (va, n_pieces);
  for (i = 0; i < n_pieces; ++i)
    {
      INTEGER tag;
      INTEGER length;
      void *p;
		
      tag = va_arg (va, int);
      length = va_arg (va, int);
      p = va_arg (va, void *);
      n_bytes_needed += sizeof (INTEGER) + sizeof (INTEGER) + EVENUP (length);
    }
  va_end (va);
  n_bytes_needed += sizeof (INTEGER) + sizeof (INTEGER);
  h = NewHandle (n_bytes_needed);
  if (!h)
    retval = MemError ();
  else
    {
      char *op;
		
      headp->length = n_bytes_needed;
      op = (char *) STARH (h);
      memcpy (op, headp, sizeof (*headp));
      op += sizeof (*headp);
      va_start (va, n_pieces);
      for (i = 0; i < n_pieces; ++i)
	{
	  INTEGER tag, tag_x;
	  INTEGER length, length_x;
	  void *p;
		
	  tag = va_arg (va, int);
	  tag_x = CW (tag);
	  length = va_arg (va, int);
	  length_x = CW (length);
	  p = va_arg (va, void *);
	  memcpy (op, &tag_x, sizeof tag_x);
	  op += sizeof tag_x;
	  memcpy (op, &length_x, sizeof length_x);
	  op += sizeof length_x;
	  memcpy (op, p, length);
	  op += length;
	  if (length & 1)
	    *op++ = 0;
	}
      va_end (va);
      memset (op, -1, sizeof (INTEGER));
      op += sizeof (INTEGER);
      memset (op, 0, sizeof (INTEGER));
		
      *ahp = (AliasHandle) RM (h);
      retval = noErr;
    }
  return retval;
}

/*
FULL_PATH_TAG, path_len, fullPath,
TAIL_TAG     , sizeof (tail)-2, &tail.weird_info)
*/

P5 (PUBLIC pascal trap, OSErr, NewAliasMinimalFromFullPath,
    INTEGER, path_len, Ptr, fullPath, Str32, zoneName, Str31, serverName,
    AliasHandle *, ahp)
{
  OSErr retval;

  warning_unimplemented ("not tested much");
  if (zoneName[0] > 32 || serverName[0] > 31 || !ahp)
    retval = paramErr;
  else
    {
      Str27 volumeName;
      Str63 fileName;
		
      retval = decompose_full_path (path_len, fullPath, volumeName, fileName);
      if (retval == noErr)
	{
	  alias_head_t head;
	  alias_tail_t tail;
			
	  init_head (&head, volumeName, fileName);
	  if (volumeName[0] < 27)
	    head.volumeName[volumeName[0]+1] = ':';
	  head.zero_or_one = CWC (1);
	  head.zero_or_neg_one = CLC (-1);
	  head.ioDirID = CLC (-1);
	  head.ioFlCrDat = CLC (0);
	  init_tail (&tail, zoneName, serverName, volumeName);
	  retval = assemble_pieces (ahp, &head, 2,
				    FULL_PATH_TAG, path_len, (void *) fullPath,
				    TAIL_TAG     , (int) sizeof (tail)-2,
				    (void *) &tail.weird_info);
			
	}
    }
	
  if (retval != noErr)
    *ahp = NULL;
	
  return retval;
}

P2 (PUBLIC pascal trap, OSErr, NewAliasMinimal,
FSSpecPtr, fsp, AliasHandle *, ahp)
{
  HParamBlockRec hpb;
  OSErr retval;
  Str27 volName;

  warning_unimplemented ("not tested much");
  memset (&hpb, 0, sizeof hpb);
  hpb.ioParam.ioNamePtr = RM (&volName[0]);
  hpb.ioParam.ioVRefNum = fsp->vRefNum;
  retval = PBHGetVInfo (&hpb, FALSE);
  if (retval == noErr)
    {
      alias_head_t head;
		
      init_head (&head, volName, fsp->name);
      head.ioVCrDate = hpb.volumeParam.ioVCrDate;
      head.ioVSigWord = hpb.volumeParam.ioVSigWord;
      memset (&hpb, 0, sizeof hpb);
      hpb.ioParam.ioNamePtr = RM (&fsp->name[0]);
      hpb.ioParam.ioVRefNum = fsp->vRefNum;
      hpb.fileParam.ioDirID = fsp->parID;
      retval = PBHGetFInfo (&hpb, FALSE);
      if (retval == noErr)
	{
	  alias_tail_t tail;
	  Handle h;
	  Str31 serverName;
			
	  head.ioDirID = hpb.fileParam.ioDirID;
	  head.ioFlCrDat = hpb.fileParam.ioFlCrDat;
	  head.type_info = hpb.fileParam.ioFlFndrInfo.fdType;
	  head.creator = hpb.fileParam.ioFlFndrInfo.fdCreator;
	  h = (Handle) GetString (-16413);
	  if (!h)
	    serverName[0] = 0;
	  else
	    {
	      int len;
				
	      len = MIN (GetHandleSize (h), 32);
	      memcpy (serverName, STARH (h), len);
	    }
	  init_tail (&tail, (StringPtr) "\1*", serverName, volName);
	  retval = assemble_pieces (ahp, &head, 1,
				    TAIL_TAG, (int) sizeof (tail)-2,
				    (void *) &tail.weird_info);
	}
    }
	
  if (retval != noErr)
    *ahp = NULL;
  return retval;
}
