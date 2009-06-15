/* Copyright 1994 - 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_launch[] =
	    "$Id: launch.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"

#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "ResourceMgr.h"
#include "SegmentLdr.h"
#include "MemoryMgr.h"
#include "StdFilePkg.h"
#include "EventMgr.h"
#include "VRetraceMgr.h"
#include "OSUtil.h"
#include "FontMgr.h"
#include "ScrapMgr.h"
#include "ToolboxUtil.h"
#include "FileMgr.h"
#include "ControlMgr.h"
#include "DeviceMgr.h"
#include "SoundDvr.h"
#include "TextEdit.h"
#include "SysErr.h"
#include "SysError.h"
#include "MenuMgr.h"
#include "ScriptMgr.h"
#include "DeskMgr.h"
#include "AppleTalk.h"
#include "PrintMgr.h"
#include "StartMgr.h"
#include "ToolboxEvent.h"
#include "TimeMgr.h"
#include "ProcessMgr.h"
#include "AppleEvents.h"
#include "Gestalt.h"
#include "Package.h"

#include "rsys/trapglue.h"
#include "rsys/file.h"
#include "rsys/next.h"
#include "rsys/sounddriver.h"
#include "rsys/pstuff.h"
#include "rsys/jumpvectors.h"
#include "rsys/prefs.h"
#include "rsys/flags.h"
#include "rsys/aboutpanel.h"
#include "rsys/segment.h"
#include "rsys/misc.h"
#include "rsys/pstuff.h"
#include "rsys/tesave.h"
#include "rsys/blockinterrupts.h"
#include "rsys/resource.h"
#include "rsys/hfs.h"
#include "rsys/osutil.h"
#include "rsys/memory_layout.h"
#include "rsys/stdfile.h"
#include "rsys/notmac.h"
#include "rsys/ctl.h"
#include "rsys/refresh.h"

#include "rsys/options.h"
#include "rsys/cquick.h"
#include "rsys/desk.h"
#include "rsys/parse.h"
#include "rsys/executor.h"
#include "rsys/crc.h"
#include "rsys/float.h"
#include "rsys/mman.h"
#include "rsys/vdriver.h"
#include "rsys/font.h"
#include "rsys/emustubs.h"

#include "rsys/adb.h"
#include "rsys/print.h"
#include "rsys/gestalt.h"
#include "rsys/osevent.h"
#include "rsys/license.h"

#include "rsys/cfm.h"

PRIVATE boolean_t ppc_launch_p = FALSE;

PUBLIC void ROMlib_set_ppc (boolean_t val)
{
  ppc_launch_p = val;
}

#define CONFIGEXTENSION	".ecf"
#define OLD_CONFIG_EXTENSION ".econf" /* must be longer than configextension */

FILE *configfile;

int32 ROMlib_options;

static int16 name0stripappl(StringPtr name)
{
    char *p; 
    int16 retval;

    retval = name[0];
    if (name[0] >= 5) {
	p = (char *)name + name[0] + 1 - 5;
	if ( p[0] == '.' &&
	    (p[1] == 'a' || p[1] == 'A') &&
	    (p[2] == 'p' || p[2] == 'P') &&
	    (p[3] == 'p' || p[3] == 'P') &&
	    (p[4] == 'l' || p[4] == 'L'))
	retval -= 5;
    }
    return retval;
}

/*
 * NOTE: ParseConfigFile now has three arguments.  The second is a standard
 * OSType and will be expanded to 0x1234ABCD.ecf if exefname.ecf
 * doesn't open.  The third is whether or not to do resizes (based on whether
 * the user has changed things by hand).
 */

PUBLIC int ROMlib_nowarn32;

PUBLIC char *ROMlib_configfilename = NULL;

PUBLIC int ROMlib_pretend_help = FALSE;
PUBLIC int ROMlib_pretend_alias = FALSE;
PUBLIC int ROMlib_pretend_edition = FALSE;
PUBLIC int ROMlib_pretend_script = FALSE;

void
remalloc (char **strp)
{
  char *new_string;
  long len;

  if (*strp)
    {
      len = strlen (*strp) + 1;
      new_string = malloc (len);
      if (new_string)
	memcpy (new_string, *strp, len);
      *strp = new_string;
    }
}

void
reset_string (char **strp)
{
  if (*strp)
    free (*strp);
  *strp = 0;
}

int ROMlib_desired_bpp;

