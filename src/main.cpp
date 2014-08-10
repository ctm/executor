/* Copyright 1988 - 2005 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_main[] =
		"$Id: main.c 127 2006-04-04 00:18:10Z ctm $";
#endif

/*
 * Errors should be handled more cleanly.
 */

#include "rsys/common.h"

#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "SegmentLdr.h"
#include "rsys/notmac.h"
#include "MemoryMgr.h"
#include "FontMgr.h"
#include "SysErr.h"
#include "OSUtil.h"
#include "ResourceMgr.h"
#include "TextEdit.h"
#include "FileMgr.h"
#include "DialogMgr.h"
#include "StdFilePkg.h"
#include "MenuMgr.h"
#include "ScrapMgr.h"
#include "SoundMgr.h"
#include "ScriptMgr.h"
#include "SANE.h"
#include "DeskMgr.h"
#include "DeviceMgr.h"
#include "ToolboxUtil.h"
#include "Gestalt.h"
#include "AppleTalk.h"
#include "AppleEvents.h"

#include "rsys/cquick.h"
#include "rsys/tesave.h"
#include "rsys/mman.h"
#include "rsys/menu.h"
#include "rsys/next.h"
#include "rsys/setuid.h"
#include "rsys/pstuff.h"
#include "rsys/prefs.h"
#include "rsys/misc.h"
#include "rsys/flags.h"
#include "rsys/option.h"
#include "rsys/host.h"
#include "rsys/syncint.h"
#include "rsys/vdriver.h"
#include "rsys/vbl.h"
#include "rsys/time.h"
#include "rsys/segment.h"
#include "rsys/keycode.h"
#include "rsys/version.h"
#include "rsys/m68kint.h"
#include "rsys/misc.h"
#include "rsys/blockinterrupts.h"
#include "rsys/rgbutil.h"
#include "rsys/refresh.h"
#include "rsys/executor.h"
#include "rsys/wind.h"
#include "rsys/osevent.h"
#include "rsys/image.h"
#include "rsys/dump.h"
#include "rsys/file.h"
#include "rsys/ctl.h"
#include "rsys/parseopt.h"
#include "rsys/print.h"
#include "rsys/memsize.h"
#include "rsys/splash.h"
#include "rsys/stdfile.h"
#include "rsys/autorefresh.h"
#include "rsys/sounddriver.h"
#include "rsys/desperate.h"
#include "rsys/dcache.h"
#include "rsys/system_error.h"
#include "rsys/filedouble.h"
#include "rsys/option.h"

#include "rsys/os.h"
#include "rsys/arch.h"
#include "rsys/checkpoint.h"
#include "rsys/cleanup.h"
#include "rsys/custom.h"
#include "rsys/gestalt.h"
#include "rsys/keyboards.h"
#include "rsys/launch.h"
#include "rsys/text.h"
#include "rsys/appearance.h"
#include "rsys/hfs_plus.h"

#include "rsys/check_structs.h"

#include "paramline.h"

#if defined (MSDOS)
#include "aspi.h"
#include "dosdisk.h"
#include "dosmem.h"
#include "dpmilock.h"
#include "vga.h"
#include "openmany.h"
#include "dosserial.h"
#include <process.h>
#endif /* MSDOS */

#include <ctype.h>

#if !defined (MSDOS) && !defined(CYGWIN32)
#include <sys/wait.h>
#endif

#if defined (MMAP_LOW_GLOBALS)
#include <sys/types.h>
#include <sys/mman.h>
#endif /* MMAP_LOW_GLOBALS */

#if defined (CYGWIN32)
#include "winfs.h"
#include "dosdisk.h"
#include "win_except.h"
#include "win_queue.h"
#include "win_clip.h"
#include "win_print.h"
#endif

#if defined(X)
#include "x.h"
#endif

#include <vector>

using namespace Executor;
using namespace ByteSwap;
using namespace std;

BOOLEAN Executor::force_big_offset = CONFIG_OFFSET_P;

#if defined (MACOSX_)
#define main oldmain
char *ROMlib_xfervmaddr;
LONGINT ROMlib_xfervmsize = 0;
#endif /* MACOSX_ */

PUBLIC int Executor::ROMlib_noclock = 0;

#if defined (NOMOUSE_COMMAND_LINE_OPTION)
PUBLIC int ROMlib_no_mouse = 1;
#endif

/* optional resolution other than 72dpix72dpi for printing */
PUBLIC INTEGER Executor::ROMlib_optional_res_x, Executor::ROMlib_optional_res_y;

/* A string approximating the original command line. */
PUBLIC const char *Executor::ROMlib_command_line;

/* Set to TRUE if there was any error parsing arguments. */
PRIVATE boolean_t bad_arg_p = FALSE;

/* Set to FALSE when we know that the command line switches will result
   in no graphics. */

PRIVATE boolean_t graphics_p = TRUE;

/* for a description of flags declared here, see <rsys/flags.h> */

/* Initial screen size.  This can be changed dynamically. */
INTEGER Executor::flag_width = 0, Executor::flag_height = 0;  /* 0 means "use default". */

/* Initial bits per pixel.  The screen depth can be changed dynamically. */
int Executor::flag_bpp = 0;  /* 0 means "use default". */

INTEGER Executor::ROMlib_shadow_screen_p = TRUE;

INTEGER Executor::ROMlib_no_windows;

#if defined (MSDOS) || defined (CYGWIN32)
int ROMlib_drive_check = 0;
#endif

#if defined(SYN68K)
static boolean_t use_native_code_p = TRUE;
#endif

/* the system version that executor is currently reporting to
   applications, set through the `-system' options.  contains the
   version number in the form `0xABC' which corresponds to system
   version A.B.C */
uint32 Executor::system_version = 0x700; /* keep this in sync with Browser's .ecf file */

