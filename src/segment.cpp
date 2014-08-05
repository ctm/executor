/* Copyright 1989, 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_segment[] =
		"$Id: segment.c 88 2005-05-25 03:59:37Z ctm $";
#endif

/* Forward declarations in SegmentLdr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"

#include "FileMgr.h"
#include "SegmentLdr.h"
#include "MemoryMgr.h"
#include "ResourceMgr.h"
#include "StdFilePkg.h"
#include "SysErr.h"
#include "FontMgr.h"
#include "OSEvent.h"
#include "WindowMgr.h"

#include "rsys/hfs.h"
#include "rsys/file.h"
#include "rsys/notmac.h"
#include "rsys/glue.h"
#include "rsys/wind.h"
#include "rsys/segment.h"
#include "rsys/host.h"
#include "rsys/vdriver.h"
#include "rsys/executor.h"
#include "rsys/flags.h"
#include "rsys/prefs.h"
#include "rsys/osevent.h"
#include "rsys/cquick.h"
#include "rsys/desk.h"
#include "rsys/dcache.h"
#include "rsys/launch.h"

#include <ctype.h>

#if defined (CYGWIN32)

#include "winfs.h"

/*
 * NOTE:  I've looked at MINGW32 0.1.3 and the io.h #defines
 *        X_OK, which we use below.  I think Sam uses 0.1.3, even though
 *	  ARDI is currently at 0.1.2.
 */

#if !defined (OLD_MINGWIN32)
#include <io.h> /* needed for X_OK */
#else
#define X_OK 4 /* this is really just to get segment.c to compile */
#endif

#endif

#if defined(MSDOS)
#include "dosevents.h"
#include "vga.h"
#endif /* defined(MSDOS) */
namespace Executor {
typedef finderinfo *finderinfoptr;
MAKE_HIDDEN(finderinfoptr);
typedef HIDDEN_finderinfoptr *finderinfohand;
}

using namespace Executor;
using namespace ByteSwap;

PUBLIC int Executor::ROMlib_cacheheuristic = FALSE;


#if defined(ONLY_DESTROY_BETWEEN_CODE_SEGMENTS)
static short flush_bypass = 0;

/* This is the total number of code resources accessible to the program.
 * If it's zero, we recompute it.  Remember to set it to zero whenever
 * launching a new program!
 */

typedef struct {
    syn68k_addr_t start;
    uint32 num_bytes;
} destroy_pair_t;

INTEGER ROMlib_num_code_resources = 0;

static int compar(const void *p1, const void *p2)
{
    int retval;
    uint32 s1, s2;

    s1 = (uint32) ((const destroy_pair_t *) p1)->start;
    s2 = (uint32) ((const destroy_pair_t *) p2)->start;

    if (s1 < s2)
	retval = -1;
    else if (s2 < s1)
	retval = 1;
    else
	retval = 0;
    return retval;
}
#endif

A0(PUBLIC, void, flushcache)
{
    ROMlib_destroy_blocks( (syn68k_addr_t) 0, (uint32) ~0, TRUE );
}

A2(PUBLIC trap, void, HWPriv, LONGINT, d0, LONGINT, a0)
{
    static char d_cache_enabled = TRUE, i_cache_enabled = TRUE;
    int new_state;

    switch (d0) {
    case 0:	/* Dis/Ena Instr cache */
        warning_unimplemented ("Dis/Ena instr cache");
	new_state = ((EM_A0 & 0xFFFF) != 0);
	EM_A0 = i_cache_enabled;
	i_cache_enabled = new_state;
	break;
    case 1:	/* Flush Instr cache */
	flushcache();
	break;
    case 2:	/* Dis/Ena Data cache */
        warning_unimplemented ("Dis/Ena data cache");
	new_state = ((EM_A0 & 0xFFFF) != 0);
	EM_A0 = d_cache_enabled;
	d_cache_enabled = new_state;
	break;
    case 3:	/* Flush Data cache */
#if 0
	flushcache();
#endif
	break;
    case 4:	/* Enable external cache */
        warning_unimplemented ("Enable external cache");
	break;
    case 5:	/* Disable external cache */
        warning_unimplemented ("Disable external cache");
	break;
    case 6:	/* Flush external cache */
	flushcache();
	break;
    case 9:	/* Flush cache range */
	ROMlib_destroy_blocks ((syn68k_addr_t) a0, EM_A1, TRUE);
	EM_D0 = noErr;	/* Maybe we should only touch d0.w? */
	break;
    default:
        warning_unexpected ("d0 = 0x%x", d0); 
	EM_D0 = hwParamErr;	/* Maybe we should only touch d0.w? */
	break;
    }
}