PRIVATE void ParseConfigFile(StringPtr exefname, OSType type)
{
    Ptr strdst, savestrdst;
    INTEGER allocsize, dirsize;
    int strwidth;
    char *newtitle;
    char *dot;
    INTEGER fname0;

    reset_string (&ROMlib_WindowName);
    reset_string (&ROMlib_Comments);
    ROMlib_desired_bpp = 0;
    fname0 = name0stripappl(exefname);
    allocsize = fname0;
    if (allocsize < (int) sizeof("0x1234ABCD")-1)	/* IMPORTANT TO CODE */
	allocsize = sizeof("0x1234ABCD")-1;	/* BELOW */
    dirsize = strlen(ROMlib_ConfigurationFolder);
    ROMlib_configfilename =
      realloc(ROMlib_configfilename,
	      dirsize + 1 + allocsize + sizeof(OLD_CONFIG_EXTENSION));
    strdst = (Ptr) ROMlib_configfilename;
    BlockMove((Ptr) ROMlib_ConfigurationFolder, strdst, dirsize);
    strdst += dirsize;
    *strdst++ = '/';
    BlockMove((Ptr) exefname+1, strdst, fname0);
    savestrdst = strdst;
    strdst += fname0;
    BlockMove((Ptr) CONFIGEXTENSION, strdst, sizeof(CONFIGEXTENSION));
    configfile = Ufopen(ROMlib_configfilename, "r");
    if (!configfile) {
      BlockMove((Ptr) OLD_CONFIG_EXTENSION, strdst, sizeof(OLD_CONFIG_EXTENSION));
      configfile = Ufopen(ROMlib_configfilename, "r");
    }
    if (!configfile && type != 0) {
	strdst = savestrdst;
	sprintf((char *) strdst, "%08x", type);
	strdst += sizeof("1234ABCD")-1;
	BlockMove((Ptr) CONFIGEXTENSION, strdst, sizeof(CONFIGEXTENSION));
	configfile = Ufopen(ROMlib_configfilename, "r");
    }
    if (configfile) {
	yyparse();

	/* Stash away the system version. */
	gestalt_set_system_version (system_version);
	SysVersion = CW (system_version);

#if 0	
	if (ROMlib_options & ROMLIB_NOCLOCK_BIT)
	    ROMlib_noclock = 1;
#endif
	if (ROMlib_options & ROMLIB_BLIT_OS_BIT)
	    ROMlib_WriteWhen(WriteInOSEvent);
	if (ROMlib_options & ROMLIB_BLIT_TRAP_BIT)
	    ROMlib_WriteWhen(WriteAtEndOfTrap);
	if (ROMlib_options & ROMLIB_BLIT_OFTEN_BIT)
	    ROMlib_WriteWhen(WriteInBltrgn);
#if 0
	if (ROMlib_options & ROMLIB_ACCELERATED_BIT)
	    ROMlib_accelerated = TRUE;
	else
	    ROMlib_accelerated = FALSE;
#endif
	if (ROMlib_options & ROMLIB_REFRESH_BIT)
	    ROMlib_refresh = 10;
	if (ROMlib_options & ROMLIB_DIRTY_VARIANT_BIT)
	    ROMlib_dirtyvariant = TRUE;
	else
	    ROMlib_dirtyvariant = FALSE;
	if (ROMlib_options & ROMLIB_SOUNDOFF_BIT)
	    ROMlib_PretendSound = soundoff;
	if (ROMlib_options & ROMLIB_PRETENDSOUND_BIT)
	    ROMlib_PretendSound = soundpretend;
	if (ROMlib_options & ROMLIB_SOUNDON_BIT)
	    ROMlib_PretendSound = SOUND_WORKS_P () ? soundon : soundpretend;
#if 0
	if (ROMlib_options & ROMLIB_PASSPOSTSCRIPT_BIT)
	    ROMlib_passpostscript = TRUE;
	else
	    ROMlib_passpostscript = FALSE;
#else
/* #warning ROMlib_passpostscript wired to TRUE */
	ROMlib_passpostscript = TRUE;
#endif
	if (ROMlib_options & ROMLIB_NEWLINETOCR_BIT)
	    ROMlib_newlinetocr = TRUE;
	else
	    ROMlib_newlinetocr = FALSE;
	if (ROMlib_options & ROMLIB_DIRECTDISKACCESS_BIT)
	    ROMlib_directdiskaccess = TRUE;
	else
	    ROMlib_directdiskaccess = FALSE;
	if (ROMlib_options & ROMLIB_NOWARN32_BIT)
	    ROMlib_nowarn32 = TRUE;
	else
	    ROMlib_nowarn32 = FALSE;
	if (ROMlib_options & ROMLIB_FLUSHOFTEN_BIT)
	    ROMlib_flushoften = TRUE;
	else
	    ROMlib_flushoften = FALSE;

	if (ROMlib_options & ROMLIB_PRETEND_HELP_BIT)
	  ROMlib_pretend_help = TRUE;
	else
	  ROMlib_pretend_help = FALSE;

	if (ROMlib_options & ROMLIB_PRETEND_ALIAS_BIT)
	  ROMlib_pretend_alias = TRUE;
	else
	  ROMlib_pretend_alias = FALSE;

	if (ROMlib_options & ROMLIB_PRETEND_EDITION_BIT)
	  ROMlib_pretend_edition = TRUE;
	else
	  ROMlib_pretend_edition = FALSE;

	if (ROMlib_options & ROMLIB_PRETEND_SCRIPT_BIT)
	  ROMlib_pretend_script = TRUE;
	else
	  ROMlib_pretend_script = FALSE;

	if (ROMlib_desired_bpp)
	  SetDepth (MR (MainDevice), ROMlib_desired_bpp, 0, 0);
	fclose(configfile);
    }

#if !defined (VDRIVER_DISPLAYED_IN_WINDOW)
#define ROMlib_SetTitle(x) do { ROMlib_WindowName = x; } while (0)
#endif

    if (ROMlib_WindowName)
	ROMlib_SetTitle(ROMlib_WindowName);
    else {
	strwidth = fname0;
	newtitle = alloca(strwidth + 1);
	memcpy(newtitle, exefname+1, strwidth);
	newtitle[strwidth] = 0;
	dot = strrchr(newtitle, '.');
	if (dot && (strcmp(dot, ".appl") == 0 ||
		    strcmp(dot, ".APPL") == 0))
	    *dot = 0;
	ROMlib_SetTitle(newtitle);
    }
#if 0
    if (ROMlib_ScreenLocation.first != INITIALPAIRVALUE)
	ROMlib_HideScreen();
    if (ROMlib_ScreenLocation.first != INITIALPAIRVALUE) {
	ROMlib_SetLocation(&ROMlib_ScreenLocation);
	ROMlib_ShowScreen();
    }
#endif

    remalloc (&ROMlib_WindowName);
    remalloc (&ROMlib_Comments);
}

PRIVATE void beginexecutingat( LONGINT startpc )
{
#if defined (SYN68K)
#define ALINETRAPNUMBER       0xA
    trap_install_handler( ALINETRAPNUMBER, alinehandler, (void *) 0);
#endif

    EM_D0 = 0;
    EM_D1 = 0xFFFC000;
    EM_D2 = 0;
    EM_D3 = 0x800008;
    EM_D4 = 0x408;
    EM_D5 = 0x10204000;
    EM_D6 = 0x7F0000;
    EM_D7 = 0x80000800;

    EM_A0 = 0x3EF796;
    EM_A1 = 0x910;
    EM_A2 = EM_D3;
    EM_A3 = 0;
    EM_A4 = 0;
    EM_A5 = CL((LONGINT) CurrentA5);	/* was smashed when we
					   initialized above */
    EM_A6 = 0x1EF;

    CALL_EMULATOR(startpc);
    C_ExitToShell();
}

PUBLIC LONGINT ROMlib_appbit;

size_info_t size_info;

#define VERSFMT		"(0x%02x, 0x%02x, 0x%02x, 0x%02x, %d)"
#define VERSSIZE(vp)	(sizeof(VERSFMT) + (vp)->shortname[0] + 6)
typedef struct {
    unsigned char c[4]		PACKED;
    short loc			PACKED;
    unsigned char shortname[1]	PACKED;
} vers_t;

LONGINT ROMlib_creator;

PUBLIC void *ROMlib_foolgcc;	/* to force the alloca to be done */