const option_vec Executor::common_opts =
{
  { "sticky",      "sticky menus",                opt_no_arg,   NULL },
  { "pceditkeys",  "have Delete key delete one character forward",
    opt_no_arg, NULL },
  { "nobrowser",   "don't run Browser",           opt_no_arg,   NULL },
  { "bpp",         "default screen depth",        opt_sep,      NULL }, 
  { "size",        "default screen size",         opt_sep,      NULL },
  { "debug",
      ("enable certain debugging output and consistency checks.  This "
       "is primarily used by ARDI developers, but we are making it "
       "available during the pre-beta period to expert users.  The next "
       "argument must be a list of comma-separated words describing which "
       "debug options you want enabled.  You can abbreviate the debug "
       "options as long as the abbreviation is unambiguous.  Here is a "
       "list of the options (some of which will may do nothing):  "
       "\"all\" enables all debugging options, "
       "\"fslog\" enables filesystem call logging, "
       "\"memcheck\" enables heap consistency checking (slow!), "
       "\"textcheck\" enables text record consistency checking (slow!), "
       "\"trace\" enables miscellaneous trace information, "
       "\"sound\" enables miscellaneous sound logging information, "
       "\"trapfailure\" enables warnings when traps return error codes, "
       "\"errno\" enables some C library-related warnings, "
       "\"unexpected\" enables warnings for unexpected events, "
       "\"unimplemented\" enables warnings for unimplemented traps.  "
       "Example: \"executor -debug unimp,trace\""),

      opt_sep, NULL },
  { "nodiskcache", "disable internal disk cache.",
      opt_no_arg, NULL },
  { "nosound", "disable any sound hardware",
      opt_no_arg, NULL },
  { "info",        "print information about your system",
      opt_no_arg, NULL },
#if defined (SUPPORT_LOG_ERR_TO_RAM)
  { "ramlog",
      "log debugging information to RAM; alt-shift-7 dumps the "
      "accrued error log out via the normal mechanism.",
      opt_no_arg, NULL },
#endif

#if defined (MSDOS)
  { "oldtimer",
      "use old-style 18.2 Hz timer, instead of the newer 1024 Hz "
      "timer.  This will result in less accurate timer emulation, "
      "but may work around problems with certain BIOSes.",
	opt_no_arg, NULL },
  { "vga",
      "use old-style VGA graphics modes only, and don't use SuperVGA.  "
      "This will limit you to a slow 640x480 with 16 colors, but may let "
      "you run Executor even when Executor has problems with your "
      "video driver.",
	opt_no_arg, NULL },
  { "nofilescheck",
      "bypass a startup check that attempts to determine if your FILES= "
      "parameter is large enough.",
        opt_no_arg, NULL },
  { "videospeedup",
      "if you have a linear frame buffer (which requires a VBE 2.0 video "
      "driver like SciTech Display Doctor) this can make graphics much faster "
      "for games that bypass QuickDraw and therefore require \"refresh\" mode."
      "  This isn't needed under plain DOS if you are using cwsdpmi.exe.  "
      "The drawback is that enabling this feature removes some memory "
      "protection from your system and makes it possible for Executor "
      "to crash your computer or destroy data.  On some systems this "
      "switch will cause Executor to crash right away; you can't use it "
      "on such systems, and there's nothing that can be done about it.  "
      "Use at your own risk!",
        opt_no_arg, NULL },
  { "nosync",
      "Allow disk data structures to stay in memory until Executor quits.  "
      "This will dramatically speed up Executor's creation and deletion of "
      "many small files, but if Executor crashes the data in memory won't "
      "be written to disk and HFV corruption could result.  "
      "Use at your own risk!",
        opt_no_arg, NULL },
  { "skipaspi", "totally bypass ASPI drivers",
      opt_no_arg, NULL },
#endif /* MSDOS */
#if defined (MSDOS) || defined (CYGWIN32)
  { "macdrives", "drive letters that represent Mac formatted media",
      opt_sep, NULL },
  { "dosdrives", "drive letters that represent DOS formatted media",
      opt_sep, NULL },
  { "skipdrives", "drive letters that represent drives to avoid",
      opt_sep, NULL },
#endif
#if defined (LINUX)
  { "nodrivesearch",
      "Do not look for a floppy drive, CD-ROM drive or any other drive "
      "except as specified by the MacVolumes environment variable",
	opt_no_arg,   NULL },
#endif /* LINUX */
  { "keyboards", "list available keyboard mappings",
      opt_no_arg, NULL },
  { "keyboard", "choose a specific keyboard map", opt_sep, NULL },
  { "print",
      "tell program to print file; not useful unless you also "
      "specify a program to run and one or more documents to print.",
	opt_no_arg, NULL },
#if defined (NEXT) || defined (LINUX)
  { "nodotfiles",  "do not display filenames that begin with dot", opt_no_arg,
      NULL },
#endif
#if 0
  { "noclock",     "disable timer",               opt_no_arg,   NULL },
#endif

#if defined (NOMOUSE_COMMAND_LINE_OPTION)
  /* Hack Dr. Chung wanted */
  { "nomouse",     "ignore missing mouse",        opt_no_arg,   NULL },
#endif
  { "noautorefresh",
      "turns off automatic detection of programs that bypass QuickDraw.",
      opt_no_arg, NULL },
  { "refresh",
      "handle programs that bypass QuickDraw, at a performance penalty.  "
      "Follow -refresh with an number indicating how many 60ths of a second "
      "to wait between each screen refresh, e.g. \"executor -refresh 10\".",
      opt_optional, "10", },
  { "help",        "print this help message", opt_no_arg, NULL, },
  { "version", "print the Executor version", opt_no_arg, NULL, },
#if defined (MALLOC_MAC_MEMORY)
  { "memory",
      "specify the total memory you want reserved for use by the programs "
      "run under Executor and for Executor's internal system software.  "
      "For example, \"executor -memory 5.5M\" would "
      "make five and a half megabytes available to the virtual machine.  "
      "Executor will require extra memory above and beyond this amount "
      "for other uses.",
      opt_sep, NULL, },
  { "applzone",
      "specify the memory to allocate for the application being run, "
      "e.g. \"executor -applzone 4M\" would make four megabytes "
      "of RAM available to the application.  \"applzone\" stands for "
      "\"application zone\".",
      opt_sep, NULL, },
  { "stack",
      "like -applzone, but specifies the amount of stack memory to allocate.",
	opt_sep, NULL, },
  { "syszone", "like -applzone, but specifies the amount of memory to make "
      "available to Executor's internal system software.",
	opt_sep, NULL, },
#if defined (MM_MANY_APPLZONES)
  { "napplzones",
    "debugging flag, specifies the number of applzones to use",
    opt_sep, NULL },
#endif /* MM_MANY_APPLZONES */
#endif /* MALLOC_MAC_MEMORY */
  { "system",
      "specify the system version that executor reports to applications",
    opt_sep, NULL },
#if defined (SYN68K)
  { "notnative",
      "don't use native code in syn68k",
    opt_no_arg, NULL },
#endif

#if defined (MSDOS) || defined(VDRIVER_SVGALIB) || defined (CYGWIN32)
  { "logerr", "log diagnostic output to error log file", opt_no_arg,
    NULL },
#endif

  { "grayscale", "\
specify that executor should run in grayscale mode even if it is \
capable of color.",
    opt_no_arg, NULL },
  { "netatalk", "use netatalk naming conventions for AppleDouble files",
      opt_no_arg, NULL },
  { "afpd", "use afpd conventions for AppleDouble files (implies -netatalk)",
      opt_no_arg, NULL },

#if defined (MSDOS)
  { "modemport", "which PC serial port (1, 2, 3 or 4) should be used when"
    " a Macintosh application access the modem port", opt_sep, NULL,
  },
  { "printerport", "which PC serial port (1, 2, 3 or 4) should be used when"
    " a Macintosh application *directly* accesses the printer port.  This"
    " option does not affect printing.  It only affects communications type "
    " programs that do their own serial port management.", opt_sep, NULL,
  },
#endif

#if CONFIG_OFFSET_P == 0
  { "offset", "offset Mac memory (i.e. simulate Win32)", opt_no_arg, NULL },
#endif

  { "cities", "Don't use Helvetica for Geneva, Courier for Monaco and Times "
    "for New York when generating PostScript", opt_no_arg, NULL },

#if defined (CYGWIN32)
  { "die", "allow Executor to die instead of catching trap", opt_no_arg,
    NULL },
  { "noautoevents", "disable timer driven event checking", opt_no_arg,
    NULL },
#endif

#if defined (MSDOS) || defined (CYGWIN32)
  { "nocheckpoint", "disable \"failure.txt\" checkpointing", opt_no_arg,
    NULL },
#endif

  { "prvers",
      "specify the printer version that executor reports to applications",
    opt_sep, NULL },

  { "prres",
    "specify an additional resolution available for printing, e.g. "
    "\"executor -prres 600x600\" will make 600dpi resolution available "
    "in addition to the standard 72dpi.  Not all apps will be able to use "
    "this additional resolution.", opt_sep, NULL },

#if defined (SDL)
  { "fullscreen", "try to run in full-screen mode", opt_no_arg, NULL },
  { "hwsurface", "UNSUPPORTED", opt_no_arg, NULL },
#if defined (CYGWIN)
  { "sdlaudio", "specify the audio driver to attempt to use first, e.g. "
    "\"executor -sdlaudio waveout\" will tell Executor to use the old "
    "windows sound drivers and not use DirectSound.", opt_sep, NULL, }, 
#else
  { "sdlaudio", "specify the audio driver to attempt to use first, e.g. "
    "\"executor -sdlaudio esd\" will tell Executor to use esound, "
    "the Enlightened Sound Daemon instead of /dev/dsp.", opt_sep, NULL, }, 
#endif
#endif

#if defined (CYGWIN32) && defined (SDL)
  { "clipleak", "UNSUPPORTED (ignored)", opt_no_arg, NULL },
#endif


#if defined(X) || defined (SDL)
  { "scancodes", "different form of key mapping (may be useful in "
                 "conjunction with -keyboard)", opt_no_arg, NULL },
#endif

  { "desperate", /* Handled specially; here for documentation purposes only. */
      "run in \"desperation mode\".  This will cause Executor "
      "to use as few system features as possible, which is handy for "
      "troubleshooting if Executor is having serious problems with "
      "your system.",
	opt_no_arg, NULL },

#if defined (powerpc) || defined (__ppc__)
  { "ppc", "try to execute the PPC native code if possible (UNSUPPORTED)", opt_no_arg, NULL },
#endif

#if defined (CYGWIN32)
  { "realmodecd", "try to use real-mode cd-rom driver", opt_no_arg, NULL },
#endif

  { "appearance", "(mac or windows) specify the appearance of windows and "
    "menus.  For example \"executor -appearance windows\" will make each "
    "window have a blue title bar", opt_sep, NULL },

  { "hfsplusro", "unsupported -- do not use", opt_no_arg, NULL },
};
  
opt_database_t Executor::common_db;


/* Prints the specified value in a representation appropriate for command
 * line switches.
 */
static void
print_command_line_value (FILE *fp, int v)
{
  if (v > 0 && v % 1024 == 0)
    {
      if (v % (1024 * 1024) == 0)
	fprintf (fp, "%dM", v / (1024 * 1024));
      else
	fprintf (fp, "%dK", v / 1024);
    }
  else
    fprintf (fp, "%d", v);
}