A1(PUBLIC, char *, ROMlib_undotdot, char *, origp)
{
    int dotcount, nleft;
    char *p, *oldloc;
    boolean_t slashseen_p;

    nleft = strlen(origp) + 1;
    slashseen_p = FALSE;
    dotcount = -1;
    for (p = origp; *p; p++, nleft--) {
	switch (*p) {
	case '/':
	    switch (dotcount) {
	    case 0:		/* slash slash */
	        BlockMove((Ptr) p+1, (Ptr) p, (Size) nleft-1);
		break;
	    case 1:		/* slash dot slash */
		BlockMove((Ptr) p+1, (Ptr) p-1, (Size) nleft-1);
		p -= 2;
		dotcount = 0;
		break;
	    case 2:		/* slash dot dot slash */
		for (oldloc = p-4; oldloc >= origp && *oldloc != '/'; --oldloc)
		    ;
		if (oldloc < origp)
		    oldloc = origp + 1;
		else
		    ++oldloc;
		BlockMove((Ptr) p+1, (Ptr) oldloc, (Size) nleft-1);
		p = oldloc - 1;
		dotcount = 0;
		break;
	    default:
		dotcount = 0;
		break;
	    }
	    slashseen_p = TRUE;
	    break;
	case '.':
	    if (slashseen_p)
	      dotcount++;
	    break;
	default:
	    slashseen_p = FALSE;
	    dotcount = -1;
	    break;
	}
    }
    return origp;
}

PRIVATE void lastcomponent( StringPtr dest, StringPtr src )
{
    unsigned char *c, *lastcolon;
    int n;

    lastcolon = 0;
    for (c = src + 1, n = src[0]; --n >= 0; ++c)
	if (*c == ':')
	    lastcolon = c;
    if (lastcolon) {
	++lastcolon;
	n = src[0] - (lastcolon - (src+1));
    } else
	n = src[0];
    dest[0] = n;
    memmove(dest+1, lastcolon, n);
}

#if defined (MSDOS) || defined (CYGWIN32)
PRIVATE char *
canonicalize_potential_windows_path (char *uname)
{
  char *p;

  for (p = uname; *p; ++p)
    if (*p == '\\')
      *p = '/';

  return uname;
}
#endif

PRIVATE boolean_t
full_pathname_p (char *uname)
{
  boolean_t retval;

  retval = uname[0] == '/';
#if defined (MSDOS) || defined(CYGWIN32)
  if (!retval && uname[0] && uname[1] == ':' && uname[2] == '/')
    retval = TRUE;
#endif
  return retval;
}

PRIVATE uint8
hexval (char c)
{
  uint8 retval;

  if (c >= '0' && c <= '9')
    retval = c - '0';
  else if (c >= 'a' && c <= 'f')
    retval = c - 'a' + 10;
  else if (c >= 'A' && c <= 'F')
    retval = c - 'A' + 10;
  else
    retval = 0;

  return retval;
}

/*
 * Copies a c string into a pascal string changing occurrances of ::XY
 * (where X and Y are hex digits) into the character 0xXY.
 */