PUBLIC void SFSaveDisk_Update (INTEGER vrefnum, Str255 filename)
{
  ParamBlockRec pbr;
  Str255 save_name;

  str255assign (save_name, filename);
  pbr.volumeParam.ioNamePtr = (Ptr) RM ((long) save_name);
  pbr.volumeParam.ioVolIndex = CWC (-1);
  pbr.volumeParam.ioVRefNum = CW (vrefnum);
  PBGetVInfo (&pbr, FALSE);
  SFSaveDisk = CW (-CW (pbr.volumeParam.ioVRefNum));
}

PUBLIC uint32 ROMlib_version_long;

PRIVATE void
cfm_launch (Handle cfrg0, OSType desired_arch, FSSpecPtr fsp)
{
  cfrg_resource_t *cfrgp;
  int n_descripts;
  cfir_t *cfirp;
  OSType desired_arch_x;

  cfrgp = (cfrg_resource_t *) STARH (cfrg0);
  cfirp = (cfir_t *) ((char *) cfrgp + sizeof *cfrgp);
  desired_arch_x = CL (desired_arch);
  for (n_descripts = CFRG_N_DESCRIPTS (cfrgp);
       n_descripts > 0 && CFIR_ISA_X (cfirp) != desired_arch_x;
       --n_descripts, cfirp = (cfir_t *) ((char *) cfirp + CFIR_LENGTH(cfirp)))
    ;

#if defined (powerpc) || defined (__ppc__)
  if (CFIR_ISA_X(cfirp) == desired_arch_x)
    {
      Ptr mainAddr;
      Str255 errName;
      ConnectionID c_id;

#warning were ignoring a lot of the cfir attributes
      GetDiskFragment (fsp, CFIR_OFFSET_TO_FRAGMENT (cfirp),
		       CFIR_FRAGMENT_LENGTH (cfirp), "",
		       kLoadLib,
		       &c_id,
		       &mainAddr,
		       errName);
      {
	uint32 new_toc;
	void *new_pc;
	
	new_toc = ((uint32 *)mainAddr)[1];
	new_pc  = ((void **)mainAddr)[0];
	ppc_call (new_toc, new_pc, 0);
      }
    }
#endif

  C_ExitToShell ();
}

PUBLIC int ROMlib_uaf;