static void
check_arg (char *argname, int *arg, int min, int max)
{
  if (*arg < min || *arg > max)
    {
      fprintf (stderr, "%s: invalid value for `%s': must be between ",
	       program_name, argname);
      print_command_line_value (stderr, min);
      fputs (" and ", stderr);
      print_command_line_value (stderr, max);
      fputs (" inclusive.\n", stderr);
      bad_arg_p = TRUE;
    }
}

#if defined (NEED_WAIT4)

/*
 * NOTE: This is a replacement for wait4 that will work for what we use wait4.
 *	 It has the side effect of acknowledging the death of other children.
 */

A4(PUBLIC, LONGINT, wait4, LONGINT, pid, union wait *, statusp, LONGINT,
					      options, struct rusage *, rusage)
{
    LONGINT retval;

    do
      {
	retval = wait3(statusp, options, rusage);
      }
    while (retval > 0 && retval != pid);

    return retval;
}
#endif /* NEED_WAIT4 */

PUBLIC char    Executor::ROMlib_startdir[MAXPATHLEN];
PUBLIC INTEGER ROMlib_startdirlen;
#if defined (MSDOS) || defined (CYGWIN32)
PUBLIC char ROMlib_start_drive;
#endif
PUBLIC char *Executor::ROMlib_appname;

#if !defined(LINUX)
#define SHELL "/bin/sh"
#define WHICH "which "
#else
#define SHELL "/bin/bash"
#define WHICH "type -path "
#endif

#define APPWRAP	"/Executor.app"

ULONGINT ROMlib_ourmtime;

#if defined (MACOSX_)

#include <mach-o/loader.h>
#include <mach-o/swap.h>

extern void willsetperms( void );
extern void badfilesystem( void );
namespace Executor {
	PRIVATE void misc_self_examination(char*);
	PRIVATE void setstartdir(char *);
}

A1(PRIVATE, void, misc_self_examination, char *, us)
{
    struct mach_header head;
    struct load_command load;
    struct segment_command seg;
    struct fat_header fat_head;
    struct fat_arch arch;
    struct stat sbuf;
    int i;
    BOOLEAN need_to_swap;
    enum NXByteOrder targetorder;
    LONGINT err;
    FILE *ROMlib_fp;

    if (!(ROMlib_fp = Ufopen(us, "r+"))) {
	if (!(ROMlib_fp = Ufopen(us, "r"))) {
	    fprintf(stderr, "couldn't open '%s'\n", us);
	    exit(3);
	}
    }
    if (fread(&fat_head, sizeof(fat_head), 1, ROMlib_fp) != 1) {
        fprintf(stderr, "couldn't read fat_header\n");
        exit(4);
    }
    if (fat_head.magic == CLC(FAT_MAGIC)) {
/*
 * NOTE: we assume that every architecture supported will have a "lowseg"
 *	 segment, hence we can just use the first one we find.  If the
 *	 binary is later thinned, the user will have to reregister anyway.
 */
	if (fread(&arch, sizeof(arch), 1, ROMlib_fp) != 1) {
	    fprintf(stderr, "couldn't read arch info\n");
	    exit(6);
	}

	if (fseek(ROMlib_fp, BigEndianValue(arch.offset), SEEK_SET) == -1) {
	    fprintf(stderr, "couldn't seek after load seg command\n");
	    exit(17);
	}
    } else {
	rewind(ROMlib_fp);
	arch.offset = 0;
    }

    if (fread(&head, sizeof(head), 1, ROMlib_fp) != 1) {
        fprintf(stderr, "couldn't read header\n");
        exit(4);
    }

/*
 * NOTE: In the following few sections of code we have to accept the
 *	 possibility that the header information will be swapped relative
 *	 to us.  Normally we only care when things are swapped relative
 *	 to a big endian machine.  But if the first architecture we find
 *	 is a big endian one and we're little endian, then we still have
 *	 to do swappage.  The alternative would be to store the registration
 *	 information in the "lowseg" of the architecture that we're currently
 *	 running on, but that means that for N archtectures, Executor would
 *	 have to be registered N times.  Ick.
 */

#if	defined(LITTLEENDIAN)
	targetorder = NX_LittleEndian;
#else	/* !defined(LITTLEENDIAN) */
	targetorder = NX_BigEndian;
#endif	/* !defined(LITTLEENDIAN) */

    if ((need_to_swap = head.magic != MH_MAGIC))
	swap_mach_header(&head, targetorder);

    if (head.magic != MH_MAGIC) {
        fprintf(stderr, "bad magic number\n");
        exit(5);
    }

    for (i = 0; i < head.ncmds; ++i) {
	if (fread(&load, sizeof(load), 1, ROMlib_fp) != 1) {
	    fprintf(stderr, "couldn't read load command\n");
	    exit(6);
	}
	if (need_to_swap)
	    swap_load_command(&load, targetorder);
	if (load.cmd == LC_SEGMENT) {
	    if (fread(&seg.segname, sizeof(seg) - sizeof(load), 1,
							 ROMlib_fp) != 1) {
		fprintf(stderr, "couldn't read load segment command\n");
		exit(7);
	    }
	    if (need_to_swap)
		swap_segment_command(&seg, targetorder);
	    if (strcmp(seg.segname, "hfs_xfer") == 0) {
		ROMlib_xfervmaddr = (char *) seg.vmaddr;
		ROMlib_xfervmsize = seg.vmsize;
	    }
	    if (fseek(ROMlib_fp, load.cmdsize - sizeof(seg), SEEK_CUR) ==
								      -1) {
		fprintf(stderr, "couldn't seek after load seg command\n");
		exit(17);
	    }
	} else {
	    if (fseek(ROMlib_fp, load.cmdsize - sizeof(load), SEEK_CUR) ==
								      -1) {
		fprintf(stderr, "couldn't seek after load command\n");
		exit(8);
	    }
	}
    }
    fclose (ROMlib_fp);

#if	!defined(LETGCCWAIL)
    err = 0;
#endif
    if (Ustat(us, &sbuf) == 0) {
	if (!geteuid() && (sbuf.st_uid || (sbuf.st_mode & 07777) != 04755))
	    willsetperms();
	if ((geteuid() != 0) && (sbuf.st_uid == 0) &&
					     ((sbuf.st_mode & 04000) == 04000))
	    badfilesystem();
	if (sbuf.st_uid)
	    err = Uchown(us, 0, -1);
	if ((!sbuf.st_uid || err == 0) && (sbuf.st_mode & 07777) != 04755)
	    Uchmod(us, 04755);
    }
}
#endif /* defined (MACOSX_) */

#if defined(MSDOS) || defined (CYGWIN32)
PUBLIC char ROMlib_savecwd[MAXPATHLEN];

PRIVATE void cd_back( void )
{
  if (ROMlib_savecwd[0])
    Uchdir(ROMlib_savecwd);
}
#endif

PRIVATE void
set_appname (char *argv0)
{
  ROMlib_appname = strrchr(argv0, '/');
#if defined (MSDOS) || defined (CYGWIN32)
  if (!ROMlib_appname)
    ROMlib_appname = strrchr (argv0, '\\');
#endif  
  if (ROMlib_appname)
    ++ROMlib_appname;
  else
    ROMlib_appname = argv0;
}