PRIVATE void
colon_colon_copy (StringPtr dst, const char *src)
{
  StringPtr save_dst;

  save_dst = dst;
  while (*src)
    {
      if (src[0] == ':' && src[1] == ':'
	  && isxdigit (src[2]) && isxdigit (src[3]))
	{
	  *++dst = (hexval (src[2]) << 4) | hexval (src[3]);
	  src += 4;
	}
      else
	*++dst = *src++;
    }
  save_dst[0] = dst - save_dst;
}

/*
 * This routine converts the UNIX path path to a pascal string that
 * starts with a colon whose individual components have been converted
 * from AppleDouble representation to Mac representation.  In theory we
 * could use this whether or not we were using the netatalk naming convention,
 * but since this is a recent addition to ROMlib and since we're about to
 * start building 2.1 candidates it makes sense to just use this in the
 * unsupported case and continue using the old code with non-netatalk naming.
 */

#define MAC_LOCAL_FROM_UNIX_LOCAL(s)		\
({											\
  char *_s;									\
  unsigned char *retval;					\
  int len;									\
  int count;								\
  char *next_slash;							\
											\
  _s = (s);									\
  len = strlen (_s);						\
  retval = (unsigned char*)alloca (len + 2);\
  retval[1] = ':';							\
  count = 1;								\
											\
  do										\
    {										\
      int component_len, shrinkage;			\
											\
      next_slash = strchr (_s, '/');		\
      if (next_slash)						\
	component_len = next_slash - _s;		\
      else									\
	component_len = strlen (_s);								\
      memcpy (retval+count+1, _s, component_len);				\
      shrinkage = ROMlib_UNIX7_to_Mac ((char *) retval+count+1,	\
                                       component_len);			\
      count += component_len - shrinkage;						\
      if (next_slash) {											\
																\
	  _s += component_len + 1; /* skip the '/', too */			\
	  retval[count+1] = ':';								\
	  ++count;												\
	}						\
    }						\
    while (next_slash);		\
							\
  retval[0] = count;		\
  retval;					\
})

namespace Executor {
	PRIVATE BOOLEAN argv_to_appfile(char*, AppFile *);
	PUBLIC void ROMlib_seginit(LONGINT, char **);		/* INTERNAL */
}