PRIVATE void launchchain(StringPtr fName, INTEGER vRefNum, BOOLEAN resetmemory,
			 LaunchParamBlockRec *lpbp)
{
    OSErr err;
    FInfo finfo;
    Handle code0;
    Handle cfrg0;
    Handle h;
    vers_t *vp;
    char *versbuf, *namebuf;
    int namelen;
    LONGINT abovea5, belowa5, jumplen, jumpoff, *lp;
    INTEGER exevrefnum, toskip;
    Byte *p;
    WDPBRec wdpb;
    StringPtr ename;
    INTEGER elen;

    for (p = fName + fName[0] + 1; p > fName && *--p != ':';)
	;
    toskip = p - fName;
    CurApName[0] = MIN(fName[0]-toskip, 31);
    BlockMove((Ptr) fName+1+toskip, (Ptr) CurApName+1,
						      (Size) CurApName[0]);
#if 0
    Munger(MR(AppParmHandle), 2L*sizeof(INTEGER), (Ptr) 0,
				  (LONGINT) sizeof(AppFile), (Ptr) "", 0L);
#endif
    if (!lpbp || lpbp->launchBlockID != CWC (extendedBlock))
      {
	CInfoPBRec hpb;

	hpb.hFileInfo.ioNamePtr   = RM(&fName[0]);
	hpb.hFileInfo.ioVRefNum   = CW(vRefNum);
	hpb.hFileInfo.ioFDirIndex = CWC (0);
	hpb.hFileInfo.ioDirID     = 0;
	PBGetCatInfo(&hpb, FALSE);
	wdpb.ioVRefNum = CW(vRefNum);
	wdpb.ioWDDirID = hpb.hFileInfo.ioFlParID;
      }
    else
      {
	FSSpecPtr fsp;

	fsp = MR (lpbp->launchAppSpec);
	wdpb.ioVRefNum = fsp->vRefNum;
	wdpb.ioWDDirID = fsp->parID;
      }
    /* Do not do this -- Loser does it SFSaveDisk_Update (vRefNum, fName); */
    wdpb.ioWDProcID = T('X','c','t','r');
    wdpb.ioNamePtr = 0;
    PBOpenWD(&wdpb, FALSE);
    exevrefnum = CW(wdpb.ioVRefNum);
    ROMlib_exefname = CurApName;
#if 0
/* I'm skeptical that this is correct */
    if (CurMap != SysMap)
	CloseResFile(CurMap);
#endif
    SetVol((StringPtr) 0, exevrefnum);
    CurApRefNum = CW(OpenResFile(ROMlib_exefname));

    /* setupsignals(); */

    err = GetFInfo(ROMlib_exefname, exevrefnum, &finfo);
    
    process_create (FALSE, finfo.fdType, finfo.fdCreator);
    
    if (ROMlib_exeuname)
      free (ROMlib_exeuname);
    ROMlib_exeuname = ROMlib_newunixfrommac((char *) ROMlib_exefname+1,
							   ROMlib_exefname[0]);
    elen = strlen(ROMlib_exeuname);
    ename = (StringPtr) alloca(elen+1);
    BlockMove((Ptr) ROMlib_exeuname, (Ptr) ename+1, elen);
    ename[0] = elen;

    ROMlib_creator = CL(finfo.fdCreator);

#define LEMMINGSHACK
#if defined(LEMMINGSHACK)
    {
      if (finfo.fdCreator == CL(TICK("Psyg"))
	  || finfo.fdCreator == CL(TICK("Psod")))
	ROMlib_flushoften = TRUE;
    }
#endif /* defined(LEMMINGSHACK) */

#if defined (ULTIMA_III_HACK)
      ROMlib_ultima_iii_hack = (finfo.fdCreator == CL(TICK("Ult3")));
#endif

    h = GetResource(T('v','e','r','s'), 2);
    if (!h)
	h = GetResource(T('v','e','r','s'), 1);

    ROMlib_version_long = 0;
    if (h) {
	vp = (vers_t *) STARH(h);
	versbuf = alloca(VERSSIZE(vp));
	memcpy(versbuf, vp->shortname+1, vp->shortname[0]);
	sprintf(versbuf+vp->shortname[0], VERSFMT,
					      (LONGINT) vp->c[0],
					      (LONGINT) vp->c[1],
					      (LONGINT) vp->c[2],
					      (LONGINT) vp->c[3],
					      (LONGINT) vp->loc);
	ROMlib_version_long = ((vp->c[0] << 24)|
			       (vp->c[1] << 16)|
			       (vp->c[2] <<  8)|
			       (vp->c[3] <<  0));
    } else
	versbuf = "";

    ROMlib_ScreenSize.first = INITIALPAIRVALUE;
    ROMlib_MacSize.first    = INITIALPAIRVALUE;
    ROMlib_directdiskaccess = FALSE;
    ROMlib_clear_gestalt_list ();
    ParseConfigFile ((StringPtr) "\017ExecutorDefault", 0);
    ParseConfigFile (ename, err == noErr ? CL(finfo.fdCreator) : 0);
    ROMlib_clockonoff(!ROMlib_noclock);
    if ((ROMlib_ScreenSize.first != INITIALPAIRVALUE
	 || ROMlib_MacSize.first != INITIALPAIRVALUE))
      {
	if (ROMlib_ScreenSize.first == INITIALPAIRVALUE)
	  ROMlib_ScreenSize = ROMlib_MacSize;
	if (ROMlib_MacSize.first == INITIALPAIRVALUE)
	  ROMlib_MacSize = ROMlib_ScreenSize;
      }
    code0 = Get1Resource (T('C','O','D','E'), 0);
    cfrg0 = Get1Resource (T('c','f','r','g'), 0);

    if (cfrg0 && ppc_launch_p)
      code0 = NULL;
    else if (!code0)
      {
	if (cfrg0)
	  ParamText ("\062CFM-requiring binaries are not currently supported",
		     0, 0, 0);
	else
	  ParamText ("\061This binary appears damaged (lacks CODE and cfrg)",
		     0, 0, 0);
	NoteAlert (GENERIC_COMPLAINT_ID, (ProcPtr) 0);
	C_ExitToShell ();
      }
    
    {
      Handle size_resource_h;
      int16 size_flags;
      
      size_resource_h = Get1Resource (T ('S','I','Z','E'), 0);
      if (size_resource_h == NULL)
	size_resource_h = Get1Resource (T ('S','I','Z','E'), -1);
      if (size_resource_h)
	{
	  size_info_t *size_resource;
	  
	  size_resource = (size_info_t *) STARH (size_resource_h);
	  size_info.size_flags = CW (size_resource->size_flags);
	  size_info.preferred_size = CL (size_resource->preferred_size);
	  size_info.minimum_size = CL (size_resource->minimum_size);
	  size_info.size_resource_present_p = TRUE;
	}
      else
	{
	  memset (&size_info, '\000', sizeof size_info);
	  size_info.size_resource_present_p = FALSE;
	}
      size_info.application_p = TRUE;
      
      size_flags = size_info.size_flags;
      
      /* we don't accept open app events until a handler is installed */
      application_accepts_open_app_aevt_p = FALSE;
      send_application_open_aevt_p
	= system_version >= 0x700
	  && ((size_flags & SZisHighLevelEventAware)
	      == SZisHighLevelEventAware);
    }
    
    h = GetResource(CL(finfo.fdCreator), 0);
    if (h) {
	namelen = *MR(*(unsigned char **)h);
	namebuf = alloca(namelen+1);
	memcpy(namebuf, (char *) STARH(h)+1, namelen);
	namebuf[namelen] = 0;
    } else
	namebuf = "";

    if (!code0)
      {
	lp = 0; /* just to shut GCC up */
	jumplen = jumpoff = 0; /* just to shut GCC up */
      }
    else
      {
	HLock(code0);

	lp = (LONGINT *) STARH(code0);
	abovea5 = CL(*lp++);
	belowa5 = CL(*lp++);
	jumplen = CL(*lp++);
	jumpoff = CL(*lp++);

	/*
	 * NOTE: The stack initialization code that was here has been moved
	 *	     to ROMlib_InitZones in mman.c
	 */
/* #warning Stack is getting reinitialized even when Chain is called ... */

#if defined(SYN68K)
	EM_A7 -= abovea5 + belowa5;
	CurStackBase = (Ptr) CL(EM_A7);
#else /* !defined(SYN68K) */
	ROMlib_foolgcc = alloca(abovea5 + belowa5);
	CurStackBase
	  = CL(MAC_STACK_START + MAC_STACK_SIZE - (abovea5 + belowa5));
#endif /* !defined(SYN68K) */

	CurrentA5 = RM(MR(CurStackBase) + belowa5); /* set CurrentA5 */
	BufPtr = RM(MR(CurrentA5) + abovea5);
	CurJTOffset = CW(jumpoff);
	a5 = CL((LONGINT) CurrentA5);
      }

    GetDateTime((LONGINT *) &Time);
    ROMBase = RM((Ptr) ROMlib_phoneyrom);
    dodusesit = ROMBase;
    QDExist = WWExist = EXIST_NO;
    TheZone = ApplZone;
    ROMlib_memnomove_p = TRUE;

#if  defined(NEXTSTEP)
    ROMlib_startapp();
#endif
/*
 * NOTE: this memcpy has to be done after all local variables have been used
 *	 because it will quite possibly smash stuff that's on our stack.
 *	 In reality, we only see this if we compile without optimization,
 *	 but trust me, it was *very* confusing when this memcpy was up
 *	 before we were done with our local varibles.
 */
    if (code0)
      {
	memcpy(MR(CurrentA5) + jumpoff, lp, jumplen); /* copy in the
							 jump table */
	ROMlib_destroy_blocks (0, ~0, FALSE);
#if defined(ONLY_DESTROY_BETWEEN_CODE_SEGMENTS)
	ROMlib_num_code_resources = 0;  /* Force a recompute. */
#endif
      }
    SetCursor(STARH(GetCursor(watchCursor)));

   /* Call this routine in case the refresh value changed, either just
    * now or when the config file was parsed.  We want to do this
    * before the screen's depth might be changed.
    */
    {
      int save_ROMlib_refresh = ROMlib_refresh;
      dequeue_refresh_task ();
      set_refresh_rate (0);
      set_refresh_rate (save_ROMlib_refresh);
    }

    ROMlib_uaf = 0;

    if (code0)
      beginexecutingat(CL((LONGINT) CurrentA5) + CW(CurJTOffset) + 2);
    else
      {
	FSSpec fs;

	FSMakeFSSpec (exevrefnum, 0, ROMlib_exefname, &fs);
	cfm_launch (cfrg0, T('p','w','p','c'), &fs);
      }
}