A1(PRIVATE, void, setstartdir, char *, argv0)
{
#if !defined(MSDOS) && !defined(CYGWIN32)
    LONGINT p[2], pid;
    char buf[MAXPATHLEN];
    INTEGER nread, arg0len;
    char *lookhere, *suffix, *whichstr;
    char savedir[MAXPATHLEN];

    if (argv0[0] == '/' || Uaccess(argv0, X_OK) == 0)
	lookhere = argv0;
    else
      {
		  pipe(p);
	if ((pid = fork()) == 0) {
	    ROMlib_setuid(getuid());
	    close(1);
	    dup(p[1]);
	    arg0len = strlen(argv0);
	    whichstr = (char*)alloca(arg0len + sizeof(WHICH));
	    memmove(whichstr, WHICH, sizeof(WHICH) - 1);
	    memmove(whichstr + sizeof(WHICH) - 1, argv0, arg0len + 1);
	    execl(SHELL, "sh", "-c", whichstr, (char *) 0);
	    fprintf (stderr, "``%s -c \"%s\"'' failed\n", SHELL, whichstr);
	    fflush (stderr);
	    _exit (127);
	    /* NOTREACHED */
#if !defined (LETGCCWAIL)
	    lookhere = 0;
#endif /* LETGCCWAIL */
	} else {
	    close (p[1]);
	    nread = read(p[0], buf, sizeof(buf)-1);
            wait4(pid, 0, 0, (struct rusage *) 0);
	    if (nread)
		--nread;	/* get rid of trailing \n */
	    buf[nread] = 0;
	    lookhere = buf;
	}
      }
#if defined(MACOSX_)
    misc_self_examination (lookhere);
#endif
    suffix = rindex(lookhere, '/');
    if (suffix)
      *suffix = 0;
    getcwd(savedir, sizeof savedir);
    Uchdir(lookhere);
    if (suffix)
      *suffix = '/';
    getcwd(ROMlib_startdir, sizeof ROMlib_startdir);
    Uchdir(savedir);
    ROMlib_startdirlen = strlen(ROMlib_startdir) + 1;
#if defined(MACOSX_)
    if (strcmp(ROMlib_startdir + ROMlib_startdirlen - sizeof(APPWRAP),
							       APPWRAP) != 0) {
	memmove(ROMlib_startdir + ROMlib_startdirlen - 1, APPWRAP,
		sizeof(APPWRAP));
	ROMlib_startdirlen += sizeof(APPWRAP) - 1;
    }
#endif
#else /* defined(MSDOS) || defined(CYGWIN32) */

#if defined (MSDOS) || defined (CYGWIN32)
    atexit (call_cleanup_bat);
#endif

/*
 * When run under DOS, argv[0] contains a full path name, including
 * a dos-drive and colon at the beginning (e.g. C:/fratzand/executor).
 * However, you get neither when running under DOS under gdb.  Yahoo.
 */
    if (!getcwd(ROMlib_savecwd, sizeof(ROMlib_savecwd)))
      ROMlib_savecwd[0] = 0;
    else
      atexit(cd_back);
    if (argv0[1] == ':')
      {
	char *lastslash;
#if 1
	static char cd_to[] = "C:/";

	ROMlib_start_drive = argv0[0];
	cd_to[0] = argv0[0];
	Uchdir(cd_to);
#if 0
	strcpy(ROMlib_startdir, argv0 + 2);
#else
	/* We now include the drive letter in with ROMlib_startdir. */
	   
	strcpy(ROMlib_startdir, argv0);
#endif

#else
	strcpy(ROMlib_startdir, argv0);
#endif
	lastslash = strrchr(ROMlib_startdir, '/');
#if defined (CYGWIN32)
	if (!lastslash)
	  lastslash = strrchr(ROMlib_startdir, '\\');
#endif
	if (lastslash)
	  *lastslash = 0;
      }
    else
      {
	ROMlib_start_drive = 0;
	strcpy(ROMlib_startdir, ".");
      }
    ROMlib_startdirlen = strlen(ROMlib_startdir);
#endif /* defined(MSDOS) */
}

PUBLIC BOOLEAN Executor::ROMlib_startupscreen = TRUE;

char *Executor::program_name;


#if defined (SYN68K)
static syn68k_addr_t
unhandled_trap (syn68k_addr_t callback_address, void *arg)
{
  static const char *trap_description[] = {
    /* I've only put the really interesting ones in. */
    NULL, NULL, NULL, NULL,
    "Illegal instruction",
    "Integer divide by zero",
    "CHK/CHK2",
    "FTRAPcc, TRAPcc, TRAPV",
    "Privilege violation",
    "Trace",
    "A-line",
    "FPU",
    NULL,
    NULL,
    "Format error"
  };
  int trap_num;
  char pc_str[128];

  trap_num = (long) arg;

  switch (trap_num) {
  case 4:   /* Illegal instruction. */
  case 8:   /* Privilege violation. */
  case 10:  /* A-line. */
  case 11:  /* F-line. */
  case 14:  /* Format error. */
  case 15:  /* Uninitialized interrupt. */
  case 24:  /* Spurious interrupt. */
  case 25:  /* Level 1 interrupt autovector. */
  case 26:  /* Level 2 interrupt autovector. */
  case 27:  /* Level 3 interrupt autovector. */
  case 28:  /* Level 4 interrupt autovector. */
  case 29:  /* Level 5 interrupt autovector. */
  case 30:  /* Level 6 interrupt autovector. */
  case 31:  /* Level 7 interrupt autovector. */
  case 32:  /* TRAP #0 vector. */
  case 33:  /* TRAP #1 vector. */
  case 34:  /* TRAP #2 vector. */
  case 35:  /* TRAP #3 vector. */
  case 36:  /* TRAP #4 vector. */
  case 37:  /* TRAP #5 vector. */
  case 38:  /* TRAP #6 vector. */
  case 39:  /* TRAP #7 vector. */
  case 40:  /* TRAP #8 vector. */
  case 41:  /* TRAP #9 vector. */
  case 42:  /* TRAP #10 vector. */
  case 43:  /* TRAP #11 vector. */
  case 44:  /* TRAP #12 vector. */
  case 45:  /* TRAP #13 vector. */
  case 46:  /* TRAP #14 vector. */
  case 47:  /* TRAP #15 vector. */
  case 3:   /* Address error. */
  case 6:   /* CHK/CHK2 instruction. */
  case 7:   /* FTRAPcc, TRAPcc, TRAPV instructions. */
  case 9:   /* Trace. */
  case 5:   /* Integer divide by zero. */
    sprintf (pc_str, "0x%lX", (unsigned long) READUL (EM_A7 + 2));
    break;
  default:
    strcpy (pc_str, "<unknown>");
    break;
  }

  if (trap_num < (int) NELEM (trap_description)
      && trap_description[trap_num] != NULL)
    gui_fatal ("Fatal error:  unhandled trap %d at pc %s (%s)",
	       trap_num, pc_str, trap_description[trap_num]);
  else
    gui_fatal ("Fatal error:  unhandled trap %d at pc %s", trap_num, pc_str);

  /* It will never get here; this is just here to quiet gcc. */
  return callback_address;
}


static void
setup_trap_vectors (void)
{
  syn68k_addr_t timer_callback;
  int i;

  /* Set up the trap vector for the timer interrupt. */
  timer_callback = callback_install (catchalarm, NULL);
  *(syn68k_addr_t *)SYN68K_TO_US(M68K_TIMER_VECTOR * 4) = BigEndianValue (timer_callback);

  /* Fill in unhandled trap vectors so they cause graceful deaths.
   * Skip over those trap vectors which are known to have legitimate
   * values.
   */
  for (i = 1; i < 64; i++)
    if (i != 10                            /* A line trap.       */
#if defined (M68K_TIMER_VECTOR)
	&& i != M68K_TIMER_VECTOR          /* timer interrupt.   */
#endif

#if defined (M68K_WATCHDOG_VECTOR)
	&& i != M68K_WATCHDOG_VECTOR       /* watchdog timer interrupt. */
#endif

#if defined (M68K_EVENT_VECTOR)
	&& i != M68K_EVENT_VECTOR
#endif

#if defined (M68K_MOUSE_MOVED_VECTOR)
	&& i != M68K_MOUSE_MOVED_VECTOR
#endif

#if defined (M68K_SOUND_VECTOR)
	&& i != M68K_SOUND_VECTOR
#endif

	&& i != 0x58 / 4)                  /* Magic ARDI vector. */
      {
	syn68k_addr_t c;
	c = callback_install (unhandled_trap, (void *) i);
	*(syn68k_addr_t *)SYN68K_TO_US(i * 4) = BigEndianValue (c);
      }
}
#endif /* SYN68K */


static void illegal_mode (void) __attribute__ ((noreturn));

static void
illegal_mode (void)
{
  const vdriver_modes_t *vm;
  int num_sizes, max_width, max_height, max_bpp;

  vm = vdriver_mode_list;
  num_sizes = vm->num_sizes;
  max_bpp = vdriver_max_bpp;
  if (num_sizes)
    {
      max_width = vm->size[num_sizes - 1].width;
      max_height = vm->size[num_sizes - 1].height;
    }
  else
    max_width = max_height = 0;  /* quiet gcc */

  vdriver_shutdown ();  /* Just to be safe. */
  if (num_sizes == 0)
    fprintf (stderr, "Unable to find legal graphics mode.\n");
  else
    {
      fprintf (stderr, "Invalid graphics mode.  Largest allowable "
	       "screen %dx%d, %d bits per pixel.\n",
	       max_width, max_height, max_bpp);
#if defined (MSDOS)
      if (!vesa_version)
	{
	  fputs ("No VESA driver detected.  To get more video modes, "
		 "make sure you have a\n"
		 "VESA-compatible video driver and a SuperVGA board.\n",
		 stderr);
	}
#endif
    }

  /* Don't call ExitToShell here; ROMlib isn't set up enough yet to do that. */
  exit (-1);
}

#if defined (MSDOS) || defined (CYGWIN32)
PUBLIC uint32 ROMlib_macdrives;  /* default computed at runtime */
PUBLIC uint32 ROMlib_dosdrives = ~0;
PRIVATE uint32 skipdrives = 0;

