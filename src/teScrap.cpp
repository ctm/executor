/* Copyright 1986-1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_teScrap[] =
	    "$Id: teScrap.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in TextEdit.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "TextEdit.h"
#include "MemoryMgr.h"
#include "ScrapMgr.h"

#include "rsys/cquick.h"
#include "rsys/mman.h"

using namespace Executor;

A0(PUBLIC, OSErr, TEFromScrap)
{
  GUEST<int32> l;
  int32 m;
    
  m = GetScrap(MR (TEScrpHandle), TICK ("TEXT"), &l);
  if (m < 0)
    {
      EmptyHandle (MR (TEScrpHandle));
      TEScrpLength = CWC (0);
    }
  else
    TEScrpLength = CW (m);
  return m < 0 ? m : noErr;
}

A0 (PUBLIC, OSErr, TEToScrap)
{
  int32 m;

    HLockGuard guard(MR (TEScrpHandle));

       m = PutScrap (CW (TEScrpLength), TICK ("TEXT"),
		     STARH (MR (TEScrpHandle)));
  return m < 0 ? m : 0;
}

A0 (PUBLIC, Handle, TEScrapHandle)
{
  return MR (TEScrpHandle);
}

A0 (PUBLIC, int32, TEGetScrapLen)
{
  return CW (TEScrpLength);
}

A1 (PUBLIC, void, TESetScrapLen, int32, ln)
{
  TEScrpLength = CW (ln);
}