A2(PUBLIC trap, void, Chain, StringPtr, fName, INTEGER, vRefNum)
{
    launchchain(fName, vRefNum, FALSE, 0);
}

PRIVATE void reset_low_globals(void)
{
/*
 * we're about to smash applzone ... we may want to verify a few low-mem
 * globals beforehand
 */

    ProcPtr saveDABeeper;
    THz saveSysZone;
    uint32 saveTicks;
    INTEGER saveBootDrive;
    LONGINT saveLo3Bytes;
    LONGINT save20, save28, save58, save5C;
    Ptr saveSoundBase;
    Ptr saveVIA;
    Ptr saveSCCRd;
    Ptr saveSCCWr;
    Handle saveAppParmHandle;
    QHdr saveVCBQHdr;
    QHdr saveFSQHdr;
    QHdr saveDrvQHdr;
    QHdr saveEventQueue;
    QHdr saveVBLQueue;
    Ptr saveFCBSPtr;
    Ptr saveWDCBsPtr;
    LONGINT saveCurDirStore;
    INTEGER saveSFSaveDisk;
    VCBPtr saveDefVCBPtr;
    char saveCurApName[sizeof(CurApName)];
    INTEGER saveCurApRefNum;
    INTEGER saveCurMap;
    Handle saveTopMapHndl;
    Handle saveSysMapHndl;
    INTEGER saveSysMap;
    LONGINT saveScrapSize;
    Handle saveScrapHandle;
    INTEGER saveScrapCount;
    INTEGER saveScrapState;
    StringPtr saveScrapName;
    Handle saveROMFont0;
    Handle saveWidthListHand;
    Byte saveSPValid;
    Byte saveSPATalkA;
    Byte saveSPATalkB;
    Byte saveSPConfig;
    INTEGER saveSPPortA;
    INTEGER saveSPPortB;
    LONGINT saveSPAlarm;
    INTEGER saveSPFont;
    Byte saveSPKbd;
    Byte saveSPPrint;
    Byte saveSPVolCtl;
    Byte saveSPClikCaret;
    Byte saveSPMisc2;
    INTEGER saveKeyThresh;
    INTEGER saveKeyRepThresh;
    INTEGER saveMenuFlash;
    LONGINT saveCaretTime;
    LONGINT saveDoubleTime;
    LONGINT saveDefDirID;
    HIDDEN_Handle saveDAStrings[4];
    Ptr saveMemTop;
    DCtlHandlePtr saveUTableBase;
    INTEGER saveUnitNtryCnt;
    Point saveMouseLocation;
    CGrafPtr saveWMgrCPort;
    Handle saveMBDFHndl;
    ProcPtr saveJCrsrTask;
    
    AE_info_t *saveAE_info;
    
    RGBColor saveHiliteRGB;
    GDHandle saveTheGDevice, saveMainDevice, saveDeviceList;
    char saveKeyMap[sizeof_KeyMap];
    
    Byte saveFinderName[sizeof(FinderName)];
    virtual_int_state_t bt;
    
    bt = block_virtual_ints ();
	saveSysZone       = SysZone;
	saveTicks         = Ticks;
	saveBootDrive     = BootDrive;
	saveLo3Bytes      = Lo3Bytes;
	save20            = *(LONGINT *) SYN68K_TO_US(0x20);
	save28            = *(LONGINT *) SYN68K_TO_US(0x28);
	save58            = *(LONGINT *) SYN68K_TO_US(0x58);
	save5C            = *(LONGINT *) SYN68K_TO_US(0x5C);
	saveVIA           = VIA;
	saveSCCRd	  = SCCRd_H.p;
	saveSCCWr	  = SCCWr_H.p;
	saveSoundBase     = SoundBase;
	saveAppParmHandle = AppParmHandle;
	saveVCBQHdr       = VCBQHdr;
	saveFSQHdr        = FSQHdr;
	saveDrvQHdr       = DrvQHdr;
	saveFCBSPtr       = FCBSPtr;
	saveWDCBsPtr      = WDCBsPtr;
	saveSFSaveDisk    = SFSaveDisk;
	saveCurDirStore   = CurDirStore;
	saveEventQueue    = EventQueue;
	saveVBLQueue      = VBLQueue;
	saveDefVCBPtr     = DefVCBPtr;
	memcpy(saveCurApName, CurApName, sizeof(CurApName));
	saveCurApRefNum   = CurApRefNum;
	saveCurMap	  = CurMap;
	saveTopMapHndl    = TopMapHndl;
	saveSysMapHndl    = SysMapHndl;
	saveSysMap        = SysMap;
	saveScrapSize     = ScrapSize;
	saveScrapHandle   = ScrapHandle;
	saveScrapCount    = ScrapCount;
	saveScrapState    = ScrapState;
	saveScrapName     = ScrapName;
	saveROMFont0	  = ROMFont0;
	saveWidthListHand = WidthListHand;
	saveSPValid	  = SPValid;
	saveSPATalkA	  = SPATalkA;
	saveSPATalkB	  = SPATalkB;
	saveSPConfig	  = SPConfig;
	saveSPPortA	  = SPPortA;
	saveSPPortB	  = SPPortB;
	saveSPAlarm	  = SPAlarm;
	saveSPFont	  = SPFont;
	saveSPKbd	  = SPKbd;
	saveSPPrint	  = SPPrint;
	saveSPVolCtl	  = SPVolCtl;
	saveSPClikCaret	  = SPClikCaret;
	saveSPMisc2	  = SPMisc2;
	saveKeyThresh	  = KeyThresh;
	saveKeyRepThresh  = KeyRepThresh;
	saveMenuFlash	  = MenuFlash;
	saveCaretTime	  = CaretTime;
	saveDoubleTime	  = DoubleTime;
	saveDefDirID	  = DefDirID;
    
        saveHiliteRGB = HiliteRGB;
        saveTheGDevice = TheGDevice;
        saveMainDevice = MainDevice;
        saveDeviceList = DeviceList;    
	saveDAStrings[0]  = DAStrings_H[0];
	saveDAStrings[1]  = DAStrings_H[1];
	saveDAStrings[2]  = DAStrings_H[2];
	saveDAStrings[3]  = DAStrings_H[3];
	saveMemTop = MemTop;
	saveUTableBase = UTableBase;
	saveUnitNtryCnt = UnitNtryCnt;

	saveMouseLocation = MouseLocation;
        saveDABeeper = DABeeper;
    
        memcpy(saveFinderName, FinderName, sizeof(saveFinderName));
        saveWMgrCPort = WMgrCPort;
        saveMBDFHndl = MBDFHndl;

	saveJCrsrTask = JCrsrTask;
	
	saveAE_info = AE_info;
	memcpy (saveKeyMap, KeyMap, sizeof_KeyMap);
	
	/* Set low globals to 0xFF, but don't touch exception vectors. */
	memset ((char *)&nilhandle + 64 * sizeof (ULONGINT),
		0xFF,
		((char *)&lastlowglobal - (char *)&nilhandle
		 - 64 * sizeof (ULONGINT)));
	
	AE_info = saveAE_info;
	
	JCrsrTask = saveJCrsrTask;
	
        MBDFHndl = saveMBDFHndl;
        WMgrCPort = saveWMgrCPort;
        WindowList = NULL;
        memcpy(FinderName, saveFinderName, sizeof(FinderName));

        DABeeper = saveDABeeper;
    	MouseLocation = saveMouseLocation;
        MouseLocation2 = saveMouseLocation;
    
	UTableBase   = saveUTableBase;
	UnitNtryCnt  = saveUnitNtryCnt;
	MemTop = saveMemTop;
	DAStrings_H[3]  = saveDAStrings[3];
	DAStrings_H[2]  = saveDAStrings[2];
	DAStrings_H[1]  = saveDAStrings[1];
	DAStrings_H[0]  = saveDAStrings[0];
	DefDirID      = saveDefDirID;
        DoubleTime    = saveDoubleTime;
        CaretTime     = saveCaretTime;
        MenuFlash     = saveMenuFlash;
        KeyRepThresh  = saveKeyRepThresh;
        KeyThresh     = saveKeyThresh;
        SPMisc2       = saveSPMisc2;
        SPClikCaret   = saveSPClikCaret;
        SPVolCtl      = saveSPVolCtl;
        SPPrint       = saveSPPrint;
        SPKbd         = saveSPKbd;
        SPFont        = saveSPFont;
        SPAlarm       = saveSPAlarm;
        SPPortB       = saveSPPortB;
        SPPortA       = saveSPPortA;
        SPConfig      = saveSPConfig;
        SPATalkB      = saveSPATalkB;
        SPATalkA      = saveSPATalkA;
        SPValid       = saveSPValid;
	WidthListHand = saveWidthListHand;
	ROMFont0      = saveROMFont0;
	ScrapName     = saveScrapName;
	ScrapState    = saveScrapState;
	ScrapCount    = saveScrapCount;
	ScrapHandle   = saveScrapHandle;
	ScrapSize     = saveScrapSize;
	SysMap        = saveSysMap;
	SysMapHndl    = saveSysMapHndl;
	TopMapHndl    = saveTopMapHndl;
	CurMap	      = saveCurMap;
	CurApRefNum   = saveCurApRefNum;
	memcpy(CurApName, saveCurApName, sizeof(CurApName));
	DefVCBPtr     = saveDefVCBPtr;
	VBLQueue      = saveVBLQueue;
	EventQueue    = saveEventQueue;
	CurDirStore   = saveCurDirStore;
	SFSaveDisk    = saveSFSaveDisk;
	WDCBsPtr      = saveWDCBsPtr;
	FCBSPtr       = saveFCBSPtr;
	DrvQHdr       = saveDrvQHdr;
	FSQHdr        = saveFSQHdr;
	VCBQHdr       = saveVCBQHdr;
	Lo3Bytes      = saveLo3Bytes;
	VIA           = saveVIA;
	SCCRd_H.p     = saveSCCRd;
	SCCWr_H.p     = saveSCCWr;
	SoundBase     = saveSoundBase;
	Ticks_UL.u    = saveTicks;
	SysZone       = saveSysZone;
	BootDrive     = saveBootDrive;
	AppParmHandle = saveAppParmHandle;

        HiliteRGB = saveHiliteRGB;
        TheGDevice = saveTheGDevice;
        MainDevice = saveMainDevice;
        DeviceList = saveDeviceList;    
    
    restore_virtual_ints (bt);

    nilhandle   = 0;	/* so nil dereferences "work" */

    CrsrBusy    = 0;
    TESysJust   = 0;
    DSAlertTab  = 0;
    ResumeProc  = 0;
    GZRootHnd   = 0;
    ANumber     = 0;
    ResErrProc  = 0;
#if 0
    FractEnable = 0xff;	/* NEW MOD -- QUESTIONABLE */
#else
    FractEnable = 0;
#endif
    SEvtEnb     = 0;
    MenuList    = 0;
    MBarEnable  = 0;
    MenuFlash   = 0;
    TheMenu     = 0;
    MBarHook    = 0;
    MenuHook    = 0;
    HeapEnd     = 0;
    ApplLimit   = 0;
    SoundActive = soundactiveoff;
    PortBUse    = 2;	/* configured for Serial driver */

    memcpy (KeyMap, saveKeyMap, sizeof_KeyMap);
    OneOne      = CLC(0x00010001);
    DragHook    = 0;
    MBDFHndl    = 0;
    MenuList    = 0;
    MBSaveLoc   = 0;
    SysFontFam  = 0;

    SysVersion  = CW (system_version);
    FSFCBLen    = CWC (94);

/*
 * TODO:  how does this relate to Launch?
 */
    /* Set up default floating point environment. */
    {
      INTEGER env = 0;
      ROMlib_Fsetenv (&env, 0);
    }

    TEDoText = RM((ProcPtr) P_ROMlib_dotext);	/* where should this go ? */

    WWExist = QDExist = EXIST_NO;	/* TODO:  LOOK HERE! */
    SCSIFlags = CWC (0xEC00); /* scsi+clock+xparam+mmu+adb
				 (no fpu,aux or pwrmgr) */

    MMUType = 5;
    KbdType = 2;

    MCLKPCmiss1 = 0;	/* &MCLKPCmiss1 = 0x358 + 72 (MacLinkPC starts
			   adding the 72 byte offset to VCB pointers too
			   soon, starting with 0x358, which is not the
			   address of a VCB) */
			   
    MCLKPCmiss2 = 0;	/* &MCLKPCmiss1 = 0x358 + 78 (MacLinkPC misses) */
    AuxCtlHead = 0;
    CurDeactive = 0;
    CurActivate = 0;
    macfpstate[0] = 0;
    fondid = 0;
    PrintErr = 0;
    mouseoffset = 0;
    heapcheck = 0;
    DefltStack = CLC(0x2000);	/* nobody really cares about these two */
    MinStack = CLC(0x400);		/* values ... */
    IAZNotify = 0;
    CurPitch = 0;
    JSwapFont = (ProcPtr) RM(P_FMSwapFont);
    JInitCrsr = (ProcPtr) RM(P_InitCursor);

    JHideCursor = (ProcPtr) RM(P_HideCursor);
    JShowCursor = (ProcPtr) RM(P_ShowCursor);
    JShieldCursor = (ProcPtr) RM(P_ShieldCursor);
    JSetCrsr = (ProcPtr) RM(P_SetCursor);
    JCrsrObscure = (ProcPtr) RM(P_ObscureCursor);

#if 0
    JUnknown574 = (ProcPtr) RM(P_Unknown574);
#else
    *(long *)(0x574 + ROMlib_offset) = (long) RM(P_Unknown574);
#endif

    Key1Trans = (Ptr) RM(P_Key1Trans);
    Key2Trans = (Ptr) RM(P_Key2Trans);
    JFLUSH = (ProcPtr) RM(P_flushcache);
    JResUnknown1 = JFLUSH;	/* I don't know what these are supposed to */
    JResUnknown2 = JFLUSH;	/* do, but they're not called enough for
				   us to worry about the cache flushing
				   overhead */

    CPUFlag = 4;	/* mc68040 */
    UnitNtryCnt = 0;	/* how many units in the table */

    TheZone = ApplZone;

#if !defined(SYN68K)
    bzero((char *) &nilhandle, 0xc0);	/* exception vectors */
					/* pagemaker 2.0 looks at 108
					   and doesn't want to see -1 */
#endif /* !defined(SYN68K) */
    *(LONGINT *) SYN68K_TO_US(0x20) = save20;
    *(LONGINT *) SYN68K_TO_US(0x28) = save28;
    *(LONGINT *) SYN68K_TO_US(0x58) = save58;
    *(LONGINT *) SYN68K_TO_US(0x5C) = save5C;

    HiliteMode = CB(0xFF);	/* I think this is correct */
    ROM85 = CWC(0x3FFF);	/* We be color now */
    MMU32Bit = 0x01;
    loadtrap = 0;
    *(LONGINT *) SYN68K_TO_US(0x1008) = CLC (0x4); /* Quark XPress 3.0 references 0x1008
					explicitly.  It takes the value
					found there, subtracts four from
					it and dereferences that value.
					Yahoo */
    *(short *) SYN68K_TO_US(4) = CWC (0x4e75); /* RTS, so when we dynamically recompile
				    code starting at 0 we won't get far */

    /* Micro-cap dereferences location one of the AppPacks locations */

    {
      int i;
      
      for (i = 0; i < (int) NELEM (AppPacks_H); ++i)
	AppPacks_H[i].p = 0;
    }
    SysEvtMask = CWC(~(1L<< keyUp)); /* EVERYTHING except keyUp */
    SdVolume = 7; /* for Beebop 2 */
    CurrentA5 = (Ptr) CL (EM_A5);
}

