/* Copyright 1992-1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_executor[] =
	    "$Id: executor.c 88 2005-05-25 03:59:37Z ctm $";
#endif

//#define DEBUG

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
#include "PrintMgr.h"
#include "StartMgr.h"
#include "CommTool.h"

#include "rsys/trapglue.h"
#include "rsys/cquick.h"
#include "rsys/file.h"
#include "rsys/next.h"
#include "rsys/soundopts.h"
#include "rsys/pstuff.h"
#include "rsys/jumpvectors.h"
#include "rsys/prefs.h"
#include "rsys/aboutpanel.h"
#include "rsys/segment.h"
#include "rsys/misc.h"
#include "rsys/host.h"
#include "rsys/executor.h"
#include "rsys/hfs.h"
#include "rsys/float.h"
#include "rsys/vdriver.h"
#include "rsys/trapname.h"

#include "rsys/options.h"
#include "rsys/suffix_maps.h"
#include "rsys/string.h"
#include "rsys/custom.h"

#undef PRIVATE
#define PRIVATE
#define PUBLIC

using namespace Executor;

#define FASTALINETRAPS

/*
 * For now the NEXT handles A-line traps differently than on the Sun.
 * The NEXT will bypass the A-line and priv instruction dispatches if
 * possible (meaning if there's no page fault during the dispatch).  If
 * not possible, we get here and are expected to force the page fault
 * ourselves so we can go back into kernel mode.  Pretty bizzare, eh?
 */

#if defined(NEXT) && !defined(SYN68K)
PRIVATE void emthandler(LONGINT wsignal, LONGINT code, struct sigcontext *scp)
{
    LONGINT saved2;
    volatile LONGINT temp;
    static int beenhere = 0;


    if (!beenhere) {
	ROMlib_load_ardi_mods();
	beenhere = 1;
    }

    asm("movel d2, %0" : "=g" (saved2));

    temp  = *(volatile LONGINT *) 0x5C;	/* will get page 0 and the stack */

    asm("movel %0, d2" : : "g" (saved2));
}
#endif /* NEXT && !SYN68K */


LONGINT Executor::debugnumber, debugtable[1<<12], cutoff = 20;

#if !defined (MSDOS) && !defined (NDEBUG)
typedef struct
{
  unsigned trapno;
  int32 when;
} trap_when_t;


static int
compare_trap_when (const void *p1, const void *p2)
{
  return ((const trap_when_t *)p1)->when - ((const trap_when_t *) p2)->when;
}


/* Prints out the traps executed within the last NUM_TRAPS_BACK traps. */
void
dump_recent_traps (int num_traps_back)
{
  trap_when_t traps[0x1000];
  int i, num_interesting_traps;

  /* Record all the traps that are recent enough. */
  for (i = 0, num_interesting_traps = 0; i < (int) NELEM (traps); i++)
    {
      int32 when;

      when = debugtable[i];
      if (when != 0 && when >= debugnumber - num_traps_back)
	{
	  traps[num_interesting_traps].trapno = 0xA000 + i;
	  traps[num_interesting_traps].when = when;
	  num_interesting_traps++;
	}
    }

  /* Sort them by time, oldest first. */
  qsort (traps, num_interesting_traps, sizeof traps[0], compare_trap_when);

  /* Print them out. */
  for (i = 0; i < num_interesting_traps; i++)
    printf ("0x%04X\t%ld\t%s\n", traps[i].trapno, (long) traps[i].when,
	    trap_name (traps[i].trapno));
}
#endif /* !MSDOS && !NDEBUG */



#if !defined (NDEBUG)

char trap_watchpoint_data[1024];
char *trap_watchpoint_name[1024];
struct
{
  /* pointer to actual memory */
  int handle_p;
  char *x;
  /* pointer to our `orignal' copy */
  char *orig;
  int size;
} trap_watchpoints[1024];
int trap_watchpoint_next_data;
int trap_watchpoint_next;


