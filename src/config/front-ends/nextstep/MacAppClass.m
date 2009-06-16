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
#ifndef OPENSTEP
#import <appkit/appkit.h>
#else /* OPENSTEP */
#import <AppKit/AppKit.h>
#endif /* OPENSTEP */
#undef Cursor
#undef Control

#include "QuickDraw.h"
#include "rsys/next.h"
#import "MacAppClass.h"
#import "MacViewClass.h"

#include "SegmentLdr.h"
#include "MemoryMgr.h"
#include "rsys/soundopts.h"
#include "rsys/blockinterrupts.h"
#include "rsys/syn68k_public.h"
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

@implementation MacAppClass

char ROMlib_started;

#ifndef OPENSTEP
// appDidInit is called as the first thing in the run loop of the 
// application. At this point, everything is created, but we haven't entered
// the event loop yet. appDidInit initializes a few things and
// calls the gotoFirstLevel: method of the BreakView instance to get
// started on a new game.

#endif /* not OPENSTEP */
id global_gameWindow;
id global_menu;
id global_serialText;
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
id global_registerWindow;
id global_registerWindow2;
id global_serialwindow;
id global_serialform;
id global_game;
id global_splashScreen;

#ifndef OPENSTEP
/*
 * Don't have to worry about interrupts in appDidInit, since it gets
 * called before the mac side starts up.
 */

- appDidInit:app 
#else /* OPENSTEP */
- (void)finishLaunching
#endif /* OPENSTEP */
{
#ifndef OPENSTEP
    NXRect tempr;
    char const *types[5] = {
	    NXRTFPboardType,
	    NXPostScriptPboardType,
	    NXAsciiPboardType,
	    NXTIFFPboardType,
	    0
    };
#else /* OPENSTEP */
#if 0
    NSRect tempr;
#endif

    NSArray *types;

    [super finishLaunching];
    types = [NSArray arrayWithObjects:NSRTFPboardType, NSPostScriptPboardType,
		    NSStringPboardType, NSTIFFPboardType, nil];
#endif /* OPENSTEP */

    nextmain();
    [gameWindow setDelegate:self];  // We want window resized messages
#ifndef OPENSTEP
    [game getBounds:&tempr];
    [gameWindow getFrame:&tempr];
#endif /* not OPENSTEP */
    global_gameWindow  = gameWindow;
    global_game        = game;
    global_splashScreen = splashScreen;
    global_menu = myMenu;
    global_serialText = serialText;
    global_myForm = myForm;
    global_myButton = myButton;
    global_doneButton = doneButton;
    global_registerWindow  = registerWindow;
    global_registerWindow2 = registerWindow2;
    global_serialwindow = serialWindow;
    global_serialform = serialForm;

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

#ifndef OPENSTEP
    [self registerServicesMenuSendTypes:types andReturnTypes:types];

    return self;
#else /* OPENSTEP */
    [self registerServicesMenuSendTypes:types returnTypes:types];
#endif /* OPENSTEP */
}

// Constrain window resizing.

short ROMlib_constrained = NO;