PRIVATE void reset_traps(void)
{
    static void *savetooltraptable[0x400];
    static void   *saveostraptable[0x100];
    static BOOLEAN beenhere = FALSE;

    ROMlib_reset_bad_trap_addresses ();
    if (!beenhere) {
	memcpy(  saveostraptable,   ostraptable, sizeof(  saveostraptable));
	memcpy(savetooltraptable, tooltraptable, sizeof(savetooltraptable));
	beenhere = TRUE;
    } else {
/*
 * NOTE: I'm not preserving patches that go into the SystemZone.  Right now
 *       that just seems like an unnecessary source of mystery bugs.
 */
	memcpy(  ostraptable,   saveostraptable, sizeof(  saveostraptable));
	memcpy(tooltraptable, savetooltraptable, sizeof(savetooltraptable));
    }
}

PRIVATE boolean_t
our_special_map (resmaphand map)
{
  boolean_t retval;
  Handle h;

  CurMap = STARH(map)->resfn;
  h = Get1Resource (TICK("nUSE"), 0);
  retval = h ? TRUE : FALSE;

  return retval;
}


void
empty_timer_queues (void)
{
  TMTask *tp, *nexttp;
  VBLTaskPtr vp, nextvp;
  virtual_int_state_t bt;

  bt = block_virtual_ints ();

  dequeue_refresh_task ();
  clear_pending_sounds ();
  for (vp = (VBLTaskPtr) MR (VBLQueue.qHead); vp; vp = nextvp)
    {
      nextvp = (VBLTaskPtr) MR (vp->qLink);
      VRemove (vp);
    }
  for (tp = MR ((TMTask *) ROMlib_timehead.qHead); tp; tp = nexttp)
    {
      nexttp = (TMTask *) MR (tp->qLink);
      RmvTime ((QElemPtr) tp);
    }

  restore_virtual_ints (bt);
}