A2(PRIVATE, BOOLEAN, argv_to_appfile, char *, uname, AppFile *, ap)
{
  int namelen, pathlen;
  unsigned char *path, *p;
  VCBExtra *vcbp;
  BOOLEAN retval;
  INTEGER sysnamelen, totnamelen;
  CInfoPBRec cinfo;
  WDPBRec wpb;
  struct stat sbuf;

#if defined (MSDOS) || defined(CYGWIN32)
  uname = canonicalize_potential_windows_path (uname);
#endif

  if (uname && Ustat(uname, &sbuf) == 0)
    {    
      if (!full_pathname_p (uname))
	{
	  namelen = strlen (uname);
	  path = (unsigned char *) ALLOCA(MAXPATHLEN + 1 + namelen + 1);
	  if (getcwd ((char *) path, MAXPATHLEN))
	    {
	      pathlen = strlen ((char *) path);
	      path[pathlen] = '/';
	      if (uname[0] == '.' && uname[1] == '/')
		BlockMove ((Ptr) uname+2, (Ptr) (path+pathlen+1),
			   (Size) namelen+1-2);
	      else
		BlockMove ((Ptr) uname, (Ptr) (path+pathlen+1),
			   (Size) namelen+1);
	      uname = (char *) path;
#if defined (MSDOS) || defined(CYGWIN32)
	      uname = canonicalize_potential_windows_path (uname);
#endif
	    }
	  else
	    warning_unexpected ("getwd failed: expect trouble reading "
				"resources");
	}
      ROMlib_automount(uname);
      vcbp = (VCBExtra *) ROMlib_vcbbybiggestunixname(uname);
      if (vcbp)
	{
	  cinfo.hFileInfo.ioVRefNum = vcbp->vcb.vcbVRefNum;
	  wpb.ioNamePtr  = 0;
	  sysnamelen = strlen(vcbp->unixname);
	  if (sysnamelen == 1 + SLASH_CHAR_OFFSET) /* don't remove leading
						      '/' if that's
						      all there is */
	    {
	      sysnamelen = 0;
	      uname += SLASH_CHAR_OFFSET;
	    }

	  /* NOTE: we can probably skip the following "if" and the else
	     part and just use the "path = MAC_LOC..." line unconditionally,
	     but I don't want to possibly introduce a subtle bug at the last
	     minute before releasing 2.1 final. */

	  if (apple_double_quote_char == ':')
	    path = MAC_LOCAL_FROM_UNIX_LOCAL (uname + sysnamelen + 1);
	  else
	    {
	      totnamelen = strlen(uname);
	      pathlen = totnamelen - sysnamelen + 2;	/* count & NUL */
	      path = (unsigned char*)alloca(pathlen);
	      strcpy((char *) path+1, uname + sysnamelen); /* path has count and */
	      path[0] = pathlen - 2;		     /* is NUL terminated */
	      path[1] = ':';
	      for (p = path; *++p;)
		if (*p == '/')
		  *p = ':';

	      path[0] -= ROMlib_UNIX7_to_Mac((char *) path+1, path[0]);
	    }
	}
      else
	path = 0; /* TODO:  Some sort of problem here */
    }
  else
    {
      int len;
      len = strlen (uname);
      path = (unsigned char*)alloca (len + 1);
      colon_colon_copy (path, uname);
      cinfo.hFileInfo.ioVRefNum = 0;
      wpb.ioNamePtr  = RM(path);
    }
  cinfo.hFileInfo.ioNamePtr = RM(path);
  cinfo.hFileInfo.ioFDirIndex = CWC (0);
  cinfo.hFileInfo.ioDirID = 0;
  if ((retval = (PBGetCatInfo(&cinfo, FALSE) == noErr)))
    {
      ap->fType = cinfo.hFileInfo.ioFlFndrInfo.fdType;
      ap->versNum = 0;
      wpb.ioVRefNum  = cinfo.hFileInfo.ioVRefNum;
      wpb.ioWDProcID = TICKX("unix");
      wpb.ioWDDirID  = cinfo.hFileInfo.ioFlParID;
      if (PBOpenWD(&wpb, FALSE) == noErr)
	{
	  ap->vRefNum = wpb.ioVRefNum;
	  lastcomponent(ap->fName, path);
	}
      else
	{
	  ap->vRefNum = cinfo.hFileInfo.ioVRefNum;
	  str255assign(ap->fName, path);
	}
    }
  else
    {
      warning_unexpected ("%s: unable to get info on `%s'\n", program_name,
			  uname);
    }
  return retval;
}

#if defined(MACOSX_) && defined(BINCOMPAT)
PUBLIC INTEGER Executor::ROMlib_acceptsanotherfile = TRUE;
#endif

PUBLIC int Executor::ROMlib_print;

#if !defined (MSDOS) && !defined(CYGWIN32)
#define PATH_SEPARATER ':'
#else
#define PATH_SEPARATER ';'
#endif