#ifndef OPENSTEP
- windowDidBecomeKey : sender
#else /* OPENSTEP */
- (void) windowDidBecomeKey : (NSNotification *)anotification
#endif /* OPENSTEP */
{
#ifndef OPENSTEP
    NXRect tempr;
#else /* OPENSTEP */
    NSRect tempr;
#endif /* OPENSTEP */

/*
 * NOTE: we could become the key window while the mouse is outside our window
 *	 (because some application hid, for instance).  The following three
 *       lines will insure that we notice that we are on the outside if this
 *       happens.
 */

    SETUPA5;
#ifndef OPENSTEP
    if (!ROMlib_pasteboard)
	ROMlib_pasteboard = [Pasteboard new];
#endif /* not OPENSTEP */

#ifndef OPENSTEP
    [game getFrame:&tempr];
    [game convertRect:&tempr toView:nil];
    [gameWindow setTrackingRect:&tempr inside:YES owner:game tag:1 left:NO
								     right:NO];
#else /* OPENSTEP */
    tempr = [game bounds];

/* NOTE: userData of 0 may cause trouble ... I have vague recollections
         of that happening under NEXTSTEP */

#if 0
    [gameWindow addTrackingRect:tempr owner:game userData:0 assumeInside:YES];
#else
#warning need to fix trackingrect stuff
#endif
#endif /* OPENSTEP */
    if (CrsrVis)
	[realcursor set];
    else
	[blankcursor set];
    if (ROMlib_started >= 3)
#ifndef OPENSTEP
	sendresumeevent([ROMlib_pasteboard changeCount] >
#else /* OPENSTEP */
	sendresumeevent([[NSPasteboard generalPasteboard] changeCount] >
#endif /* OPENSTEP */
							ROMlib_ourchangecount);
    RESTOREA5;
#ifndef OPENSTEP
    return self;
#endif /* not OPENSTEP */
}

#ifndef OPENSTEP
- windowDidResignKey : sender
#else /* OPENSTEP */
- (void) windowDidResignKey : (NSNotification *)anotification
#endif /* OPENSTEP */
{
    SETUPA5;

    if (ROMlib_started >= 3)
	sendsuspendevent();
    RESTOREA5;
#ifndef OPENSTEP
    return self;
#endif /* not OPENSTEP */
}

// printGame: allows us to print the game window. We could've just connected
// the "Print..." menu item to the window's smartPrintPSCode:; however, we
// wanted to be able to change the title to reflect the status.
/* ctm NOTE: we no longer change the title */

- printGame:sender
{
#ifndef OPENSTEP
    [gameWindow smartPrintPSCode:sender];
#else /* OPENSTEP */
    [gameWindow print:sender];
#endif /* OPENSTEP */

    return self;
}


// Method to load the .nib file for the info panel.

- showInfo:sender
{
    if (!infoPanel) {
#ifndef OPENSTEP
	[self loadNibSection:"Info.nib" owner:self withNames:NO];
	[versionString setStringValue:ROMlib_executor_version];
#else /* OPENSTEP */
	[NSBundle loadNibFile:@"Info.nib" externalNameTable:nil withZone:NULL];
	[versionString setStringValue:
	  [NSString stringWithCString:ROMlib_executor_version]];
#endif /* OPENSTEP */
    }
    [infoPanel makeKeyAndOrderFront:sender];
    return self;
}

- appWillTerminate:sender
{
    SETUPA5;
    ROMlib_OurClose();
    RESTOREA5;
    return self;
}

#if defined(BINCOMPAT)
- (BOOL) appAcceptsAnotherFile:sender
{
     return ROMlib_acceptsanotherfile ? YES : NO;
}

char *ROMlib_toexec = 0;
char *ROMlib_toopen = 0;

static struct {
    char *extension;
    char **pathp;
} ourtable[] = {
    "xl",	&ROMlib_ExcelApp,
    "XL",	&ROMlib_ExcelApp,
    "mxl",	&ROMlib_ExcelApp,
    "MXL",	&ROMlib_ExcelApp,
    "msw",	&ROMlib_WordApp,
    "MSW",	&ROMlib_WordApp,
};

static char *mallocname(const char *path)
{
    char *retval;
    char *slash;
    char isres;

    SETUPA5;
    retval = malloc(strlen(path) + 1);
    strcpy(retval, path); 
    slash = rindex(retval, '/');
    isres = ROMlib_isresourcefork(path);
    if (slash && isres)
	memmove(slash+1, slash+2, strlen(slash+1));
    RESTOREA5;
    return retval;
};

- (int) openFile:(const char *) path ok:(int *) flag
{
    const char *dot;
    int i;

    dot = rindex(path, '.');
    i = NELEM(ourtable);
    if (dot) {
	++dot;
	for (i = 0; i < NELEM(ourtable); ++i) {
	    if (strcmp(ourtable[i].extension, dot) == 0)
/*-->*/		break;
	}
    }
    if (i < NELEM(ourtable)) {
	ROMlib_toexec = (char *) ourtable[i].pathp; /* ick */
	ROMlib_toopen = mallocname(path);
    } else
	ROMlib_toexec = mallocname(path);

    *flag = YES;	/* we're lying */
    return 0;
}
#endif

#define KEYTYPE \
"Registration Keys are thirteen lower case letters and/or digits."

#define FILLIN \
"You must fill in the Owner and Registration Key fields."

#define NOTVALID \
"That registration key is not valid\nCall Abacus R&D Inc. at +1 505 766 9115."

#define TOOMANYONCPU \
"You may only run up to ten copies of this application simultaneously on one CPU."

#ifndef OPENSTEP
#define TOOMANYONNET \
"This program is already in use on the maximum number of CPUs that your" \
" current license allows.  Call ARDI at +1 505 766 9115 to increase your limit."
#else /* OPENSTEP */
#define TOOMANYONNET "This program is already in use on the maximum number of CPUs that your current license allows.  Call ARDI at +1 505 766 9115 to increase your limit."
#endif /* OPENSTEP */

#define NEGSERIALNUMBERS

LONGINT ROMlib_whichapps;

void toomanycopiesonnet( void )
{
    ROMlib_started = 4;
#ifndef OPENSTEP
    NXRunAlertPanel((const char *) 0, TOOMANYONNET, (const char *)0,
					     (const char *)0, (const char *)0);
#else /* OPENSTEP */
    NSRunAlertPanel(nil, @TOOMANYONNET, nil, nil, nil);
#endif /* OPENSTEP */
    C_ExitToShell();
}

void toomanycopiesonthiscpu( void )
{
    ROMlib_started = 4;
#ifndef OPENSTEP
    NXRunAlertPanel((const char *) 0, TOOMANYONCPU, (const char *)0,
					     (const char *)0, (const char *)0);
#else /* OPENSTEP */
    NSRunAlertPanel(nil, @TOOMANYONCPU, nil, nil, nil);
#endif /* OPENSTEP */
    C_ExitToShell();
}

void warnuser( const char *str )
{
#ifndef OPENSTEP
    NXRunAlertPanel("Warning", str, (const char *)0, (const char *)0,
							      (const char *)0);
#else /* OPENSTEP */
    NSRunAlertPanel(@"Warning", @"%@", nil, nil, nil, str);
#endif /* OPENSTEP */
}

static int whichbit(unsigned long n)
{
    return n <= 1 ? 0 : whichbit(n>>1) + 1;
}

#if defined(BINCOMPAT)
void norunthisapp( void )
{
    char message[80];
    int apptag;
    id regview, apptext;
#ifdef OPENSTEP
    char buf[40];
#endif /* OPENSTEP */

    ROMlib_started = 4;
    apptag = 10 + whichbit(ROMlib_appbit);
    regview = [global_registerWindow2 contentView];
#ifndef OPENSTEP
    apptext = [regview findViewWithTag:apptag];
#else /* OPENSTEP */
    apptext = [regview viewWithTag:apptag];
    [[apptext stringValue] getCString:buf maxLength:(sizeof (buf) - 1)];
#endif /* OPENSTEP */
    sprintf(message, "\"%s\" is not enabled.\n"
	    "Call Abacus R&D at (505) 766-9115 if you would like to enable it",
#ifndef OPENSTEP
							[apptext stringValue]);
    NXRunAlertPanel((const char *) 0, message, (const char *)0,
					     (const char *)0, (const char *)0);
#else /* OPENSTEP */
							buf);
    NSRunAlertPanel(nil, @"%@", nil, nil, nil, message);
#endif /* OPENSTEP */
    C_ExitToShell();
}
#endif