PRIVATE void reinitialize_things(void)
{
    resmaphand map, nextmap;
    filecontrolblock *fcbp, *efcbp;
    INTEGER length;
    INTEGER special_fn;
    int i;

    ROMlib_shutdown_font_manager ();
    SetZone (MR(SysZone));
    /* NOTE: we really shouldn't be closing desk accessories at all, but
       since we don't properly handle them when they're left open, it is
       better to close them down than not.  */

    for (i = DESK_ACC_MIN; i <= DESK_ACC_MAX; ++i)
      CloseDriver (-i - 1);

    empty_timer_queues ();
    ROMlib_clock = 0;	/* CLOCKOFF */

    special_fn = 0;
    for (map = (resmaphand) MR(TopMapHndl); map; map = nextmap) {
	nextmap = (resmaphand) HxP(map, nextmap);
	if (HxX(map, resfn) == SysMap)
	  UpdateResFile (Hx (map, resfn));
	else
	  {
	    if (!our_special_map (map))
	      CloseResFile(Hx(map, resfn));
	    else
	      {
		special_fn = Hx(map, resfn);
		UpdateResFile (special_fn);
	      }
	  }
    }

    length = CW(*(short *)MR(FCBSPtr));
    fcbp = (filecontrolblock *) ((short *)MR(FCBSPtr)+1);
    efcbp = (filecontrolblock *) ((char *)MR(FCBSPtr) + length);
    for (;fcbp < efcbp;
		     fcbp = (filecontrolblock *) ((char *)fcbp + CW(FSFCBLen)))
      {
	INTEGER rn;

	rn = (char *) fcbp - (char *) MR(FCBSPtr);
	if (fcbp->fcbCName[0]
	    /* && rn != Param_ram_rn */
	    && rn != CW (SysMap)
	    && rn != special_fn)
	  FSClose((char *) fcbp - (char *) MR(FCBSPtr));
      }

    CurMap = STARH((resmaphand)MR(TopMapHndl))->resfn;

    /* TODO replace the code in main.c with this stuff */
#if !defined(MSDOS)
/*
 * In the GO32 environment we're not guaranteed to have a floating point
 * coprocessor, so we delay floating point environment initialization until
 * we actually use it.
 */
    /* Set up default floating point environment. */
    {
      INTEGER env = 0;
      ROMlib_Fsetenv (&env, 0);
    }
#endif

    ROMlib_destroy_blocks (0, ~0, FALSE);
#if defined(ONLY_DESTROY_BETWEEN_CODE_SEGMENTS)
    ROMlib_num_code_resources = 0;  /* Force a recompute. */
#endif
}