PRIVATE boolean_t
drive_number_from_letter (char c, int *nump)
{
  boolean_t retval;

  if (isupper (c) || ((c >= '[' && c <= '`')))
    {
      *nump = c - 'A';
      retval = TRUE;
    }
  else if (islower (c))
    {
      *nump = c - 'a';
      retval = TRUE;
    }
  else
    retval = FALSE;
  return retval;
}

PRIVATE void
drive_error (const char *opt)
{
  fprintf (stderr, "Invalid drive specification.\nUse something like "
	   "\"-%s A-EJ\" (for drives A, B, C, D, E and J).\n", opt);
  bad_arg_p = TRUE;
}

PRIVATE const char *
munch_next_char (const char *p, uint32 *destp, const char *opt)
{
  const char *retval;
  int d;

  if (drive_number_from_letter (p[0], &d))
    {
      if (p[1] == '-')
	{
	  int d2;
	  if (drive_number_from_letter (p[2], &d2))
	    {
	      int lower, upper, i;

	      lower = MIN (d, d2);
	      upper = MAX (d, d2);
	      for (i = lower; i <= upper; ++i)
		*destp |= (1 << i);
	      retval = p+3;
	    }
	  else
	    {
	      drive_error (opt);
	      retval = "";
	    }
	}
      else
	{
	  *destp |= 1 << d;
	  retval = p+1;
	}
    }
  else
    {
      drive_error (opt);
      retval = "";
    }
  return retval;
}

PUBLIC uint32
parse_drive_opt (const char *opt_name, const char *opt_value)
{
  uint32 retval;
  const char *next_charp;

  retval = 0;
  for (next_charp = opt_value; *next_charp;)
    next_charp = munch_next_char(next_charp, &retval, opt_name);
  return retval;
}
#endif

#if !defined (MSDOS)
static void
print_info (void)
{
  printf ("This is %s, compiled.\n",
	  ROMlib_executor_full_name);

#if defined (i386)
  /* Print out CPU type. */
  if (arch_type == ARCH_TYPE_I386)
    printf ("CPU type is 80386.\n");
  else
    printf ("CPU type is 80486 or better.\n");
#endif /* i386 */

#define MB (1024 * 1024U)

  /* Print out actual memory size chosen. */
  printf ("Choosing %u.%02u MB for applzone, %u.%02u MB for syszone, "
	  "%u.%02u MB for stack\n",
	  ROMlib_applzone_size / MB,
	  (ROMlib_applzone_size % MB) * 100 / MB,
	  ROMlib_syszone_size / MB,
	  (ROMlib_syszone_size % MB) * 100 / MB,
	  ROMlib_stack_size / MB,
	  (ROMlib_stack_size % MB) * 100 / MB);

#undef MB
}
#endif /* !MSDOS */

/* This is to tell people about the switch from "-applzone 4096" to
 * "-applzone 4M".
 */
static void
note_memory_syntax_change (const char *arg, unsigned val)
{
  /* Make sane error messages when the supplied value is insane.
   * Otherwise, try to print an example that illustrates how to
   * use the value they are trying to generate.
   */
  if (val < 100 || val > 1000000)
    val = 2048;

  fprintf (stderr, "Specified %s is too small.  The syntax "
	   "has changed; now, for a %u.%02u\nmegabyte %s you would "
	   "say \"-%s ", arg, val / 1024, (val % 1024) * 100 / 1024,
	   arg, arg);

  if (val % 1024 == 0)
    fprintf (stderr, "%uM", val / 1024);
  else if (val < 1024)
    fprintf (stderr, "%uK", val);
  else
    fprintf (stderr, "%u.%02uM", val / 1024, (val % 1024) * 100 / 1024);

  fputs ("\"\n", stderr);

  bad_arg_p = TRUE;
}


/* Concatenates all strings in the array, separated by spaces. */
static const char *
construct_command_line_string (int argc, char **argv)
{
  char *s;
  unsigned long string_length;
  int i, j;

  for (i = 0, string_length = 0; i < argc; i++)
    string_length += strlen (argv[i]) + 1;
  s = (char*)malloc (string_length + 1);
  s[0] = '\0';
  for (j = 0; j < argc; j++)
    sprintf (s + strlen (s), "%s%s", j ? " " : "", argv[j]);
  return s;
}

PRIVATE int
zap_comments (char *buf, int n_left)
{
  char *ip, *op;
  boolean_t last_was_cr;
  int retval;

  ip = buf;
  op = buf;
  retval = 0;
  last_was_cr = TRUE;
  while (n_left > 0)
    {
      while (n_left > 0 && (*ip != '#' || !last_was_cr))
	{
	  last_was_cr = *ip == '\n' || *ip == '\r';
	  *op++ = *ip++;
	  ++retval;
	  --n_left;
	}
      if (n_left > 0)
	{
	  while (n_left > 0 && *ip != '\n' && *ip != '\r')
	    {
	      ++ip;
	      --n_left;
	    }
	}
    }
  return retval;
}

#if !defined (LINUX)

#define EXECUTOR_DIR(filename)						  \
({									  \
  char *retval;								  \
									  \
  retval = (char*)alloca (strlen (ROMlib_startdir) + 1 + strlen (filename) + 1); \
  sprintf (retval, "%s/%s", ROMlib_startdir, (filename));		  \
  retval;								  \
})

#else

#define EXECUTOR_DIR(filename)			\
({						\
  char *s;					\
  char *retval;					\
						\
  s = alloca (2 + strlen(filename) + 1);	\
  sprintf (s, "+/%s", filename);		\
  s = copystr (s);				\
  retval = alloca (strlen (s) + 1);		\
  strcpy (retval, s);				\
  free (s);					\
  retval;					\
})

#endif

PUBLIC FILE *
Executor::executor_dir_fopen (const char *file, const char *perm)
{
  char *pathname;
  FILE *retval;

  pathname = EXECUTOR_DIR (file);
  retval = fopen (pathname, perm);
  return retval;
}

PUBLIC int
Executor::executor_dir_remove (const char *file)
{
  char *pathname;
  int retval;

  pathname = EXECUTOR_DIR (file);
  retval = remove (pathname);
  return retval;
}

PRIVATE void
read_args_from_file (const char *filename, int *argcp, char ***argvpp)
{
  FILE *fp;

  fp = executor_dir_fopen (filename, "r");
  if (fp)
    {
      int nread, n_extra_params, i;
      char buf[8192], *bufp;
      char **saveargv;

      nread = fread (buf, 1, sizeof buf, fp);
      fclose (fp);
      nread = zap_comments (buf, nread);
      n_extra_params = count_params (buf, nread);
      *argcp += n_extra_params;
      saveargv = *argvpp;
      *argvpp = (char**)malloc ((*argcp+1) * sizeof *argvpp);
      bufp = buf;
      memcpy (*argvpp, saveargv, (*argcp - n_extra_params) * sizeof **argvpp);
      for (i = *argcp - n_extra_params; i < *argcp; ++i)
	(*argvpp)[i] = get_param ((const char **) &bufp, &nread);
      (*argvpp)[i] = 0;
    }
}

#if defined (CYGWIN32)
PRIVATE uint32
win_drive_to_bit (const char *drive_namep)
{
  uint32 retval;

  if (drive_namep[1] == ':')
    retval = 1 << (tolower(drive_namep[0]) - 'a');
  else
    {
      warning_unexpected ("drive name = '%s'", drive_namep);
      retval = 0;
    }
  return retval;
}
#endif

#if defined(LINUX)
#define PERSONALITY_HACK
#include <sys/personality.h>
#define READ_IMPLIES_EXEC 0x0400000
#endif

#ifdef MACOSX_
namespace Executor {
	PUBLIC int oldmain(int, char**);
}