#ifndef OPENSTEP
#define WILLSETPERMS \
"The permissions for this program are not properly set.  I will reset them" \
" for you."
#else /* OPENSTEP */
#define WILLSETPERMS \
"The permissions for this program are not properly set.  I will reset them for you."
#endif /* OPENSTEP */

#ifndef OPENSTEP
#define MUSTBESETUID \
"The permissions for this program are not set properly.  You need to run" \
" this program once as \"root\" so the permissions will be reset."
#else /* OPENSTEP */
#define MUSTBESETUID \
"The permissions for this program are not set properly.  You need to run this program once as \"root\" so the permissions will be reset."
#endif /* OPENSTEP */

#define CANNOTCREATECONFIG \
"The configuration file could not be created."

#ifndef OPENSTEP
#define SOMEONEELSEONFLOPPY \
"The floppy drive is already in use by a running program, so this program" \
" will not be able to use the floppy drive."
#else /* OPENSTEP */
#define SOMEONEELSEONFLOPPY \
"The floppy drive is already in use by a running program, so this program will not be able to use the floppy drive."
#endif /* OPENSTEP */

#ifndef OPENSTEP
#define BADFILESYSTEM \
"The filesystem that Executor is installed on does not support setuid root" \
" programs.  Contact your system administrator for more help."
#else /* OPENSTEP */
#define BADFILESYSTEM \
"The filesystem that Executor is installed on does not support setuid root programs.  Contact your system administrator for more help."
#endif /* OPENSTEP */