PUBLIC void
NewLaunch (StringPtr fName_arg, INTEGER vRefNum_arg, LaunchParamBlockRec *lpbp)
{
    static char beenhere = FALSE;
    static jmp_buf buf;
    static Str255 fName;
    static INTEGER vRefNum;
    static LaunchParamBlockRec lpb;

    if (lpbp && lpbp->launchBlockID == CWC (extendedBlock))
      {
	lpb = *lpbp;
	str255assign (fName, (MR (lpbp->launchAppSpec))->name);
      }
    else
      {
	lpb.launchBlockID = 0;
	str255assign (fName, fName_arg);
	vRefNum = vRefNum_arg;
      }
    
    /* This setjmp/longjmp code might be better put in launchchain */
    if (!beenhere)
      {
	beenhere = TRUE;
	setjmp(buf);
      }
    else
      longjmp(buf, 1);

    reset_adb_vector ();
    reinitialize_things();
    reset_traps();
    reset_low_globals();
    ROMlib_InitZones(offset_no_change); /* too late to change memory offset */
    ROMlib_InitGWorlds();
    hle_reinit ();
    AE_reinit ();
    print_reinit ();

    gd_set_bpp (MR (MainDevice), !vdriver_grayscale_p, vdriver_fixed_clut_p,
		vdriver_bpp);
    ROMlib_init_stdfile();
#if ERROR_SUPPORTED_P (ERROR_UNEXPECTED)
    if (ERROR_ENABLED_P (ERROR_UNEXPECTED))
      {
	long lp;

	for (lp = (long) &nilhandle_H; lp <= (long) &lastlowglobal; lp += 2)
	    if (   lp != (long) &TheZone_H
		&& lp != (long) &ApplZone_H
		&& lp != (long) &FSFCBLen
		&& lp != (long) &SysMap
		&& lp != (long) SYN68K_TO_US(0x2f6)
		&& lp != (long) SYN68K_TO_US(0x8e6)
		&& lp != (long) SYN68K_TO_US(0x900)
		&& lp != (long) &CurMap
		&& lp != (long) SYN68K_TO_US(0x8a6)
		&& lp != (long) SYN68K_TO_US(0x8aa)
		&& lp != (long) SYN68K_TO_US(0x268)
		&& lp != (long) SYN68K_TO_US(0x982)
		&& lp != (long) SYN68K_TO_US(0xaee)
		&& lp != (long) SYN68K_TO_US(0xcca)
		&& lp != (long) SYN68K_TO_US(0xd50)
		&& lp != (long) SYN68K_TO_US(0x18e)
		&& lp != (long) SYN68K_TO_US(0x190)
		&& lp != (long) SYN68K_TO_US(0x2f2)
		&& lp != (long) SYN68K_TO_US(0x11e)
		&& lp != (long) SYN68K_TO_US(0x15c)
		&& lp != (long) SYN68K_TO_US(0x27e)
		&& lp != (long) SYN68K_TO_US(0x31a)
		&& lp != (long) SYN68K_TO_US(0x82c)
		&& lp != (long) SYN68K_TO_US(0x82e)
		&& lp != (long) SYN68K_TO_US(0x830)
		&& lp != (long) SYN68K_TO_US(0x832)
		&& lp != (long) SYN68K_TO_US(0xa4a)
		&& lp != (long) SYN68K_TO_US(0xa52)
		&& lp != (long) SYN68K_TO_US(0xa56)
		&& lp != (long) SYN68K_TO_US(0xbf4)
		&& lp != (long) SYN68K_TO_US(0x828)
		&& lp != (long) SYN68K_TO_US(0x82a)
		&& lp != (long) SYN68K_TO_US(0x16c)
		)
		if (MR(*(int32 *)lp) >= (int32) MR(ApplZone)
		    && MR(*(int32 *)lp) < (int32) MR(MR(ApplZone)->bkLim))
		  warning_unexpected ("Low global at 0x%lx may point into "
				      "ApplZone and probably shouldn't.",
				      (unsigned long) US_TO_SYN68K (lp));
      }
#endif
    launchchain(fName, vRefNum, TRUE, &lpb);
}

A2(PUBLIC trap, void, Launch, StringPtr, fName_arg, INTEGER, vRefNum_arg)
{
#if 0
  /* NOTE: we're messing with launch between Executor 2 beta 1 and Executor 2
     proper.  We really shouldn't do anything differently from what we were
     doing before, so we don't have the liberty to do things cleanly */

  LaunchParamBlockRec pbr;

  memset (&pbr, 0, sizeof pbr);
  pbr.launchBlockID = CWC (extendedBlock);
  pbr.launchEPBLength = CLC (extendedBlockLen);
  pbr.launchControlFlags = CWC (launchNoFileFlags|launchInhibitDaemon);
  FSMakeFSSpec (vRefNum_arg, 0, fName_arg, &pbr.launchAppSpec);
  pbr.launchAppSpec.vRefNum = vRefNum_arg);
  NewLaunch (&pbr);
#else
  NewLaunch (fName_arg, vRefNum_arg, 0);
#endif
}