A2 (PUBLIC, int, main, int, argc, char **, argv)
#else
int main(int argc, char** argv)
#endif
{
  check_structs ();

  INTEGER i;
  static unsigned short jmpl_to_ResourceStub[3] =
    {
      CWC ((unsigned short)0x4EF9), 0, 0		/* Filled in below. */
    };
  long l;
  ULONGINT save_trap_vectors[64];
  virtual_int_state_t int_state;
  THz saveSysZone, saveApplZone;
  Ptr saveApplLimit;
#if defined (MSDOS)
  boolean_t check_files_p;  /* See if FILES= is big enough? */
#endif
  static void (*reg_funcs[]) (void) =
    {
      vdriver_opt_register,
      NULL,
    };
  string arg;

#if defined(PERSONALITY_HACK)
  int pers;
  
  pers = personality(0xffffffff);
  if ((pers & MMAP_PAGE_ZERO) == 0)
    {
      if (personality(pers|MMAP_PAGE_ZERO|READ_IMPLIES_EXEC) == 0)
	execv(argv[0], argv);
    }
#endif

  
  int grayscale_p = FALSE;

#if defined (DISPLAY_SPLASH_SCREEN)
  boolean_t splash_screen_displayed_p;
#endif  

#if defined (VDRIVER_SVGALIB)
  /* We need to do this right away, to give up suid root privileges. */
  vga_init ();
#endif

#if defined (MSDOS)
  /* Guarantee that ds and ss are the same, so we can use
   * -fomit-frame-pointer and other fun hacks.  We need to do this
   * *right away*, before we might use %ebp as a general register.
   */
  asm ("pushl %ds\n\t"
       "popl %ss");
#endif /* MSDOS */

  ROMlib_do_custom ();
  ROMlib_command_line = construct_command_line_string (argc, argv);

  if (!arch_init ())
    {
      fprintf (stderr, "Unable to initialize CPU information.\n");
      exit (-100);
    }

  if (!os_init ())
    {
      fprintf (stderr, "Unable to initialize operating system features.\n");
      exit (-101);
    }
  
  {
    char *t;
    
    t = strrchr (*argv, '/');
    if (t)
      program_name = &t[1];
    else
      program_name = *argv;
  }
  
  /* Guarantee various time variables are set up properly. */
  msecs_elapsed ();

#if defined (MMAP_LOW_GLOBALS)
  for (i = 1; i < argc && strcmp (argv[i], "-offset") != 0; ++i)
    ;
  if (i == argc)
    mmap_lowglobals ();
#endif /* MMAP_LOW_GLOBALS */


  ROMlib_WriteWhen (WriteInOSEvent);

#if defined (MACOSX_)
  ROMlib_seteuid (0);		/* This is necessary because when people copy
				   setuid-root files from floppies, the setuid
				   bit stays, but the "root" part doesn't, so
				   even if they run as root, they're messed */
#endif

  setstartdir (argv[0]);
  set_appname (argv[0]);

#define COMMANDS "commands.txt"

  read_args_from_file (COMMANDS, &argc, &argv);
#if defined (MSDOS) || defined (CYGWIN32)
  read_args_from_file (CHECKPOINT_FILE, &argc, &argv);
  checkpointp = checkpoint_init ();
#endif

  /* Replace "-desperate" switch with what it implies.  We must do
   * this before normal command line processing.
   */
  if (!handle_desperate_switch (&argc, &argv))
    exit (-1);

  opt_init ();
  common_db = opt_alloc_db ();
  opt_register ("common", common_opts);
  
  opt_register_pre_note ("welcome to the executor help message.");
  opt_register_pre_note ("usage: `executor [option...] "
			 "[program [document1 document2 ...]]'");
  
  for (i = 0; reg_funcs[i]; i ++)
    (*reg_funcs[i]) ();

  if (!bad_arg_p)
    bad_arg_p = opt_parse (common_db, common_opts,
			   &argc, argv);
  
  if (opt_val (common_db, "version", NULL))
    {
      fprintf (stdout, "%s\n", EXECUTOR_VERSION);
      exit (0);
    }


/*
 * If they ask for help, it's not an error -- it should go to stdout
 */

  if (opt_val (common_db, "help", NULL))
    {
      fprintf (stdout, "%s", opt_help_message ());
      exit (0);
    }

  /* Verify that the user input a legal bits per pixel.  "0" is a legal
   * value here; that means "use the vdriver's default."
   */
  opt_int_val (common_db, "bpp", &flag_bpp, &bad_arg_p);
  if (flag_bpp != 0 && flag_bpp != 1
      && flag_bpp != 2 && flag_bpp != 4 && flag_bpp != 8
      && flag_bpp != 16 && flag_bpp != 32)
    {
      fprintf (stderr, "Bits per pixel must be 1, 2, 4, 8, 16 or 32.\n");
      bad_arg_p = TRUE;
    }

#if defined (SDL)
  if (opt_val (common_db, "fullscreen", NULL))
    ROMlib_fullscreen_p = TRUE;

  if (opt_val (common_db, "hwsurface", NULL))
    ROMlib_hwsurface_p = TRUE;
#endif

#if defined(X) || (defined (CYGWIN32) && defined (SDL))
  if (opt_val (common_db, "scancodes", NULL))
    ROMlib_set_use_scancodes (TRUE);
#endif

#if defined (SDL)

  if (opt_val (common_db, "sdlaudio", &arg))
    ROMlib_set_sdl_audio_driver_name (arg);

#endif

  if (opt_val (common_db, "ppc", NULL))
    ROMlib_set_ppc (TRUE);

  if (opt_val (common_db, "hfsplusro", NULL))
    ROMlib_hfs_plus_support = TRUE;

#if defined (CYGWIN32)
  if (opt_val (common_db, "realmodecd", NULL))
    ROMlib_set_realmodecd (TRUE);
#endif

  if (opt_val (common_db, "size", &arg))
    bad_arg_p |= !parse_size_opt ("size", arg);

  if (opt_val (common_db, "prres", &arg))
    bad_arg_p |= !parse_prres_opt (&ROMlib_optional_res_x,
				   &ROMlib_optional_res_y, arg);

  if (opt_val (common_db, "debug", &arg))
    bad_arg_p |= !error_parse_option_string (arg);

#if defined (MSDOS) || defined (CYGWIN32)
  if (opt_val (common_db, "macdrives", &arg))
    ROMlib_macdrives = parse_drive_opt ("macdrives", arg);
  else
    {
#if !defined (CYGWIN32)
      ROMlib_macdrives = 3; /* A: + B: */
      {
	int cdrom;

	cdrom = dosdisk_find_cdrom ();
	if (cdrom != -1)
	  ROMlib_macdrives |= (1 << cdrom);
      }
#else
      char buf[512];

      if (win_GetLogicalDriveStrings (sizeof buf - 1, buf))
	{
	  char *p;

	  for (p = buf; *p; p += strlen (p) + 1)
	    if (win_direct_accessible_disk (p))
	      ROMlib_macdrives |= win_drive_to_bit (p);
	}
#endif
    }

  if (opt_val (common_db, "dosdrives", &arg))
    ROMlib_dosdrives = parse_drive_opt ("dosdrives", arg);
  
  if (opt_val (common_db, "skipdrives", &arg))
    {
      skipdrives = parse_drive_opt ("skipdrives", arg);
      ROMlib_macdrives &= ~skipdrives;
      ROMlib_dosdrives &= ~skipdrives;
    }

#endif

#if defined (MSDOS)
  {
    int dangerous_video_p = FALSE;
    opt_int_val (common_db, "dangerousvideospeedup",
		 &dangerous_video_p, &bad_arg_p);
    if (!dangerous_video_p)
      opt_int_val (common_db, "videospeedup",
		   &dangerous_video_p, &bad_arg_p);
    try_to_use_fat_ds_vga_hack_p = dangerous_video_p;
  }

  opt_int_val (common_db, "nosync", &ROMlib_nosync, &bad_arg_p);

  opt_int_val (common_db, "skipaspi", &ROMlib_skipaspi, &bad_arg_p);

  {
    int only_vga_p = FALSE;
    opt_int_val (common_db, "vga", &only_vga_p, &bad_arg_p);
    only_use_vga_p = only_vga_p;
  }
#endif

  {
    int skip;
    skip = 0;
    opt_int_val (common_db, "nosound", &skip, &bad_arg_p);
    sound_disabled_p = (skip != 0);
  }

  {
    int nocache;
    nocache = 0;
    opt_int_val (common_db, "nodiskcache", &nocache, &bad_arg_p);
    dcache_set_enabled (!nocache);
  }

#if defined (SYN68K)  
  use_native_code_p = ! opt_val (common_db, "notnative", NULL);
#endif  

  afpd_conventions_p = opt_val (common_db, "afpd", NULL);
  netatalk_conventions_p = (afpd_conventions_p ||
			    opt_val (common_db, "netatalk", NULL));
  if (netatalk_conventions_p)
    {
      apple_double_quote_char = ':';
      apple_double_fork_prefix = ".AppleDouble/";
    }
  else
    {
      apple_double_quote_char = '%';
      apple_double_fork_prefix = "%";
    }
  apple_double_fork_prefix_length = strlen (apple_double_fork_prefix);

  substitute_fonts_p = !opt_val (common_db, "cities", NULL);

  if (opt_val (common_db, "offset", NULL))
    force_big_offset = TRUE;

#if defined (CYGWIN32)
  if (opt_val (common_db, "die", NULL))
    uninstall_exception_handler ();
  if (opt_val (common_db, "noautoevents", NULL))
    set_timer_driven_events (FALSE);
#endif

#if defined (MSDOS) || defined (CYGWIN32)
  if (opt_val (common_db, "nocheckpoint", NULL))
    disable_checkpointing ();
#endif

  /* Parse the "-memory" option. */
  {
    int total_memory;
    if (opt_int_val (common_db, "memory", &total_memory, &bad_arg_p))
      {
	check_arg ("memory", &total_memory,
		   (MIN_APPLZONE_SIZE + DEFAULT_SYSZONE_SIZE
		    + DEFAULT_STACK_SIZE),
		   (MAX_APPLZONE_SIZE + DEFAULT_SYSZONE_SIZE
		    + DEFAULT_STACK_SIZE));

	/* Set up the three memory sizes appropriately.  For now we
	 * just allocate the defaults for syszone and stack, and
	 * put everything else in -applzone.
	 */
	ROMlib_syszone_size  = DEFAULT_SYSZONE_SIZE;
	ROMlib_stack_size    = DEFAULT_STACK_SIZE;
	ROMlib_applzone_size = (total_memory - ROMlib_syszone_size
				- ROMlib_stack_size);
      }
  }
  
  /* I bumped the minimal ROMlib_applzone to 512, since Loser needs
     more than 256.  I guess it's a little unfair to people who bypass
     Loser, but it will prevent confusion.  */
  opt_int_val (common_db, "applzone", &ROMlib_applzone_size, &bad_arg_p);
  if (ROMlib_applzone_size < 65536)
    note_memory_syntax_change ("applzone", ROMlib_applzone_size);
  else
    check_arg ("applzone", &ROMlib_applzone_size, MIN_APPLZONE_SIZE,
	       MAX_APPLZONE_SIZE);
  
  opt_int_val (common_db, "syszone", &ROMlib_syszone_size, &bad_arg_p);
  if (ROMlib_syszone_size < 65536)
    note_memory_syntax_change ("syszone", ROMlib_syszone_size);
  else
    check_arg ("syszone", &ROMlib_syszone_size, MIN_SYSZONE_SIZE,
	       MAX_SYSZONE_SIZE);
  
  opt_int_val (common_db, "stack", &ROMlib_stack_size, &bad_arg_p);
  if (ROMlib_stack_size < 32768)
    note_memory_syntax_change ("stack", ROMlib_stack_size);
  else
    check_arg ("stack", &ROMlib_stack_size, MIN_STACK_SIZE, MAX_STACK_SIZE);
  
#if defined (MM_MANY_APPLZONES)
  opt_int_val (common_db, "napplzones", &mm_n_applzones, &bad_arg_p);
  check_arg ("napplzones", &mm_n_applzones, 1, /* random */ 255);
#endif /* !MM_MANY_APPLZONES */
  
  ROMlib_InitZones (force_big_offset ? offset_big : offset_none);

  {
    uint32 save_a7;

    save_a7 = EM_A7;
#if defined (SYN68K)
    /* Set up syn68k. */
    initialize_68k_emulator (vdriver_system_busy,
#if defined (__CHECKER__)
			     FALSE,
#else
			     use_native_code_p,
#endif
			     (uint32 *) SYN68K_TO_US (0),
#if defined (USE_BIOS_TIMER)
			     dos_int_flag.rm_segment * 16
#else /* !USE_BIOS_TIMER */
			     0
#endif /* !USE_BIOS_TIMER */
			     );
#endif /* SYN68K */

    EM_A7 = save_a7;
  }

  if (opt_val (common_db, "keyboards", NULL))
    graphics_p = FALSE;

  /* Block virtual interrupts, until the system is fully set up. */
  int_state = block_virtual_ints ();

#if defined (MSDOS)
  /* We must switch to a non-moving sbrk before calling vdriver_init,
   * if the user wants to try the fat %ds vga hack.  Otherwise,
   * the address of the frame buffer relative to the base of %ds
   * could change after we've already determined its value.
   */
  if (try_to_use_fat_ds_vga_hack_p)
    switch_to_non_moving_sbrk ();
#endif

  if (graphics_p && !vdriver_init (0, 0, 0, FALSE, &argc, argv))
    {
      fprintf (stderr, "Unable to initialize video driver.\n");
      exit (-12);
    }

#if defined (SYN68K)
  /* Save the trap vectors away. */
  memcpy (save_trap_vectors, SYN68K_TO_US(0), sizeof save_trap_vectors);
#endif

#if defined (MSDOS)
  {
    int use_old_timer_p = FALSE;
    opt_int_val (common_db, "oldtimer", &use_old_timer_p, &bad_arg_p);
    use_bios_timer_p = !use_old_timer_p;
  }
  {
    int no_files_check_p = FALSE;
    opt_int_val (common_db, "nofilescheck", &no_files_check_p, &bad_arg_p);
    check_files_p = !no_files_check_p;
  }
#endif

  opt_int_val (common_db, "sticky",      &ROMlib_sticky_menus_p, &bad_arg_p);

  opt_int_val (common_db, "pceditkeys",  &ROMlib_forward_del_p, &bad_arg_p);

  opt_int_val (common_db, "nobrowser",   &ROMlib_nobrowser, &bad_arg_p);

  opt_int_val (common_db, "print",	 &ROMlib_print,     &bad_arg_p);
  
#if defined (NEXT) || defined (LINUX)
  opt_int_val (common_db, "nodotfiles",  &ROMlib_no_dot_files, &bad_arg_p);
#endif
  
#if defined (NOMOUSE_COMMAND_LINE_OPTION)
  opt_int_val (common_db, "nomouse",     &ROMlib_no_mouse,  &bad_arg_p);
#endif

#if 0
  opt_int_val (common_db, "noclock",     &ROMlib_noclock,   &bad_arg_p);
#endif
  
  {
    int no_auto = FALSE;
    opt_int_val (common_db, "noautorefresh",  &no_auto, &bad_arg_p);
    do_autorefresh_p = !no_auto;
  }
  
  opt_int_val (common_db, "refresh",     &ROMlib_refresh,   &bad_arg_p);
  check_arg ("refresh", &ROMlib_refresh, 0, 60);
  
  opt_int_val (common_db, "grayscale",   &grayscale_p,  &bad_arg_p);


#if defined (LINUX)
  opt_int_val (common_db, "nodrivesearch", &nodrivesearch_p, &bad_arg_p);
#endif

  {
    string str;

    if (opt_val (common_db, "prvers", &str))
      {
	uint32 vers;

	if (!ROMlib_parse_version (str, &vers))
	  bad_arg_p = TRUE;
	else
	  ROMlib_PrDrvrVers = (vers >> 8) * 10 + ((vers >> 4) & 0xF);
      }
  }
  
#if defined (SUPPORT_LOG_ERR_TO_RAM)
  {
    int log;
    log = 0;
    opt_int_val (common_db, "ramlog", &log, &bad_arg_p);
    log_err_to_ram_p = (log != 0);
  }
#endif
  
  if (opt_val (common_db, "info", NULL))
    {
#if defined (MSDOS)
      msdos_print_info ();
#else
      print_info ();
#endif /* MSDOS */
      exit (0);
    }

#if defined (MSDOS)
  {
    int port;

    if (opt_int_val (common_db, "modemport", &port, &bad_arg_p))
      {
	if (port < 1 || port > 4)
	  {
	    fprintf (stderr, "modemport must be 1, 2, 3 or 4.\n");
	    bad_arg_p = TRUE;
	  }
	else
	  set_modem_port_mapping_to_pc_port (port);
      }

    if (opt_int_val (common_db, "printerport", &port, &bad_arg_p))
      {
	if (port < 1 || port > 4)
	  {
	    fprintf (stderr, "printerport must be 1, 2, 3 or 4.\n");
	    bad_arg_p = TRUE;
	  }
	else
	  set_printer_port_mapping_to_pc_port (port);
      }
  }
#endif
  
  /* If we failed to parse our arguments properly, exit now.
   * I don't think we should call ExitToShell yet because the
   * rest of the system isn't initialized.
   */
   if (argc >= 2)
     {
       int a;

       /* Only complain if we see something with a leading dash; anything
	* else might be a file to launch.
	*/
       for (a = 1; a < argc; a++)
	 if (argv[a][0] == '-')
	   {
	     fprintf (stderr, "%s: unknown option `%s'\n",
		      program_name, argv[a]);
	     bad_arg_p = TRUE;
	   }
     }

  filltables ();

  l = (long) ostraptable[0x0FC];
  ((unsigned char *) jmpl_to_ResourceStub)[2] = l >> 24;
  ((unsigned char *) jmpl_to_ResourceStub)[3] = l >> 16;
  ((unsigned char *) jmpl_to_ResourceStub)[4] = l >> 8;
  ((unsigned char *) jmpl_to_ResourceStub)[5] = l;
  ostraptable[0xFC]  = (void *) US_TO_SYN68K(jmpl_to_ResourceStub);
  osstuff[0xFC].orig = (void *) US_TO_SYN68K(jmpl_to_ResourceStub);

  saveSysZone = SysZone;
  saveApplZone = ApplZone;
  saveApplLimit = ApplLimit;
  memset (&nilhandle, ~0, (char *) &lastlowglobal - (char *) &nilhandle);

  setupsignals ();


  Ticks_UL.u = 0;
  nilhandle = 0;		/* so nil dereferences "work" */

  memset (&EventQueue, 0, sizeof (EventQueue));
  memset (&VBLQueue,   0, sizeof (VBLQueue));
  memset (&DrvQHdr,    0, sizeof (DrvQHdr));
  memset (&VCBQHdr,    0, sizeof (VCBQHdr));
  memset (&FSQHdr,     0, sizeof (FSQHdr));
  TESysJust   = 0;
  SysZone     = saveSysZone;
  ApplZone    = saveApplZone;
  ApplLimit   = saveApplLimit;
  BootDrive   = 0;
  DefVCBPtr   = 0;
  CurMap      = 0;
  TopMapHndl  = 0;
  DSAlertTab  = 0;
  ResumeProc  = 0;
  SFSaveDisk  = 0;
  GZRootHnd   = 0;
  ANumber     = 0;
  ResErrProc  = 0;
  FractEnable = 0;
  SEvtEnb     = 0;
  MenuList    = 0;
  MBarEnable  = 0;
  MenuFlash   = 0;
  TheMenu     = 0;
  MBarHook    = 0;
  MenuHook    = 0;
  MenuCInfo   = NULL;
  HeapEnd     = 0;
  ApplLimit   = 0;
  SoundActive = soundactiveoff;
  PortBUse    = 2;			/* configured for Serial driver */
  memset (KeyMap, 0, sizeof_KeyMap);
  if (vdriver_grayscale_p || grayscale_p)
    {
      /* Choose a nice light gray hilite color. */
      HiliteRGB.red   = CWC (0xAAAA);
      HiliteRGB.green = CWC (0xAAAA);
      HiliteRGB.blue  = CWC (0xAAAA);
    }
  else
    {
      /* how about a nice yellow hilite color? */
      HiliteRGB.red   = CWC (0xFFFF);
      HiliteRGB.green = CWC (0xFFFF);
      HiliteRGB.blue  = CWC (0);
    }
  
  {
    static uint16 ret = CWC (0x4E75);
    
    JCrsrTask = (ProcPtr) RM (&ret);
  }
  
  SET_HILITE_BIT ();
  TheGDevice = MainDevice = DeviceList = CLC_NULL;

  OneOne     = CLC (0x00010001);
  Lo3Bytes   = CLC (0xFFFFFF);
  DragHook   = 0;
  TopMapHndl = 0;
  SysMapHndl = 0;
  MBDFHndl   = 0;
  MenuList   = 0;
  MBSaveLoc  = 0;

  SysVersion = BigEndianValue (system_version);
  FSFCBLen = CWC (94);
  ScrapState = CWC (-1);

#if defined (MSDOS)
  switch_to_non_moving_sbrk ();
#endif

#if defined (MSDOS)
  /* Make sure FILES= is big enough. */
  if (check_files_p && !msdos_test_max_files ())
    exit (-39);

  /* Make sure we have at least 0.75 megabytes of memory remaining, so
   * that syn68k will have a big enough working area.  This figure is
   * somewhat arbitrary, but we really don't want the user cutting it
   * particularly close and then complain about lousy performance and
   * flaky behavior.
   */
  if (!msdos_check_memory_remaining (768 * 1024))
    {
      vdriver_shutdown ();
      print_mem_full_message ();
      exit (-50);
    }

#endif

#if defined(NEXT) && !defined(SYN68K)
  ROMlib_install_ardi_mods ();
#endif /* NEXT && !SYN68K */

#if defined (NEXT)
  ROMlib_determine040ness ();
  ROMlib_checkadb ();
#endif /* NEXT */

  TheZone = SysZone;
  UTableBase =
    (DCtlHandlePtr) (long) RM (NewPtr (sizeof (UTableBase[0].p) * NDEVICES));
  memset (MR (UTableBase), 0, sizeof (UTableBase[0].p) * NDEVICES);
  UnitNtryCnt = BigEndianValue (NDEVICES);
  TheZone = ApplZone;

  if (graphics_p)
    {
      /* Set up the current graphics mode appropriately. */
      if (!vdriver_set_mode (flag_width, flag_height, flag_bpp, grayscale_p))
	illegal_mode ();
      
      /* initialize the mac rgb_spec's */
      make_rgb_spec (&mac_16bpp_rgb_spec,
		     16, TRUE, 0,
		     5, 10, 5, 5, 5, 0,
		     BigEndianValue (GetCTSeed ()));
      
      make_rgb_spec (&mac_32bpp_rgb_spec,
		     32, TRUE, 0,
		     8, 16, 8, 8, 8, 0,
		     BigEndianValue (GetCTSeed ()));
      
      gd_allocate_main_device ();
    }

#if defined (DISPLAY_SPLASH_SCREEN)  
  if (graphics_p)
    splash_screen_displayed_p = splash_screen_display (FALSE, "splash");
#endif

  ROMlib_eventinit (graphics_p);
  hle_init ();
  
  ROMlib_fileinit ();

  InitUtil ();
  
#if !defined (X) && !defined (SDL)
/* #warning "Hack so we don't smash mouse/keyboard m68k interrupt vectors." */
#if defined (M68K_EVENT_VECTOR)
  save_trap_vectors[M68K_EVENT_VECTOR]
    = *(syn68k_addr_t *)SYN68K_TO_US(M68K_EVENT_VECTOR * 4);
#endif
#if defined (M68K_MOUSE_MOVED_VECTOR)
  save_trap_vectors[M68K_MOUSE_MOVED_VECTOR]
    = *(syn68k_addr_t *)SYN68K_TO_US(M68K_MOUSE_MOVED_VECTOR * 4);
#endif
#endif

  {
    string appearance_str;

    if (opt_val (common_db, "appearance", &appearance_str))
      bad_arg_p |= !ROMlib_parse_appearance (appearance_str.c_str());
  }

  InitResources ();

  /* parse the `-system' option */
  {
    string system_str;
    
    if (opt_val (common_db, "system", &system_str))
      bad_arg_p |= !parse_system_version (system_str);
    else
      ROMlib_set_system_version (system_version);
  }

  if (bad_arg_p)
    {
      fprintf (stderr,
	       "Type \"%s -help\" for a list of command-line options.\n",
	       program_name);
      exit (-10);
    }

  {
    boolean_t keyboard_set_failed;
    
    if (opt_val (common_db, "keyboard", &arg))
      {
	keyboard_set_failed = !ROMlib_set_keyboard (arg.c_str());
	if (keyboard_set_failed)
	  printf ("``%s'' is not an available keyboard\n", arg.c_str());
      }
    else
      keyboard_set_failed = FALSE;

    if (keyboard_set_failed || opt_val (common_db, "keyboards", NULL))
      display_keyboard_choices ();
  }

  ROMlib_seginit (argc, argv);
  
  InitFonts ();
  
  
#if !defined (NDEBUG)
  dump_init (NULL);
#endif
  
  /* see qColorutil.c */
  ROMlib_color_init ();
  
  wind_color_init ();
  /* must be called after `ROMlib_color_init ()' */
  image_inits ();
  
  /* must be after `image_inits ()' */
  sb_ctl_init ();
  
  AE_init ();
  
  {
    INTEGER env = 0;
    ROMlib_Fsetenv (&env, 0);
  }

  TEDoText = RM ((ProcPtr) P_ROMlib_dotext);	/* where should this go ? */

#if defined (SYN68K)
  {
    LONGINT save58;

    save58 = *(LONGINT *) SYN68K_TO_US(0x58);
    /* Replace the trap vectors which got smashed during initialization. */
    memcpy (SYN68K_TO_US (0), save_trap_vectors, sizeof save_trap_vectors);
    *(LONGINT *) SYN68K_TO_US(0x58) = save58;

    setup_trap_vectors ();
  }

  /* Set up timer interrupts.  We need to do this after everything else
   * has been initialized.
   */
  if (!syncint_init ())
    {
      vdriver_shutdown ();
      fputs ("Fatal error:  unable to initialize timer.\n", stderr);
      exit (-11);
    }

#endif /* SYN68K */

  sound_init ();
  
  set_refresh_rate (ROMlib_refresh);
  
  restore_virtual_ints (int_state);

  WWExist = QDExist = EXIST_NO;

#if defined (CYGWIN32)
  complain_if_no_ghostscript ();
#endif
  
  executor_main ();

  if (!ROMlib_no_windows)
    ExitToShell ();
  else
    exit (0);
  /* NOT REACHED */
  return 0; 
}