/* make a per-trap watchpoint of the data at `x', of size `size' */
void
trap_watch (char *name, void *x, int size)
{
  char *trap_data;
  
  if (trap_watchpoint_next == 1024)
    {
      fprintf (stderr, "all available trap watchpoints used\n");
      return;
    }
  trap_watchpoints[trap_watchpoint_next].size = size;
  trap_watchpoints[trap_watchpoint_next].handle_p = FALSE;
  trap_watchpoints[trap_watchpoint_next].x = (char*)x;
  trap_data
    = trap_watchpoints[trap_watchpoint_next].orig
    = &trap_watchpoint_data[trap_watchpoint_next_data];
  trap_watchpoint_next ++;
  trap_watchpoint_next_data += size;

  memcpy (trap_data, x, size);
}

void
trap_watch_handle (char *name, Handle x, int size)
{
  char *trap_data;
  
  if (trap_watchpoint_next == 1024)
    {
      fprintf (stderr, "all available trap watchpoints used\n");
      return;
    }
  trap_watchpoints[trap_watchpoint_next].size = size;
  trap_watchpoints[trap_watchpoint_next].handle_p = TRUE;
  trap_watchpoints[trap_watchpoint_next].x = (char *) x;
  trap_data
    = trap_watchpoints[trap_watchpoint_next].orig
    = &trap_watchpoint_data[trap_watchpoint_next_data];
  trap_watchpoint_next ++;
  trap_watchpoint_next_data += size;

  memcpy (trap_data, STARH ((Handle) x), size);
}

void
trap_break_me (void)
{
  
}

void
trap_dump_bits (const char *msg, char *data, int size)
{
  int i;

  fprintf (stderr, "%s: ", msg);
  for (i = 0; i < size; i ++)
    fprintf (stderr, "%x%x",
	     (int) ((data[i] >> 4) & 0xF),
	     (int) (data[i] & 0xF));
  fprintf (stderr, "\n");
}


int check_trap_watchpoints_p = FALSE;

void
check_trap_watchpoints (char *msg)
{
  int size, i;
  
  for (i = 0; i < 1024; i ++)
    {
      if (trap_watchpoints[i].x)
	{
	  void *x;
	  
	  x = (trap_watchpoints[i].handle_p
	       ? (void *) STARH ((Handle) trap_watchpoints[i].x)
	       : trap_watchpoints[i].x);
	      
	  size = trap_watchpoints[i].size;
	  
	  if (memcmp (x, trap_watchpoints[i].orig,
		      size))
	    {
	      /* differs */
	      fprintf (stderr, "%s", msg);
	      trap_dump_bits ("old", trap_watchpoints[i].orig, size);
	      trap_dump_bits ("new", (char*)x, size);
	      
	      memcpy (trap_watchpoints[i].orig, x, size);
	      
	      trap_break_me ();
	    }
	}
    }
}
#endif /* !NDEBUG */

PRIVATE void setup28( void )
{
#if defined (MACOSX_) && !defined (SYN68K)
#if 1
    static LONGINT trap_dispatcher_callback = -1;

    if (trap_dispatcher_callback == -1)
      trap_dispatcher_callback = callback_install (aline_stub, NULL);
    *(LONGINT *) 0x28 = trap_dispatcher_callback;
#else
    *(LONGINT *) 0x28 = (LONGINT) trapdispatcher;
#endif

    *(LONGINT *) 0x20 = (LONGINT) privdispatcher;
    *(LONGINT *) 0x5C = (LONGINT) T('A','R','D','I');
#endif /* NEXT && !SYN68K */
}

