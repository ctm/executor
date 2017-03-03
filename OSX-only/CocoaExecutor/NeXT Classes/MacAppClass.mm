/*
 * Here is the original comment... it's not too relevant anymore:
 *
 * BreakApp.m, subclass of Application for BreakApp.
 * Author: Ali Ozer
 * Written for 1.0 July 89.
 * Modified for 2.0 Sept 90; separated the panels into their own .nib files.
 *
 * This class manages the windows & such for the BreakApp application.
 *
 *  You may freely copy, distribute and reuse the code in this example.
 *  NeXT disclaims any warranty of any kind, expressed or implied,
 *  as to its fitness for any particular use.
 */

#include "rsys/common.h"
#define Cursor NeXT_Cursor
#define Control NeXT_Control
#import <AppKit/AppKit.h>
#undef Cursor
#undef Control

#include "QuickDraw.h"
#include "rsys/next.h"
#import "MacAppClass.h"
#import "MacViewClass.h"
#import "MacWinClass.h"

#include "SegmentLdr.h"
#include "MemoryMgr.h"
#include "rsys/soundopts.h"
#include "rsys/blockinterrupts.h"
#include <syn68k_public.h>
#include "rsys/prefs.h"
#include "rsys/hfs.h"
#include "rsys/segment.h"
#include "rsys/notmac.h"
#include <sys/types.h>
#include <sys/time.h>
#include "rsys/version.h"
#include "FileMgr.h"
#include "rsys/file.h"
#include "ourstuff.h"
#include "rsys/parse.h"

using namespace Executor;

@implementation MacAppClass

char ROMlib_started;

MacWindow *global_gameWindow;
NSMenu *Executor::global_menu;
id global_myForm;
id global_myButton;
id global_registerDone;
id global_doneButton;
id global_sptext;
id global_starsptext;
id global_pctext;
id global_starpctext;
id global_d0text;
id global_pstext;
id global_sigtext;
id global_debtext;
id global_debtable;
id global_deathwindow;
id global_deathMenuCell;
id global_commenttext;
MacViewClass *global_game;
id global_splashScreen;

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{

  NSArray *types = @[NSRTFPboardType, NSPostScriptPboardType,
                     NSStringPboardType, NSTIFFPboardType, NSPICTPboardType];

  global_gameWindow  = gameWindow;
  global_game        = game;
  global_splashScreen = splashScreen;
  global_menu = myMenu;
  global_myForm = myForm;
  global_myButton = myButton;
  global_doneButton = doneButton;

  global_sptext        = sptext;
  global_starsptext    = starsptext;
  global_pctext        = pctext;
  global_starpctext    = starpctext;
  global_d0text        = d0text;
  global_pstext        = psltext;
  global_sigtext       = sigtext;
  global_debtext       = debtext;
  global_debtable      = debtable;
  global_deathwindow   = deathwindow;
  global_deathMenuCell = deathMenuCell;
  global_commenttext   = commenttext;

  [[NSApplication sharedApplication] registerServicesMenuSendTypes:types returnTypes:types];
}

// Constrain window resizing.

short ROMlib_constrained = NO;

- (void) windowDidBecomeKey : (NSNotification *)anotification
{
    NSRect tempr;

  /*
   * NOTE: we could become the key window while the mouse is outside our window
   *	 (because some application hid, for instance).  The following three
   *       lines will insure that we notice that we are on the outside if this
   *       happens.
   */

  tempr = [game bounds];

  /* NOTE: userData of 0 may cause trouble ... I have vague recollections
   of that happening under NEXTSTEP */
  
#if 0
  [gameWindow addTrackingRect:tempr owner:game userData:0 assumeInside:YES];
#else
#warning need to fix trackingrect stuff
#endif
  if (CrsrVis)
	[realcursor set];
  else
	[blankcursor set];
  if (ROMlib_started >= 3)
	sendresumeevent([[NSPasteboard generalPasteboard] changeCount] >
                    ROMlib_ourchangecount);
}

- (void) windowDidResignKey : (NSNotification *)anotification
{
    if (ROMlib_started >= 3)
	sendsuspendevent();
}

// printGame: allows us to print the game window. We could've just connected
// the "Print..." menu item to the window's smartPrintPSCode:; however, we
// wanted to be able to change the title to reflect the status.
/* ctm NOTE: we no longer change the title */

- (IBAction)printGame:(id)sender
{
    [gameWindow print:sender];
}


// Method to load the .nib file for the info panel.

- (IBAction)showInfo:(id)sender
{
  if (!infoPanel) {
    [NSBundle loadNibNamed:@"info" owner:self];
    [versionString setStringValue:@(ROMlib_executor_version)];
  }
  [infoPanel makeKeyAndOrderFront:sender];
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
  ROMlib_OurClose();
}

@end