A2(PUBLIC, void, ROMlib_seginit, LONGINT, argc, char **, argv)	/* INTERNAL */
{
    char *path, *firstcolon;
    char *fullpathname;
    finderinfohand fh;
    AppFile app;
    INTEGER newcount;
    THz saveZone;

    fullpathname = 0;
    if (Uaccess(argv[0], X_OK) == 0)
	fullpathname = argv[0];
    else {
	for (path = getenv("PATH"); path && path[0];
				      path = firstcolon ? firstcolon + 1 : 0) {
	    if ((firstcolon = strchr(path, PATH_SEPARATER)))
		*firstcolon = 0;
	    if (path[0]) {
		fullpathname =
		       (char *) NewPtr(strlen(path) + 1 + strlen(argv[0]) + 1);
		sprintf(fullpathname, "%s/%s", path, argv[0]);
	    } else {
		fullpathname = (char *) NewPtr(strlen(argv[0]) + 1);
		sprintf(fullpathname, "%s", argv[0]);
	    }
	    if (firstcolon)
	      *firstcolon = PATH_SEPARATER; /* if we don't replace this,
					       Linux gets confused */
	    if (Uaccess(fullpathname, X_OK) == 0)
		firstcolon = 0;	/* this will break us out of the loop */
	    else {
		DisposPtr((Ptr) fullpathname);
		fullpathname = 0;
	    }
	}
    }

    /* NOTE: It's not clear why we're calling argv_to_appfile on fullpathname
       below, but argv_to_appfile has enough potential side-effects that I'm
       not about to remove it.  OTOH, the call to OpenRFPerm will create
       a spurious %executor file and then fail, so I've #if 0'd it out. */

    if (argv_to_appfile(fullpathname, &app)) {
#if 0
	CurApRefNum = BigEndianValue(OpenRFPerm(app.fName, BigEndianValue(app.vRefNum), fsCurPerm));
#endif
	CurApName[0] = MIN(app.fName[0], sizeof(CurApName)-1);
	BlockMove((Ptr) app.fName+1, (Ptr) CurApName+1, (Size) CurApName[0]);
    } else {
	CurApRefNum = -1;
	CurApName[0] = 0;
    }
    saveZone = TheZone;
    TheZone = SysZone;
    fh = (finderinfohand)
		        NewHandle((Size) sizeof(finderinfo) - sizeof(AppFile));
    TheZone = saveZone;

    AppParmHandle = (Handle) RM(fh);
    HxX(fh, count)   = 0;
    HxX(fh, message) = ROMlib_print ? CWC(appPrint) : CWC(appOpen) ;
    if (fullpathname && fullpathname != argv[0])
	DisposPtr((Ptr) fullpathname);
#if defined(NEXTSTEP) && defined(BINCOMPAT)
    ROMlib_acceptsanotherfile = FALSE;
    if (ROMlib_toexec) {
	if (ROMlib_toopen)
	    ROMlib_toexec = *(char **) ROMlib_toexec;	/* ick! */
	if (argv_to_appfile(ROMlib_toexec, &app)) {
	    ROMlib_startupscreen = FALSE;
	    ROMlib_exit = TRUE;
	    newcount = Hx(fh, count) + 1;
	    HxX(fh, count) = BigEndianValue(newcount);
	    SetHandleSize((Handle) fh,
		     (char *) &HxX(fh, files)[newcount] - (char *)STARH(fh));
	    HxX(fh, files)[Hx(fh, count)-1] = app;
	}
	if (ROMlib_toopen) {
	    if (argv_to_appfile(ROMlib_toopen, &app)) {
		newcount = Hx(fh, count) + 1;
		HxX(fh, count) = BigEndianValue(newcount);
		SetHandleSize((Handle) fh,
		    (char *) &HxX(fh, files)[newcount] - (char *) STARH(fh));
		HxX(fh, files)[Hx(fh, count)-1] = app;
	    }
	}
    } else
#endif
    while (--argc > 0) {
	++argv;
	if (argv_to_appfile(argv[0], &app)) {
	    ROMlib_startupscreen = FALSE;
	    ROMlib_exit = TRUE;
	    newcount = Hx(fh, count) + 1;
	    HxX(fh, count) = BigEndianValue(newcount);
	    SetHandleSize((Handle) fh,
		    (char *) &HxX(fh, files)[newcount] - (char *) STARH(fh));
	    HxX(fh, files)[Hx(fh, count)-1] = app;
	}
    }
}

A2(PUBLIC, void, CountAppFiles, INTEGER *, messagep,
					INTEGER *, countp)	/* IMII-57 */
{
    if (AppParmHandle)
      {
	if (messagep)
	  *messagep = STARH((finderinfohand) MR(AppParmHandle))->message;
	if (countp)
	  *countp   = STARH((finderinfohand) MR(AppParmHandle))->count;
      }
    else
      *countp = 0;
}