PUBLIC void Executor::setupsignals( void )
{
#if defined (NEXT)
    LONGINT tocatch[] = {
	SIGHUP,
	SIGINT,
	SIGQUIT,
	SIGILL,
	SIGTRAP,
	SIGIOT,
	SIGEMT,
	SIGFPE,
	SIGBUS,
	SIGSEGV,
	SIGSYS,
	SIGPIPE,
	SIGTERM,
	SIGXCPU,
	SIGXFSZ,
	SIGVTALRM,
	SIGPROF,
	SIGUSR1,
	SIGUSR2,
	0
    }, *tocatchp;
#endif /* NEXT */
#if !defined (FASTALINETRAPS) || defined (MACOSX_)
#if defined(USE_BSD_SIGNALS)
    typedef struct sigvec sigvec_t;
    typedef struct sigstack sigstack_t;
#endif
#endif /* FASTALINETRAPS || NEXT */

#if (!defined (FASTALINETRAPS) || defined (MACOSX_)) && !defined (SYN68K)
    {
      sigstack_t sigstackarg;
      sigvec_t sigvecarg;
      static char signalstack[3*1024];

      sigstackarg.ss_sp = signalstack + sizeof(signalstack);
      sigstackarg.ss_onstack = 0;
      sigstack(&sigstackarg, (sigstack_t *) 0);
      sigvecarg.sv_handler = (void *) emthandler;
      sigvecarg.sv_mask = 0;
      sigvecarg.sv_flags = SV_ONSTACK;
      sigvec(SIGEMT, &sigvecarg, (sigvec_t *) 0);
    }
#endif /* (FASTALINETRAPS || NEXT) && !SYN68K */
    setup28();
}

#define TOOLMASK		(0x3FF)
#define OSMASK			(0xFF)
#define DONTSAVEA0BIT		(0x100)
#define POPBIT			(0x400)

#define LOADSEGTRAPWORD		(0xA9F0)
#define EXITTOSHELLTRAPWORD	(0xA9F4)
enum { MODESWITCHTRAPWORD = 0xaafe };

#define ALINEEXCEPTIONFRAMESIZE	8
#define WEIRDMAGIC	(0x1F52)

/*
 * FAKEPascalToCCall pushes a phoney return address and then calls
 * PascalToCCall and throws away the return value.  It will be something
 * like:
 *
 */

#define FAKEPascalToCCall(xxx) \
	do {PUSHADDR(0xEACE4321); (void) PascalToCCall(0x87654231, xxx); } while (0)

unsigned short Executor::mostrecenttrap;

/* #define MEMORY_WATCH */

#include "rsys/mman_private.h"

#if defined (MEMORY_WATCH)

int memory_watch = 0;

PRIVATE void
dump_difference (uint16 trapn, int i,
		 zone_info_t *currentp, const zone_info_t *newp)
{
  fprintf (stderr, "%d %s(%d): D#rel = %d, D#nrel = %d, D#free = %d, Dtotal = %d\n",
	   debugnumber, trap_name (trapn), i, newp->n_rel - currentp->n_rel,
	  newp->n_nrel - currentp->n_nrel, newp->n_free - currentp->n_free,
	  newp->total_free - currentp->total_free);
  *currentp = *newp;
}

PRIVATE void
compare_zone_infos (uint16 trapn, zone_info_t current[3], zone_info_t new[3])
{
  int i;

  for (i = 0; i < 2; ++i)
    {
      if ((ABS (current[i].n_rel      - new[i].n_rel ) > 2) ||
	  (ABS (current[i].n_nrel     - new[i].n_nrel) > 2) ||
	  (ABS (current[i].total_free - new[i].total_free) > 12 * 1024))
	dump_difference (trapn, i, &current[i], &new[i]);
    }
}
#endif