#define KEYTYPE \
"Registration Keys are thirteen lower case letters and/or digits."

#define FILLIN \
"You must fill in the Owner and Registration Key fields."

#define NOTVALID \
"That registration key is not valid\nCall Abacus R&D Inc. at +1 505 766 9115."

#define TOOMANYONCPU \
"You may only run up to ten copies of this application simultaneously on one CPU."

#define TOOMANYONNET "This program is already in use on the maximum number of CPUs that your current license allows.  Call ARDI at +1 505 766 9115 to increase your limit."

#define NEGSERIALNUMBERS

LONGINT Executor::ROMlib_whichapps;

void toomanycopiesonnet( void )
{
    ROMlib_started = 4;
    NSRunAlertPanel(nil, @TOOMANYONNET, nil, nil, nil);
    C_ExitToShell();
}

void toomanycopiesonthiscpu( void )
{
    ROMlib_started = 4;
    NSRunAlertPanel(nil, @TOOMANYONCPU, nil, nil, nil);
    C_ExitToShell();
}

void warnuser( const char *str )
{
    NSRunAlertPanel(@"Warning", @"%s", nil, nil, nil, str);
}

#define WILLSETPERMS \
"The permissions for this program are not properly set.  I will reset them for you."

#define MUSTBESETUID \
"The permissions for this program are not set properly.  You need to run this program once as \"root\" so the permissions will be reset."

#define CANNOTCREATECONFIG \
"The configuration file could not be created."

#define SOMEONEELSEONFLOPPY \
"The floppy drive is already in use by a running program, so this program will not be able to use the floppy drive."

#define BADFILESYSTEM \
"The filesystem that Executor is installed on does not support setuid root programs.  Contact your system administrator for more help."

void willsetperms( void )
{
    NSRunAlertPanel(nil, @WILLSETPERMS, nil, nil, nil);
}

void cannotcreatconfig( void )
{
    NSRunAlertPanel(nil, @CANNOTCREATECONFIG, nil, nil, nil);
}

void mustbesetuid( void )
{
    ROMlib_started = 4;
    NSRunAlertPanel(nil, @MUSTBESETUID, nil, nil, nil);
    C_ExitToShell();
}

void badfilesystem( void )
{
    ROMlib_started = 4;
    NSRunAlertPanel(nil, @BADFILESYSTEM, nil, nil, nil);
    C_ExitToShell();
}

void someoneelseonfloppy( void )
{
   NSRunAlertPanel(nil, @SOMEONEELSEONFLOPPY, nil, nil, nil);
}

/*
 * WindowName		 1
 * WindowLocation	 x 2, y 3
 * WindowSize		 x 4, y 5
 * Delay		 6	// No longer supported
 * WriteWhen		 7
 * Refresh		 8
 * Noclock		 9
 * PretendSound		10
 * debug		11	// No longer supported
 * NewLinetoCR		15
 * PassPostscript	16
 * DirectDiskAccess	17
 * Accelerated		18
 * NoWarn32		19
 * FlushOften		20
 */

typedef enum { NAME = 1, DELAY = 6, WRITEWHEN, REFRESH, NOCLOCK, PRETENDSOUND,
		      NEWLINETOCR = 15, PASSPOSTSCRIPT, DIRECTDISKACCESS,
				     ACCELERATED, NOWARN32, FLUSHOFTEN} pref_t;

#define ROMLIB_DEBUG_BIT	(1 << 1)

#define HASHBITS	4091	/* DON'T EVER CHANGE THIS NUMBER, OR YOU'LL */
				/* MESS UP EXISTING CONFIGURATION FILES! */
#define NCACHEBYTES	((HASHBITS+7)/ 8)

static void mydelay( long usec )
{
    struct timeval now, later, timeout;
    struct timezone tz;
    fd_set foofds;

    gettimeofday(&now, &tz);
    later.tv_usec  = now.tv_usec + usec;
    later.tv_sec   = now.tv_sec + later.tv_usec / 1000000L;
    later.tv_usec %= 1000000L;
    FD_ZERO(&foofds);
    while (usec > 0) {
	timeout.tv_sec  = usec / 1000000L;
	timeout.tv_usec = usec % 1000000L;
	select(0, &foofds, &foofds, &foofds, &timeout);
	gettimeofday(&now, &tz);
	usec = (later.tv_sec - now.tv_sec) * 1000000 +
						 (later.tv_usec - now.tv_usec);
    }
}

void ROMlib_updateworkspace( void )
{
    virtual_int_state_t block;

    block = block_virtual_ints ();
    mydelay(1250000);

    //[[NSWorkspace sharedWorkspace] noteFileSystemChanged];

    restore_virtual_ints (block);
}

void ROMlib_splashscreen( void )
{
    [global_splashScreen makeKeyAndOrderFront:0];
}