A2(PUBLIC, void, GetAppFiles, INTEGER, index, AppFile *, filep)	/* IMII-58 */
{
    *filep = STARH((finderinfohand)MR(AppParmHandle))->files[index-1];
}

A1(PUBLIC, void, ClrAppFiles, INTEGER, index)	/* IMII-58 */
{
    if (STARH((finderinfohand)MR(AppParmHandle))->files[index-1].fType) {
	STARH((finderinfohand)MR(AppParmHandle))->files[index-1].fType = 0;
	STARH((finderinfohand)MR(AppParmHandle))->count = 
		      BigEndianValue(BigEndianValue(STARH((finderinfohand)MR(AppParmHandle))->count) - 1);
    }
}

P3(PUBLIC pascal trap, void, GetAppParms, StringPtr, namep,	/* IMII-58 */
				      INTEGER *, rnp, HIDDEN_Handle *, aphandp)
{
    str255assign(namep, CurApName);
    *rnp = CurApRefNum;
    (*aphandp).p = AppParmHandle;
}

PUBLIC char *ROMlib_errorstring;
PUBLIC char ROMlib_exit = 0;

#if defined(MSDOS)
#include <limits.h>
#endif /* defined(MSDOS) */

#if defined(MSDOS)
PRIVATE void execme(const char *toexec)
{
#if !defined(MSDOS)
    execl(toexec, toexec, "-nosplash", (char *) 0);
#else
#if defined(notdef)	/* we don't do this, now that we run under Windows */
    int i;
    i386_registers_t regs;

    for (i = 3; i < OPEN_MAX; ++i)	/* leave 0, 1, 2 open */
	close(i);
    regs.l.ebx = (ULONGINT) toexec;
    regs.l.ecx = (ULONGINT) "-nosplash";
    regs.w.ax = 0xFF10;  /* Turbo assist case 16. */
    int21(&regs);
#endif
#endif
}
#endif /* defined(MSDOS) */

PUBLIC int Executor::ROMlib_nobrowser = 0;

PRIVATE BOOLEAN valid_browser( void )
{
  OSErr err;
  FInfo finfo;

  err = GetFInfo(FinderName, BigEndianValue(BootDrive), &finfo);
  return !ROMlib_nobrowser && err == noErr && finfo.fdType == TICKX("APPL");
}

PRIVATE void launch_browser( void )
{
  /* Set the depth to what was specified on the command line;
   * if nothing was specified there, set the depth to the maximum
   * supported bits per pixel.
   */
  SetDepth (MR (MainDevice),
	    (flag_bpp
	     ? MIN (flag_bpp, vdriver_max_bpp)
	     : vdriver_max_bpp),
	    0, 0);
  Launch(FinderName, BigEndianValue(BootDrive));
}