PUBLIC syn68k_addr_t Executor::alinehandler(syn68k_addr_t pc, void *ignored)
{
    syn68k_addr_t retval;
    unsigned short trapno, status;
    uint32 savea0, savea1, savea2, saved1, saved2;
    unsigned short trapword;
    syn68k_addr_t togoto;
#if defined (MEMORY_WATCH)
    zone_info_t current_zone_infos[3];
    zone_info_t new_zone_infos[3];
#endif

#if defined (MEMORY_WATCH)
    if (memory_watch)
      {
	ROMlib_sledgehammer_zones (__PRETTY_FUNCTION__, __FILE__, __LINE__,
				   "after aline", current_zone_infos);
      }
#endif


    mostrecenttrap = trapword = READUW(pc);
    retval = pc + 2;

#if !defined (NDEBUG)
    if (check_trap_watchpoints_p)
      check_trap_watchpoints ("entering `alinehandler ()'\n");
#endif 

/*
 * Code for debugging
 */

#if !defined (NDEBUG)
    debugtable[trapword&0xFFF] = ++debugnumber;
#endif /* !NDEBUG */

#if 0
    warning_trace_info ("in trapword = 0x%x, pc = 0x%x", trapword, pc);
#endif

    status = READUW(EM_A7);
    EM_A7 += ALINEEXCEPTIONFRAMESIZE;
    if (trapword & TOOLBIT) {
	trapno = trapword & TOOLMASK;
	togoto = tooltraptable[trapno];
	if (trapword & POPBIT)
	    retval = POPADDR();
	if (togoto == toolstuff[trapno].orig) {
	    if (toolstuff[trapno].ptoc.magic == (ULONGINT) -1) {
	        syn68k_addr_t new_addr;

		PUSHADDR(0x80014321);	/* better not use this */
		new_addr =
		  (*(callback_handler_t)toolstuff[trapno].ptoc.wheretogo)
							      (pc, (void *) 0);
		if (trapword == LOADSEGTRAPWORD)
		    retval -= 6;
		else if (trapword == MODESWITCHTRAPWORD)
		    retval = new_addr;
	    } else
		FAKEPascalToCCall(&toolstuff[trapno].ptoc);
	} else {
	    PUSHADDR(retval);			/* Where they'll return to */
	    retval = (syn68k_addr_t) togoto;	/* Where they have patched */
							    /* us to go to */
	}
    } else {
	trapno = trapword & OSMASK;
	togoto = ostraptable[trapno];
	if (togoto == osstuff[trapno].orig) {
	    saved1 = EM_D1;
	    saved2 = EM_D2;
	    savea1 = EM_A1;
	    savea2 = EM_A2;
	    EM_D1 = trapword;
	    EM_D2 = (trapword & 0xFF) << 2;
	    EM_A2 = (LONGINT) togoto;
	    savea0 = EM_A0;
	    PUSHADDR(0x88A84321);	/* better not use this */
	    (*(callback_handler_t)osstuff[trapno].func)(pc,
				    callback_argument((syn68k_addr_t) togoto));
	    if (!(trapword & DONTSAVEA0BIT))
		EM_A0 = savea0;
	    EM_D1 = saved1;
	    EM_D2 = saved2;
	    EM_A1 = savea1;
	    EM_A2 = savea2;
	} else {
	    PUSHADDR(retval);
	    PUSHUW(status);
	    PUSHUW(WEIRDMAGIC);
	    PUSHUL(EM_A2);
	    PUSHUL(EM_D2);
	    PUSHUL(EM_D1);
	    PUSHUL(EM_A1);
	    EM_D1 = trapword;
	    EM_D2 = (trapword & 0xFF) << 2;
	    EM_A2 = (LONGINT) togoto;
	    if (!(trapword & DONTSAVEA0BIT))
		PUSHUL(EM_A0);
	    CALL_EMULATOR((syn68k_addr_t) togoto);
	    if (!(trapword & DONTSAVEA0BIT))
		EM_A0 = POPUL();
	    EM_A1 = POPUL();
	    EM_D1 = POPUL();
	    EM_D2 = POPUL();
	    EM_A2 = POPUL();
	    EM_A7 += 4;
	    retval = POPADDR();
	}
	cpu_state.ccc = 0;
	cpu_state.ccn = (cpu_state.regs[0].sw.n < 0);
	cpu_state.ccv = 0;
	cpu_state.ccx = cpu_state.ccx;	/* unchanged */
	cpu_state.ccnz = (cpu_state.regs[0].sw.n != 0);
    }

#if defined (MEMORY_WATCH)
    if (memory_watch)
      {
	ROMlib_sledgehammer_zones (__PRETTY_FUNCTION__, __FILE__, __LINE__,
				   "after aline", new_zone_infos);
	compare_zone_infos (trapword, current_zone_infos, new_zone_infos);
      }
#endif

#if !defined (NDEBUG)
    if (check_trap_watchpoints_p)
      check_trap_watchpoints ("exiting `alinehandler ()'\n");
#endif 

#if 0
    warning_trace_info ("out retval = 0x%x", retval);
#endif

    return retval;
}