void willsetperms( void )
{
#ifndef OPENSTEP
    NXRunAlertPanel((const char *) 0, WILLSETPERMS, (const char *)0,
					     (const char *)0, (const char *)0);
#else /* OPENSTEP */
    NSRunAlertPanel(nil, @WILLSETPERMS, nil, nil, nil);
#endif /* OPENSTEP */
}

void cannotcreatconfig( void )
{
#ifndef OPENSTEP
    NXRunAlertPanel((const char *) 0, CANNOTCREATECONFIG, (const char *)0,
					     (const char *)0, (const char *)0);
#else /* OPENSTEP */
    NSRunAlertPanel(nil, @CANNOTCREATECONFIG, nil, nil, nil);
#endif /* OPENSTEP */
}

void mustbesetuid( void )
{
    ROMlib_started = 4;
#ifndef OPENSTEP
    NXRunAlertPanel((const char *) 0, MUSTBESETUID, (const char *)0,
					     (const char *)0, (const char *)0);
#else /* OPENSTEP */
    NSRunAlertPanel(nil, @MUSTBESETUID, nil, nil, nil);
#endif /* OPENSTEP */
    C_ExitToShell();
}

void badfilesystem( void )
{
    ROMlib_started = 4;
#ifndef OPENSTEP
    NXRunAlertPanel((const char *) 0, BADFILESYSTEM, (const char *)0,
					     (const char *)0, (const char *)0);
#else /* OPENSTEP */
    NSRunAlertPanel(nil, @BADFILESYSTEM, nil, nil, nil);
#endif /* OPENSTEP */
    C_ExitToShell();
}

void someoneelseonfloppy( void )
{
#ifndef OPENSTEP
    NXRunAlertPanel((const char *) 0, SOMEONEELSEONFLOPPY, (const char *)0,
					     (const char *)0, (const char *)0);
#else /* OPENSTEP */
   NSRunAlertPanel(nil, @SOMEONEELSEONFLOPPY, nil, nil, nil);
#endif /* OPENSTEP */
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
		     DEBUG, NEWLINETOCR = 15, PASSPOSTSCRIPT, DIRECTDISKACCESS,
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
#ifndef OPENSTEP
    port_t workspaceport;
    id myspeaker;
    int err;
#endif /* not OPENSTEP */
    virtual_int_state_t block;

    block = block_virtual_ints ();
    mydelay(1250000);
#ifndef OPENSTEP
    if ( workspaceport = NXPortFromName(NX_WORKSPACEREQUEST, NULL) ) {
	myspeaker = [Speaker new];
	[myspeaker setSendPort:workspaceport];
	err = [myspeaker performRemoteMethod:"update"];
	if (err)
	    fprintf(stderr, "performRemoteMethod returned %d\n", err);
	[myspeaker free];
    } else
	fprintf(stderr, "Couldn't get workspaceport\n");
#else /* OPENSTEP */

    [[NSWorkspace sharedWorkspace] noteFileSystemChanged];

#endif /* OPENSTEP */
    restore_virtual_ints (block);
}

void ROMlib_splashscreen( void )
{
    [global_splashScreen makeKeyAndOrderFront:0];
}

@end