P0(PUBLIC pascal trap, void, ExitToShell)
{
  static char beenhere = FALSE;
#if defined(MSDOS)
    char *toexec;
#endif
    ALLOCABEGIN


#if 1

    Point pt;
    static SFTypeList applonly = { CLC(T('A','P','P','L')) };
    SFReply reply;
    char quickbytes[grafSize];
    LONGINT tmpA5;
    WindowPeek t_w;
    int i;

    if (ROMlib_mods & optionKey)
      ROMlib_exit = TRUE;

  /* NOTE: closing drivers is a bad thing in the long run, but for now
     we're doing it.  We actually do it here *and* in reinitialize_things().
     We have to do it here since we want the window taken out of the
     windowlist properly, before we hack it out ourself, but we have to
     do it in reinitializethings because launch calls that, but doesn't
     call exittoshell */

  for (i = DESK_ACC_MIN; i <= DESK_ACC_MAX; ++i)
    CloseDriver (-i - 1);

  empty_timer_queues ();

  if (WWExist == EXIST_YES)
      {
	/* remove global datastructures associated with each window
           remaining in the application's window list */
	for (t_w = MR (WindowList); t_w; t_w = WINDOW_NEXT_WINDOW (t_w))
	  pm_window_closed ((WindowPtr) t_w);
      }
  WindowList = 0;
  
    if (!ROMlib_exit
	&& (!beenhere
	    || strncmp((char *) CurApName+1,
		       BROWSER_NAME, CurApName[0]) != 0)) {
        beenhere = TRUE;

	/* if we call `InitWindows ()', we don't want it shouting
	   about 32bit uncleanliness, &c */
	size_info.application_p = FALSE;
	
	if (QDExist == EXIST_NO) {
	    a5 = (LONGINT) US_TO_SYN68K(&tmpA5);
	    CurrentA5 = (Ptr) BigEndianValue(a5);
	    InitGraf((Ptr) quickbytes + sizeof(quickbytes) - 4);
	}
	InitFonts();
	FlushEvents( everyEvent, 0 );
	if (WWExist == EXIST_NO)
	    InitWindows();
        if (TEScrpHandle == (Handle)CLC(-1) || TEScrpHandle == (Handle)CLC(0))
	    TEInit();
	if (DlgFont == CWC(0) || DlgFont == CWC(-1))
	    InitDialogs((ProcPtr)0);
	InitCursor();

	if (valid_browser())
	  {
	    /* NOTE: much of the initialization done above isn't really
	             needed here, but I'd prefer to have the same environment
		     when we auto-launch browser as when we choose an app
		     using stdfile */
	    launch_browser();
	  }
	else
	  {
	    pt.h = 100;
	    pt.v = 100;
	    SFGetFile(pt, (StringPtr) "", (ProcPtr) 0, 1,
		      applonly, (ProcPtr) 0, &reply);

	    if (!reply.good)
	      ROMlib_exit = 1;
	    else
	      {
		CurApName[0] = MIN(reply.fName[0], 31);
		BlockMove((Ptr) reply.fName+1, (Ptr) CurApName+1,
			  (Size) CurApName[0]);
		Launch(CurApName, BigEndianValue(reply.vRefNum));
	      }
	  }
    }

#endif

    CloseResFile(0);
    ROMlib_OurClose();

    dcache_invalidate_all (TRUE);

#if defined (X)
    autorepeatonX();
#endif /* X */

#if defined(SUN) && !defined(X) && !defined(MACOSX_)
    close(_windowfd);	/* may be bad choice */
    BlockMove(_savebits, _addr, (Size) _nb);
#endif /* SUN && !defined */

#if defined (SCO)
    ev_close();
    ioctl( 1, (MODESWITCH|_savemode), 0);
    restorekb();
#endif /* SCO */
    if (ROMlib_errorstring) {
	write(2, ROMlib_errorstring, strlen(ROMlib_errorstring));
	gui_abort();
    }
#if defined(NEXT)
    stopprotectingus();
#endif
#if 0
    if (FinderName[0]) {
	toexec = ALLOCA(FinderName[0]+1);
	BlockMove((Ptr) FinderName+1, (Ptr) toexec, (Size) FinderName[0]);
	toexec[FinderName[0]] = 0;
	execlp(toexec, toexec, (char *) 0); 
	/* if it fails we'll exit */
    }
#endif

#if defined(MSDOS)
#define EXECUTORSUFFIX ".exe"	/* This is necessary because of the way go32 */
#else /* works with Executor */
#define EXECUTORSUFFIX ""
#endif
#if defined(MSDOS)
    if (!ROMlib_exit) {
	toexec = ALLOCA(strlen(ROMlib_startdir) + 1 +
						   strlen(ROMlib_appname) +
						   strlen(EXECUTORSUFFIX) + 1);
	sprintf(toexec, "%s/%s%s", ROMlib_startdir, ROMlib_appname,
							       EXECUTORSUFFIX);
	execme(toexec);
    }
#endif
    exit(ROMlib_exit == 1 ? 0 : ROMlib_exit); /* 1 is historically good */
    ALLOCAEND	/* yeah, right, if exit fails... */
}

#if !defined (BINCOMPAT)