PUBLIC void Executor::executor_main( void )
{
    char quickbytes[grafSize];
    LONGINT tmpA5;
    GUEST<INTEGER> mess, count_s;
    INTEGER count;
    INTEGER exevrefnum, toskip; AppFile thefile;
    Byte *p;
    int i;
    WDPBRec wdpb;
    CInfoPBRec hpb;
    Str255 name;
    StringPtr fName;

    SCSIFlags = CWC (0xEC00); /* scsi+clock+xparam+mmu+adb
				 (no fpu,aux or pwrmgr) */

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
    JSwapFont = RM ((ProcPtr)P_FMSwapFont);
    JInitCrsr = RM ((ProcPtr)P_InitCursor);

    Key1Trans = RM ((Ptr)P_Key1Trans);
    Key2Trans = RM ((Ptr)P_Key2Trans);
    JFLUSH = RM ((ProcPtr)P_flushcache);
    JResUnknown1 = JFLUSH;	/* I don't know what these are supposed to */
    JResUnknown2 = JFLUSH;	/* do, but they're not called enough for
				   us to worry about the cache flushing
				   overhead */

#if !defined (NEXT) && !defined (SYN68K)
    CPUFlag = 2;	/* mc68020 */
#else /* NEXT || SYN68K */
    CPUFlag = 4;	/* mc68040 */
#endif /* NEXT || SYN68K */
    UnitNtryCnt = 0;	/* how many units in the table */

    TheZone = SysZone;
    VIA = RM(NewPtr(16 * 512));	/* IMIII-43 */
    memset(MR(VIA), 0, (LONGINT) 16 * 512);
    *(char *)MR(VIA) = 0x80;	/* Sound Off */

#define SCC_SIZE 1024

    SCCRd = RM(NewPtrSysClear (SCC_SIZE));
    SCCWr = RM(NewPtrSysClear (SCC_SIZE));
    
    setup28();

    SoundBase = RM(NewPtr(370 * sizeof(INTEGER)));
#if 0
    memset(CL(SoundBase), 0, (LONGINT) 370 * sizeof(INTEGER));
#else /* !0 */
    for (i = 0; i < 370; ++i)
	((GUEST<INTEGER> *) MR(SoundBase))[i] = CWC (0x8000);	/* reference 0 sound */
#endif /* !0 */
    TheZone = ApplZone;
    HiliteMode = CB(0xFF);
    /* Mac II has 0x3FFF here */
    ROM85 = CWC(0x3FFF);

    a5 = US_TO_SYN68K(&tmpA5);
    CurrentA5 = guest_cast<Ptr> (CL(a5));
    InitGraf((Ptr) quickbytes + sizeof(quickbytes) - 4);
    InitFonts();
    InitCRM ();
    FlushEvents( everyEvent, 0 );
    InitWindows();
    TEInit();
    InitDialogs((ProcPtr)0);
    InitCursor();

    loadtrap = 0;

#if defined (MSDOS)
    ROMlib_WriteWhen(WriteInBltrgn);
#endif /* MSDOS */

    /* ROMlib_WriteWhen(WriteInOSEvent); */

    FinderName[0] = MIN(strlen(BROWSER_NAME), sizeof(FinderName)-1);
    memcpy(FinderName+1, BROWSER_NAME, FinderName[0]);
    
    CountAppFiles(&mess, &count_s);
    count = CW (count_s);
    if (count > 0)
	GetAppFiles(1, &thefile);
    else
	thefile.fType = 0;

#if defined (DISPLAY_SPLASH_SCREEN)  
    /* Reset the CLUT since it may have gotten changed by the splash screen. */
    gd_set_bpp (MR (MainDevice), !vdriver_grayscale_p, vdriver_fixed_clut_p,
		vdriver_bpp);
#endif /* DISPLAY_SPLASH_SCREEN */

#if defined (SYN68K)
#define ALINETRAPNUMBER       0xA
    trap_install_handler( ALINETRAPNUMBER, alinehandler, (void *) 0);
#endif
    if (thefile.fType == CLC(T('A','P','P','L')))
      {
	ClrAppFiles(1);
	Munger(MR(AppParmHandle), 2L*sizeof(INTEGER), (Ptr) 0,
	       (LONGINT) sizeof(AppFile), (Ptr) "", 0L);
      }
    hpb.hFileInfo.ioNamePtr   = RM(&thefile.fName[0]);
    hpb.hFileInfo.ioVRefNum   = thefile.vRefNum;
    hpb.hFileInfo.ioFDirIndex = CWC (0);
    hpb.hFileInfo.ioDirID     = CLC (0);
    PBGetCatInfo(&hpb, FALSE);

    if (thefile.fType == CLC(T('A','P','P','L')))
      fName = thefile.fName;
    else
      {
	const char *p;

	if (count > 0)
	  {
	    p = ROMlib_find_best_creator_type_match
	      (CL (hpb.hFileInfo.ioFlFndrInfo.fdCreator),
	       CL (hpb.hFileInfo.ioFlFndrInfo.fdType));
	  }
	else
	  {
	    if (!ROMlib_default_appp)
	      p = NULL;
	    else
	      p = (char *) ROMlib_default_appp->chars;
	  }
	fName = name;
	if (!p)
	  ExitToShell ();
	else
	  {
	    ROMlib_exit = TRUE;
	    str255_from_c_string (fName, p);
	    hpb.hFileInfo.ioNamePtr   = RM(fName);
	    hpb.hFileInfo.ioVRefNum   = CWC (0);
	    hpb.hFileInfo.ioFDirIndex = CWC (0);
	    hpb.hFileInfo.ioDirID     = CLC (0);
	    PBGetCatInfo(&hpb, FALSE);

	    {
	      HParamBlockRec hp;
	      Str255 fName2;

	      memset (&hp, 0, sizeof hp);
	      str255assign (fName2, fName);
	      hp.ioParam.ioNamePtr = RM ((StringPtr) fName2);
	      hp.volumeParam.ioVolIndex = CWC (-1);
	      PBHGetVInfo (&hp, FALSE);
	      hpb.hFileInfo.ioVRefNum   = hp.ioParam.ioVRefNum;
	    }
	  }
      }

    for (p = fName + fName[0] + 1;
	 p > fName && *--p != ':';)
      ;
    toskip = p - fName;
    CurApName[0] = MIN(fName[0]-toskip, 31);
    BlockMoveData((Ptr) fName+1+toskip, (Ptr) CurApName+1,
	      (Size) CurApName[0]);

    wdpb.ioVRefNum = hpb.hFileInfo.ioVRefNum;
    wdpb.ioWDDirID = hpb.hFileInfo.ioFlParID;
    SFSaveDisk_Update (CW (hpb.hFileInfo.ioVRefNum), fName);
    CurDirStore = hpb.hFileInfo.ioFlParID;
    wdpb.ioWDProcID = CLC (T('X','c','t','r'));
    wdpb.ioNamePtr = 0;
    PBOpenWD(&wdpb, FALSE);
    exevrefnum = CW(wdpb.ioVRefNum);
    Launch (CurApName, exevrefnum);  
}