P1(PUBLIC pascal trap, void, UnloadSeg, Ptr, addr)	/* INTERNAL */
{
    /* NOP */
}

#else /* BINCOMPAT */

#define JMPLINSTR	0x4EF9
#define MOVESPINSTR	0x3F3C
#define LOADSEGTRAP	0xA9F0

P1(PUBLIC pascal trap, void, LoadSeg, INTEGER volatile, segno)
{
    Handle newcode;
    unsigned short offbytes;
    INTEGER taboff, nentries, savenentries;
    short *ptr, *saveptr;
#if defined(MACOSX_)
    if (ROMlib_appbit && !(ROMlib_appbit & ROMlib_whichapps))
	ExitToShell();
#endif

    ResLoad = -1;	/* CricketDraw III's behaviour suggested this */
    newcode = GetResource(TICK("CODE"), segno);
    HLock(newcode);
    taboff   = BigEndianValue(((INTEGER *) STARH(newcode))[0]);
    if ((uint16) taboff == 0xA89F)  /* magic compressed resource signature */
      {
	/* We are totally dead here.  We almost certainly can't use
	 * `system_error' to inform the user because the window
	 * manager probably is not yet running.
	 */

	ROMlib_launch_failure = (system_version >= 0x700 ?
				 launch_compressed_ge7 :
				 launch_compressed_lt7); 
	C_ExitToShell ();
      }
    savenentries = nentries = BigEndianValue(((INTEGER *) STARH(newcode))[1]);

    saveptr = ptr = (short *) ((char *) (long) SYN68K_TO_US(a5) + taboff + Cx(CurJTOffset));
    while (--nentries >= 0) {
	if (ptr[1] != CWC(JMPLINSTR)) {
	    offbytes = BigEndianValue(*ptr);
	    *ptr++ = BigEndianValue(segno);
	    *ptr++ = CWC(JMPLINSTR);
	    *(LONGINT *) ptr =
		 BigEndianValue((LONGINT) (long) ((char *) US_TO_SYN68K(STARH(newcode)) + offbytes + 4));
	    ptr += 2;
	} else
	    ptr += 4;
    }
    ROMlib_destroy_blocks( (syn68k_addr_t) (long) US_TO_SYN68K(saveptr), 8L * savenentries,
			  TRUE);
}

#define SEGNOOFP(p) (BigEndianValue(((INTEGER *)p)[-1]))

namespace Executor {
  PRIVATE void unpatch(Ptr, Ptr);
}

A2(PRIVATE, void, unpatch, Ptr, segstart, Ptr, p)
{
    INTEGER *ip;
    Ptr firstpc;

    ip = (INTEGER *) p;

    firstpc = MR(*(Ptr *)(p + 2));
    ip[1]  = ip[-1];	/* the segment number */
    ip[-1] = BigEndianValue(firstpc - segstart - 4);
    ip[0]  = CWC(MOVESPINSTR);
    ip[2]  = CWC(LOADSEGTRAP);
}

P1(PUBLIC pascal trap, void, UnloadSeg, Ptr, addr)
{
    Ptr p, segstart;
    char *top, *bottom;

    Handle h;
    INTEGER segno;

    if (* (INTEGER *) addr == CWC(JMPLINSTR)) {
	segno = SEGNOOFP(addr);
	h = GetResource(TICK("CODE"), segno);
	if (!(*h).p)
	    LoadResource(h);
	segstart = STARH(h);
	for (p = addr;   SEGNOOFP(p) == segno; p += 8)
	    unpatch(segstart, p);

	bottom = (char *) p;

	for (p = addr-8; SEGNOOFP(p) == segno; p -= 8)
	    unpatch(segstart, p);

	top = (char *) p + 6;	/* +8 that we didn't zap, -2 that we went
					overboard on unpatch (see above) */
	ROMlib_destroy_blocks( (syn68k_addr_t) (long) US_TO_SYN68K(top),
			       bottom - top, FALSE);

	HUnlock(h);
	HPurge(h);
    }
}

#endif /* BINCOMPAT */
