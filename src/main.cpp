/* Copyright 1988 - 2005 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

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
#include "rsys/pstuff.h"
#include "rsys/prefs.h"
#include "rsys/flags.h"
#include "rsys/option.h"
#include "rsys/host.h"
#include "rsys/syncint.h"
#include "rsys/vdriver.h"
#include "rsys/vbl.h"
#include "rsys/time.h"
#include "rsys/segment.h"
#include "rsys/version.h"
#include "rsys/m68kint.h"
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
#include "rsys/gestalt.h"
#include "rsys/keyboards.h"
#include "rsys/launch.h"
#include "rsys/text.h"
#include "rsys/appearance.h"
#include "rsys/hfs_plus.h"

#include "rsys/check_structs.h"

#include "paramline.h"

PRIVATE void setstartdir(char *);

#include <ctype.h>

#if !defined(WIN32)
#include <sys/wait.h>
#endif

#if defined(CYGWIN32)
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
using namespace std;

PUBLIC int Executor::ROMlib_noclock = 0;

#if defined(NOMOUSE_COMMAND_LINE_OPTION)
PUBLIC int ROMlib_no_mouse = 1;
#endif

/* optional resolution other than 72dpix72dpi for printing */
PUBLIC INTEGER Executor::ROMlib_optional_res_x, Executor::ROMlib_optional_res_y;

/* A string approximating the original command line. */
PUBLIC const char *Executor::ROMlib_command_line;

/* Set to true if there was any error parsing arguments. */
PRIVATE bool bad_arg_p = false;

/* Set to false when we know that the command line switches will result
   in no graphics. */

PRIVATE bool graphics_p = true;

/* for a description of flags declared here, see <rsys/flags.h> */

/* Initial screen size.  This can be changed dynamically. */
INTEGER Executor::flag_width = 0, Executor::flag_height = 0; /* 0 means "use default". */

/* Initial bits per pixel.  The screen depth can be changed dynamically. */
int Executor::flag_bpp = 0; /* 0 means "use default". */

INTEGER Executor::ROMlib_shadow_screen_p = true;

INTEGER Executor::ROMlib_no_windows;

#if defined(MSDOS) || defined(CYGWIN32)
int ROMlib_drive_check = 0;
#endif

static bool use_native_code_p = true;

/* the system version that executor is currently reporting to
   applications, set through the `-system' options.  contains the
   version number in the form `0xABC' which corresponds to system
   version A.B.C */
uint32_t Executor::system_version = 0x700; /* keep this in sync with Browser's .ecf file */

const option_vec Executor::common_opts = {
    { "sticky", "sticky menus", opt_no_arg, "" },
    { "pceditkeys", "have Delete key delete one character forward",
      opt_no_arg, "" },
    { "nobrowser", "don't run Browser", opt_no_arg, "" },
    { "bpp", "default screen depth", opt_sep, "" },
    { "size", "default screen size", opt_sep, "" },
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

      opt_sep, "" },
    { "nodiskcache", "disable internal disk cache.",
      opt_no_arg, "" },
    { "nosound", "disable any sound hardware",
      opt_no_arg, "" },
    { "info", "print information about your system",
      opt_no_arg, "" },
#if defined(SUPPORT_LOG_ERR_TO_RAM)
    { "ramlog",
      "log debugging information to RAM; alt-shift-7 dumps the "
      "accrued error log out via the normal mechanism.",
      opt_no_arg, "" },
#endif

#if defined(MSDOS) || defined(CYGWIN32)
    { "macdrives", "drive letters that represent Mac formatted media",
      opt_sep, "" },
    { "dosdrives", "drive letters that represent DOS formatted media",
      opt_sep, "" },
    { "skipdrives", "drive letters that represent drives to avoid",
      opt_sep, "" },
#endif
#if defined(LINUX)
    { "nodrivesearch",
      "Do not look for a floppy drive, CD-ROM drive or any other drive "
      "except as specified by the MacVolumes environment variable",
      opt_no_arg, "" },
#endif /* LINUX */
    { "keyboards", "list available keyboard mappings",
      opt_no_arg, "" },
    { "keyboard", "choose a specific keyboard map", opt_sep, "" },
    { "print",
      "tell program to print file; not useful unless you also "
      "specify a program to run and one or more documents to print.",
      opt_no_arg, "" },
#if defined(MACOSX_) || defined(LINUX)
    { "nodotfiles", "do not display filenames that begin with dot", opt_no_arg,
      "" },
#endif
#if 0
  { "noclock",     "disable timer",               opt_no_arg,   "" },
#endif

#if defined(NOMOUSE_COMMAND_LINE_OPTION)
    /* Hack Dr. Chung wanted */
    { "nomouse", "ignore missing mouse", opt_no_arg, "" },
#endif
    { "noautorefresh",
      "turns off automatic detection of programs that bypass QuickDraw.",
      opt_no_arg, "" },
    {
        "refresh",
        "handle programs that bypass QuickDraw, at a performance penalty.  "
        "Follow -refresh with an number indicating how many 60ths of a second "
        "to wait between each screen refresh, e.g. \"executor -refresh 10\".",
        opt_optional, "10",
    },
    {
        "help", "print this help message", opt_no_arg, "",
    },
    {
        "version", "print the Executor version", opt_no_arg, "",
    },
#if defined(MALLOC_MAC_MEMORY)
    {
        "memory",
        "specify the total memory you want reserved for use by the programs "
        "run under Executor and for Executor's internal system software.  "
        "For example, \"executor -memory 5.5M\" would "
        "make five and a half megabytes available to the virtual machine.  "
        "Executor will require extra memory above and beyond this amount "
        "for other uses.",
        opt_sep, "",
    },
    {
        "applzone",
        "specify the memory to allocate for the application being run, "
        "e.g. \"executor -applzone 4M\" would make four megabytes "
        "of RAM available to the application.  \"applzone\" stands for "
        "\"application zone\".",
        opt_sep, "",
    },
    {
        "stack",
        "like -applzone, but specifies the amount of stack memory to allocate.",
        opt_sep, "",
    },
    {
        "syszone", "like -applzone, but specifies the amount of memory to make "
                   "available to Executor's internal system software.",
        opt_sep, "",
    },
#if defined(MM_MANY_APPLZONES)
    { "napplzones",
      "debugging flag, specifies the number of applzones to use",
      opt_sep, "" },
#endif /* MM_MANY_APPLZONES */
#endif /* MALLOC_MAC_MEMORY */
    { "system",
      "specify the system version that executor reports to applications",
      opt_sep, "" },
    { "notnative",
      "don't use native code in syn68k",
      opt_no_arg, "" },

    { "grayscale", "\
specify that executor should run in grayscale mode even if it is \
capable of color.",
      opt_no_arg, "" },
    { "netatalk", "use netatalk naming conventions for AppleDouble files",
      opt_no_arg, "" },
    { "afpd", "use afpd conventions for AppleDouble files (implies -netatalk)",
      opt_no_arg, "" },
    { "rsrc", "use native resource forks on Mac OS X",
      opt_no_arg, "" },

    { "cities", "Don't use Helvetica for Geneva, Courier for Monaco and Times "
                "for New York when generating PostScript",
      opt_no_arg, "" },

#if defined(CYGWIN32)
    { "die", "allow Executor to die instead of catching trap", opt_no_arg,
      "" },
    { "noautoevents", "disable timer driven event checking", opt_no_arg,
      "" },
#endif

#if defined(MSDOS) || defined(CYGWIN32)
    { "nocheckpoint", "disable \"failure.txt\" checkpointing", opt_no_arg,
      "" },
#endif

    { "prvers",
      "specify the printer version that executor reports to applications",
      opt_sep, "" },

    { "prres",
      "specify an additional resolution available for printing, e.g. "
      "\"executor -prres 600x600\" will make 600dpi resolution available "
      "in addition to the standard 72dpi.  Not all apps will be able to use "
      "this additional resolution.",
      opt_sep, "" },

#if defined(SDL)
    { "fullscreen", "try to run in full-screen mode", opt_no_arg, "" },
    { "hwsurface", "UNSUPPORTED", opt_no_arg, "" },
#if defined(CYGWIN)
    {
        "sdlaudio", "specify the audio driver to attempt to use first, e.g. "
                    "\"executor -sdlaudio waveout\" will tell Executor to use the old "
                    "windows sound drivers and not use DirectSound.",
        opt_sep, "",
    },
#else
    {
        "sdlaudio", "specify the audio driver to attempt to use first, e.g. "
                    "\"executor -sdlaudio esd\" will tell Executor to use esound, "
                    "the Enlightened Sound Daemon instead of /dev/dsp.",
        opt_sep, "",
    },
#endif
#endif

#if defined(CYGWIN32) && defined(SDL)
    { "clipleak", "UNSUPPORTED (ignored)", opt_no_arg, "" },
#endif

#if defined(X) || defined(SDL)
    { "scancodes", "different form of key mapping (may be useful in "
                   "conjunction with -keyboard)",
      opt_no_arg, "" },
#endif

    { "desperate", /* Handled specially; here for documentation purposes only. */
      "run in \"desperation mode\".  This will cause Executor "
      "to use as few system features as possible, which is handy for "
      "troubleshooting if Executor is having serious problems with "
      "your system.",
      opt_no_arg, "" },

#if defined(powerpc) || defined(__ppc__)
    { "ppc", "try to execute the PPC native code if possible (UNSUPPORTED)", opt_no_arg, "" },
#endif

#if defined(CYGWIN32)
    { "realmodecd", "try to use real-mode cd-rom driver", opt_no_arg, "" },
#endif

    { "appearance", "(mac or windows) specify the appearance of windows and "
                    "menus.  For example \"executor -appearance windows\" will make each "
                    "window have a blue title bar",
      opt_sep, "" },

    { "hfsplusro", "unsupported -- do not use", opt_no_arg, "" },
};

opt_database_t Executor::common_db;

/* Prints the specified value in a representation appropriate for command
 * line switches.
 */
static void
print_command_line_value(FILE *fp, int v)
{
    if(v > 0 && v % 1024 == 0)
    {
        if(v % (1024 * 1024) == 0)
            fprintf(fp, "%dM", v / (1024 * 1024));
        else
            fprintf(fp, "%dK", v / 1024);
    }
    else
        fprintf(fp, "%d", v);
}

static void
check_arg(string argname, int *arg, int min, int max)
{
    if(*arg < min || *arg > max)
    {
        fprintf(stderr, "%s: invalid value for `%s': must be between ",
                program_name, argname.c_str());
        print_command_line_value(stderr, min);
        fputs(" and ", stderr);
        print_command_line_value(stderr, max);
        fputs(" inclusive.\n", stderr);
        bad_arg_p = true;
    }
}

#if defined(NEED_WAIT4)

/*
 * NOTE: This is a replacement for wait4 that will work for what we use wait4.
 *	 It has the side effect of acknowledging the death of other children.
 */

PUBLIC LONGINT Executor::wait4(LONGINT pid, union wait * statusp, LONGINT options, struct rusage * rusage)
{
    LONGINT retval;

    do
    {
        retval = wait3(statusp, options, rusage);
    } while(retval > 0 && retval != pid);

    return retval;
}
#endif /* NEED_WAIT4 */

PUBLIC char Executor::ROMlib_startdir[MAXPATHLEN];
PUBLIC INTEGER ROMlib_startdirlen;
#if defined(WIN32)
PUBLIC char Executor::ROMlib_start_drive;
#endif
PUBLIC std::string Executor::ROMlib_appname;

#if !defined(LINUX)
#define SHELL "/bin/sh"
#define WHICH "which "
#else
#define SHELL "/bin/bash"
#define WHICH "type -path "
#endif

#define APPWRAP "/Executor.app"

PRIVATE void
set_appname(char *argv0)
{
    char *p = strrchr(argv0, '/');
#if defined(MSDOS) || defined(CYGWIN32)
    if(!p)
        p = strrchr(argv0, '\\');
#endif
    if(p)
        ++p;
    else
        p = argv0;
    ROMlib_appname = p;
}

PRIVATE void setstartdir(char * argv0)
{
#if !defined(WIN32)
    LONGINT p[2], pid;
    char buf[MAXPATHLEN];
    INTEGER nread, arg0len;
    char *lookhere, *suffix, *whichstr;
    char savedir[MAXPATHLEN];

    if(argv0[0] == '/' || Uaccess(argv0, X_OK) == 0)
        lookhere = argv0;
    else
    {
        pipe(p);
        if((pid = fork()) == 0)
        {
            close(1);
            dup(p[1]);
            arg0len = strlen(argv0);
            whichstr = (char *)alloca(arg0len + sizeof(WHICH));
            memmove(whichstr, WHICH, sizeof(WHICH) - 1);
            memmove(whichstr + sizeof(WHICH) - 1, argv0, arg0len + 1);
            execl(SHELL, "sh", "-c", whichstr, (char *)0);
            fprintf(stderr, "``%s -c \"%s\"'' failed\n", SHELL, whichstr);
            fflush(stderr);
            _exit(127);
/* NOTREACHED */
#if !defined(LETGCCWAIL)
            lookhere = 0;
#endif /* LETGCCWAIL */
        }
        else
        {
            close(p[1]);
            nread = read(p[0], buf, sizeof(buf) - 1);
            wait4(pid, 0, 0, (struct rusage *)0);
            if(nread)
                --nread; /* get rid of trailing \n */
            buf[nread] = 0;
            lookhere = buf;
        }
    }
    suffix = rindex(lookhere, '/');
    if(suffix)
        *suffix = 0;
    getcwd(savedir, sizeof savedir);
    Uchdir(lookhere);
    if(suffix)
        *suffix = '/';
    getcwd(ROMlib_startdir, sizeof ROMlib_startdir);
    Uchdir(savedir);
    ROMlib_startdirlen = strlen(ROMlib_startdir) + 1;
#else /* defined(MSDOS) || defined(CYGWIN32) */

    if(argv0[1] == ':')
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
#if defined(CYGWIN32)
        if(!lastslash)
            lastslash = strrchr(ROMlib_startdir, '\\');
#endif
        if(lastslash)
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

PUBLIC BOOLEAN Executor::ROMlib_startupscreen = true;

char *Executor::program_name;

static syn68k_addr_t
unhandled_trap(syn68k_addr_t callback_address, void *arg)
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

    trap_num = (intptr_t)arg;

    switch(trap_num)
    {
        case 4: /* Illegal instruction. */
        case 8: /* Privilege violation. */
        case 10: /* A-line. */
        case 11: /* F-line. */
        case 14: /* Format error. */
        case 15: /* Uninitialized interrupt. */
        case 24: /* Spurious interrupt. */
        case 25: /* Level 1 interrupt autovector. */
        case 26: /* Level 2 interrupt autovector. */
        case 27: /* Level 3 interrupt autovector. */
        case 28: /* Level 4 interrupt autovector. */
        case 29: /* Level 5 interrupt autovector. */
        case 30: /* Level 6 interrupt autovector. */
        case 31: /* Level 7 interrupt autovector. */
        case 32: /* TRAP #0 vector. */
        case 33: /* TRAP #1 vector. */
        case 34: /* TRAP #2 vector. */
        case 35: /* TRAP #3 vector. */
        case 36: /* TRAP #4 vector. */
        case 37: /* TRAP #5 vector. */
        case 38: /* TRAP #6 vector. */
        case 39: /* TRAP #7 vector. */
        case 40: /* TRAP #8 vector. */
        case 41: /* TRAP #9 vector. */
        case 42: /* TRAP #10 vector. */
        case 43: /* TRAP #11 vector. */
        case 44: /* TRAP #12 vector. */
        case 45: /* TRAP #13 vector. */
        case 46: /* TRAP #14 vector. */
        case 47: /* TRAP #15 vector. */
        case 3: /* Address error. */
        case 6: /* CHK/CHK2 instruction. */
        case 7: /* FTRAPcc, TRAPcc, TRAPV instructions. */
        case 9: /* Trace. */
        case 5: /* Integer divide by zero. */
            sprintf(pc_str, "0x%lX", (unsigned long)READUL(EM_A7 + 2));
            break;
        default:
            strcpy(pc_str, "<unknown>");
            break;
    }

    if(trap_num < (int)NELEM(trap_description)
       && trap_description[trap_num] != NULL)
        gui_fatal("Fatal error:  unhandled trap %d at pc %s (%s)",
                  trap_num, pc_str, trap_description[trap_num]);
    else
        gui_fatal("Fatal error:  unhandled trap %d at pc %s", trap_num, pc_str);

    /* It will never get here; this is just here to quiet gcc. */
    return callback_address;
}

static void
setup_trap_vectors(void)
{
    syn68k_addr_t timer_callback;

    /* Set up the trap vector for the timer interrupt. */
    timer_callback = callback_install(catchalarm, NULL);
    *(GUEST<syn68k_addr_t> *)SYN68K_TO_US(M68K_TIMER_VECTOR * 4) = CL(timer_callback);

    /* Fill in unhandled trap vectors so they cause graceful deaths.
   * Skip over those trap vectors which are known to have legitimate
   * values.
   */
    for(intptr_t i = 1; i < 64; i++)
        if(i != 10 /* A line trap.       */
#if defined(M68K_TIMER_VECTOR)
           && i != M68K_TIMER_VECTOR /* timer interrupt.   */
#endif

#if defined(M68K_WATCHDOG_VECTOR)
           && i != M68K_WATCHDOG_VECTOR /* watchdog timer interrupt. */
#endif

#if defined(M68K_EVENT_VECTOR)
           && i != M68K_EVENT_VECTOR
#endif

#if defined(M68K_MOUSE_MOVED_VECTOR)
           && i != M68K_MOUSE_MOVED_VECTOR
#endif

#if defined(M68K_SOUND_VECTOR)
           && i != M68K_SOUND_VECTOR
#endif

           && i != 0x58 / 4) /* Magic ARDI vector. */
        {
            syn68k_addr_t c;
            c = callback_install(unhandled_trap, (void *)i);
            *(GUEST<syn68k_addr_t> *)SYN68K_TO_US(i * 4) = CL(c);
        }
}

static void illegal_mode(void) __attribute__((noreturn));

static void
illegal_mode(void)
{
    const vdriver_modes_t *vm;
    int num_sizes, max_width, max_height, max_bpp;

    vm = vdriver_mode_list;
    num_sizes = vm->num_sizes;
    max_bpp = vdriver_max_bpp;
    if(num_sizes)
    {
        max_width = vm->size[num_sizes - 1].width;
        max_height = vm->size[num_sizes - 1].height;
    }
    else
        max_width = max_height = 0; /* quiet gcc */

    vdriver_shutdown(); /* Just to be safe. */
    if(num_sizes == 0)
        fprintf(stderr, "Unable to find legal graphics mode.\n");
    else
    {
        fprintf(stderr, "Invalid graphics mode.  Largest allowable "
                        "screen %dx%d, %d bits per pixel.\n",
                max_width, max_height, max_bpp);
    }

    /* Don't call ExitToShell here; ROMlib isn't set up enough yet to do that. */
    exit(-1);
}

#if defined(MSDOS) || defined(CYGWIN32)
PUBLIC uint32_t ROMlib_macdrives; /* default computed at runtime */
PUBLIC uint32_t ROMlib_dosdrives = ~0;
PRIVATE uint32_t skipdrives = 0;

PRIVATE bool
drive_number_from_letter(char c, int *nump)
{
    bool retval;

    if(isupper(c) || ((c >= '[' && c <= '`')))
    {
        *nump = c - 'A';
        retval = true;
    }
    else if(islower(c))
    {
        *nump = c - 'a';
        retval = true;
    }
    else
        retval = false;
    return retval;
}

PRIVATE void
drive_error(const char *opt)
{
    fprintf(stderr, "Invalid drive specification.\nUse something like "
                    "\"-%s A-EJ\" (for drives A, B, C, D, E and J).\n",
            opt);
    bad_arg_p = true;
}

PRIVATE const char *
munch_next_char(const char *p, uint32_t *destp, const char *opt)
{
    const char *retval;
    int d;

    if(drive_number_from_letter(p[0], &d))
    {
        if(p[1] == '-')
        {
            int d2;
            if(drive_number_from_letter(p[2], &d2))
            {
                int lower, upper, i;

                lower = MIN(d, d2);
                upper = MAX(d, d2);
                for(i = lower; i <= upper; ++i)
                    *destp |= (1 << i);
                retval = p + 3;
            }
            else
            {
                drive_error(opt);
                retval = "";
            }
        }
        else
        {
            *destp |= 1 << d;
            retval = p + 1;
        }
    }
    else
    {
        drive_error(opt);
        retval = "";
    }
    return retval;
}

PUBLIC uint32_t
parse_drive_opt(const char *opt_name, const char *opt_value)
{
    uint32_t retval;
    const char *next_charp;

    retval = 0;
    for(next_charp = opt_value; *next_charp;)
        next_charp = munch_next_char(next_charp, &retval, opt_name);
    return retval;
}
#endif

static void
print_info(void)
{
    printf("This is %s, compiled.\n",
           ROMlib_executor_full_name);

#define MB (1024 * 1024U)

    /* Print out actual memory size chosen. */
    printf("Choosing %u.%02u MB for applzone, %u.%02u MB for syszone, "
           "%u.%02u MB for stack\n",
           ROMlib_applzone_size / MB,
           (ROMlib_applzone_size % MB) * 100 / MB,
           ROMlib_syszone_size / MB,
           (ROMlib_syszone_size % MB) * 100 / MB,
           ROMlib_stack_size / MB,
           (ROMlib_stack_size % MB) * 100 / MB);

#undef MB
}

/* This is to tell people about the switch from "-applzone 4096" to
 * "-applzone 4M".
 */
static void
note_memory_syntax_change(const char *arg, unsigned val)
{
    /* Make sane error messages when the supplied value is insane.
   * Otherwise, try to print an example that illustrates how to
   * use the value they are trying to generate.
   */
    if(val < 100 || val > 1000000)
        val = 2048;

    fprintf(stderr, "Specified %s is too small.  The syntax "
                    "has changed; now, for a %u.%02u\nmegabyte %s you would "
                    "say \"-%s ",
            arg, val / 1024, (val % 1024) * 100 / 1024,
            arg, arg);

    if(val % 1024 == 0)
        fprintf(stderr, "%uM", val / 1024);
    else if(val < 1024)
        fprintf(stderr, "%uK", val);
    else
        fprintf(stderr, "%u.%02uM", val / 1024, (val % 1024) * 100 / 1024);

    fputs("\"\n", stderr);

    bad_arg_p = true;
}

/* Concatenates all strings in the array, separated by spaces. */
static const char *
construct_command_line_string(int argc, char **argv)
{
    char *s;
    unsigned long string_length;
    int i, j;

    for(i = 0, string_length = 0; i < argc; i++)
        string_length += strlen(argv[i]) + 1;
    s = (char *)malloc(string_length + 1);
    s[0] = '\0';
    for(j = 0; j < argc; j++)
        sprintf(s + strlen(s), "%s%s", j ? " " : "", argv[j]);
    return s;
}

PRIVATE int
zap_comments(char *buf, int n_left)
{
    char *ip, *op;
    bool last_was_cr;
    int retval;

    ip = buf;
    op = buf;
    retval = 0;
    last_was_cr = true;
    while(n_left > 0)
    {
        while(n_left > 0 && (*ip != '#' || !last_was_cr))
        {
            last_was_cr = *ip == '\n' || *ip == '\r';
            *op++ = *ip++;
            ++retval;
            --n_left;
        }
        if(n_left > 0)
        {
            while(n_left > 0 && *ip != '\n' && *ip != '\r')
            {
                ++ip;
                --n_left;
            }
        }
    }
    return retval;
}

PUBLIC FILE *
Executor::executor_dir_fopen(const char *file, const char *perm)
{
    return fopen(expandPath(std::string("+/") + file).c_str(), perm);
}

PUBLIC int
Executor::executor_dir_remove(const char *file)
{
    return remove(expandPath(std::string("+/") + file).c_str());
}

PRIVATE void
read_args_from_file(const char *filename, int *argcp, char ***argvpp)
{
    FILE *fp;

    fp = executor_dir_fopen(filename, "r");
    if(fp)
    {
        int nread, n_extra_params, i;
        char buf[8192], *bufp;
        char **saveargv;

        nread = fread(buf, 1, sizeof buf, fp);
        fclose(fp);
        nread = zap_comments(buf, nread);
        n_extra_params = count_params(buf, nread);
        *argcp += n_extra_params;
        saveargv = *argvpp;
        *argvpp = (char **)malloc((*argcp + 1) * sizeof *argvpp);
        bufp = buf;
        memcpy(*argvpp, saveargv, (*argcp - n_extra_params) * sizeof **argvpp);
        for(i = *argcp - n_extra_params; i < *argcp; ++i)
            (*argvpp)[i] = get_param((const char **)&bufp, &nread);
        (*argvpp)[i] = 0;
    }
}

#if defined(CYGWIN32)
PRIVATE uint32_t
win_drive_to_bit(const char *drive_namep)
{
    uint32_t retval;

    if(drive_namep[1] == ':')
        retval = 1 << (tolower(drive_namep[0]) - 'a');
    else
    {
        warning_unexpected("drive name = '%s'", drive_namep);
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

#if defined(LINUX)
extern char _etext, _end; /* boundaries of data+bss sections, supplied by the linker */
#endif

int main(int argc, char **argv)
{
    char thingOnStack; /* used to determine an approximation of the stack base address */
    check_structs();

    INTEGER i;
    static GUEST<uint16_t> jmpl_to_ResourceStub[3] = {
        CWC((unsigned short)0x4EF9), CWC(0), CWC(0) /* Filled in below. */
    };
    uint32_t l;
    ULONGINT save_trap_vectors[64];
    virtual_int_state_t int_state;
    GUEST<THz> saveSysZone, saveApplZone;
    GUEST<Ptr> saveApplLimit;
    static void (*reg_funcs[])(void) = {
        vdriver_opt_register,
        NULL,
    };
    string arg;

#if defined(PERSONALITY_HACK)
    int pers;

    pers = personality(0xffffffff);
    if((pers & MMAP_PAGE_ZERO) == 0)
    {
        if(personality(pers | MMAP_PAGE_ZERO | READ_IMPLIES_EXEC) == 0)
            execv(argv[0], argv);
    }
#endif

    int grayscale_p = false;

    ROMlib_command_line = construct_command_line_string(argc, argv);

    if(!arch_init())
    {
        fprintf(stderr, "Unable to initialize CPU information.\n");
        exit(-100);
    }

    if(!os_init())
    {
        fprintf(stderr, "Unable to initialize operating system features.\n");
        exit(-101);
    }

    {
        char *t;

        t = strrchr(*argv, '/');
        if(t)
            program_name = &t[1];
        else
            program_name = *argv;
    }

    /* Guarantee various time variables are set up properly. */
    msecs_elapsed();

    ROMlib_WriteWhen(WriteInOSEvent);

    setstartdir(argv[0]);
    set_appname(argv[0]);

#define COMMANDS "commands.txt"

    read_args_from_file(COMMANDS, &argc, &argv);
#if defined(MSDOS) || defined(CYGWIN32)
    read_args_from_file(CHECKPOINT_FILE, &argc, &argv);
    checkpointp = checkpoint_init();
#endif

    /* Replace "-desperate" switch with what it implies.  We must do
   * this before normal command line processing.
   */
    if(!handle_desperate_switch(&argc, &argv))
        exit(-1);

    opt_init();
    common_db = opt_alloc_db();
    opt_register("common", common_opts);

    opt_register_pre_note("welcome to the executor help message.");
    opt_register_pre_note("usage: `executor [option...] "
                          "[program [document1 document2 ...]]'");

    for(i = 0; reg_funcs[i]; i++)
        (*reg_funcs[i])();

    if(!bad_arg_p)
        bad_arg_p = opt_parse(common_db, common_opts,
                              &argc, argv);

    if(opt_val(common_db, "version", NULL))
    {
        fprintf(stdout, "%s\n", EXECUTOR_VERSION);
        exit(0);
    }

    /*
   * If they ask for help, it's not an error -- it should go to stdout
   */

    if(opt_val(common_db, "help", NULL))
    {
        fprintf(stdout, "%s", opt_help_message());
        exit(0);
    }

    /* Verify that the user input a legal bits per pixel.  "0" is a legal
   * value here; that means "use the vdriver's default."
   */
    opt_int_val(common_db, "bpp", &flag_bpp, &bad_arg_p);
    if(flag_bpp != 0 && flag_bpp != 1
       && flag_bpp != 2 && flag_bpp != 4 && flag_bpp != 8
       && flag_bpp != 16 && flag_bpp != 32)
    {
        fprintf(stderr, "Bits per pixel must be 1, 2, 4, 8, 16 or 32.\n");
        bad_arg_p = true;
    }

#if defined(SDL)
    if(opt_val(common_db, "fullscreen", NULL))
        ROMlib_fullscreen_p = true;

    if(opt_val(common_db, "hwsurface", NULL))
        ROMlib_hwsurface_p = true;
#endif

#if defined(X) || (defined(CYGWIN32) && defined(SDL))
    if(opt_val(common_db, "scancodes", NULL))
        ROMlib_set_use_scancodes(true);
#endif

#if defined(Sound_SDL_Sound)

    if(opt_val(common_db, "sdlaudio", &arg))
        ROMlib_set_sdl_audio_driver_name(arg);

#endif

    if(opt_val(common_db, "ppc", NULL))
        ROMlib_set_ppc(true);

    if(opt_val(common_db, "hfsplusro", NULL))
        ROMlib_hfs_plus_support = true;

#if defined(CYGWIN32)
    if(opt_val(common_db, "realmodecd", NULL))
        ROMlib_set_realmodecd(true);
#endif

    if(opt_val(common_db, "size", &arg))
        bad_arg_p |= !parse_size_opt("size", arg);

    if(opt_val(common_db, "prres", &arg))
        bad_arg_p |= !parse_prres_opt(&ROMlib_optional_res_x,
                                      &ROMlib_optional_res_y, arg);

    if(opt_val(common_db, "debug", &arg))
        bad_arg_p |= !error_parse_option_string(arg);

#if defined(MSDOS) || defined(CYGWIN32)
    if(opt_val(common_db, "macdrives", &arg))
        ROMlib_macdrives = parse_drive_opt("macdrives", arg);
    else
    {
#if !defined(CYGWIN32)
        ROMlib_macdrives = 3; /* A: + B: */
        {
            int cdrom;

            cdrom = dosdisk_find_cdrom();
            if(cdrom != -1)
                ROMlib_macdrives |= (1 << cdrom);
        }
#else
        char buf[512];

        if(win_GetLogicalDriveStrings(sizeof buf - 1, buf))
        {
            char *p;

            for(p = buf; *p; p += strlen(p) + 1)
                if(win_direct_accessible_disk(p))
                    ROMlib_macdrives |= win_drive_to_bit(p);
        }
#endif
    }

    if(opt_val(common_db, "dosdrives", &arg))
        ROMlib_dosdrives = parse_drive_opt("dosdrives", arg);

    if(opt_val(common_db, "skipdrives", &arg))
    {
        skipdrives = parse_drive_opt("skipdrives", arg);
        ROMlib_macdrives &= ~skipdrives;
        ROMlib_dosdrives &= ~skipdrives;
    }

#endif

#if defined(MACOSX)
    // sync() really takes a long time on Mac OS X.
    ROMlib_nosync = true;
#endif

    {
        int skip;
        skip = 0;
        opt_int_val(common_db, "nosound", &skip, &bad_arg_p);
        sound_disabled_p = (skip != 0);
    }

    {
        int nocache;
        nocache = 0;
        opt_int_val(common_db, "nodiskcache", &nocache, &bad_arg_p);
        dcache_set_enabled(!nocache);
    }

    use_native_code_p = !opt_val(common_db, "notnative", NULL);

    if(opt_val(common_db, "netatalk", NULL))
        setup_resfork_format(ResForkFormat::netatalk);
    else if(opt_val(common_db, "afpd", NULL))
        setup_resfork_format(ResForkFormat::afpd);
    else if(opt_val(common_db, "rsrc", NULL))
        setup_resfork_format(ResForkFormat::native);
    else
        setup_resfork_format(ResForkFormat::standard);
    
    substitute_fonts_p = !opt_val(common_db, "cities", NULL);

#if defined(CYGWIN32)
    if(opt_val(common_db, "die", NULL))
        uninstall_exception_handler();
    if(opt_val(common_db, "noautoevents", NULL))
        set_timer_driven_events(false);
#endif

#if defined(MSDOS) || defined(CYGWIN32)
    if(opt_val(common_db, "nocheckpoint", NULL))
        disable_checkpointing();
#endif

    /* Parse the "-memory" option. */
    {
        int total_memory;
        if(opt_int_val(common_db, "memory", &total_memory, &bad_arg_p))
        {
            check_arg("memory", &total_memory,
                      (MIN_APPLZONE_SIZE + DEFAULT_SYSZONE_SIZE
                       + DEFAULT_STACK_SIZE),
                      (MAX_APPLZONE_SIZE + DEFAULT_SYSZONE_SIZE
                       + DEFAULT_STACK_SIZE));

            /* Set up the three memory sizes appropriately.  For now we
       * just allocate the defaults for syszone and stack, and
       * put everything else in -applzone.
       */
            ROMlib_syszone_size = DEFAULT_SYSZONE_SIZE;
            ROMlib_stack_size = DEFAULT_STACK_SIZE;
            ROMlib_applzone_size = (total_memory - ROMlib_syszone_size
                                    - ROMlib_stack_size);
        }
    }

    /* I bumped the minimal ROMlib_applzone to 512, since Loser needs
   more than 256.  I guess it's a little unfair to people who bypass
   Loser, but it will prevent confusion.  */
    opt_int_val(common_db, "applzone", &ROMlib_applzone_size, &bad_arg_p);
    if(ROMlib_applzone_size < 65536)
        note_memory_syntax_change("applzone", ROMlib_applzone_size);
    else
        check_arg("applzone", &ROMlib_applzone_size, MIN_APPLZONE_SIZE,
                  MAX_APPLZONE_SIZE);

    opt_int_val(common_db, "syszone", &ROMlib_syszone_size, &bad_arg_p);
    if(ROMlib_syszone_size < 65536)
        note_memory_syntax_change("syszone", ROMlib_syszone_size);
    else
        check_arg("syszone", &ROMlib_syszone_size, MIN_SYSZONE_SIZE,
                  MAX_SYSZONE_SIZE);

    opt_int_val(common_db, "stack", &ROMlib_stack_size, &bad_arg_p);
    if(ROMlib_stack_size < 32768)
        note_memory_syntax_change("stack", ROMlib_stack_size);
    else
        check_arg("stack", &ROMlib_stack_size, MIN_STACK_SIZE, MAX_STACK_SIZE);

#if defined(MM_MANY_APPLZONES)
    opt_int_val(common_db, "napplzones", &mm_n_applzones, &bad_arg_p);
    check_arg("napplzones", &mm_n_applzones, 1, /* random */ 255);
#endif /* !MM_MANY_APPLZONES */

    ROMlib_InitZones();

#if SIZEOF_CHAR_P > 4
    /*
    On 64-bit platforms, there is no single ROMlib_offset, but rather
    a four-element array. The high two bits of the 69K address are mapped
    to an index in this array.
    This way, we can access:
        0 - the regular emulated memory
        1 - video memory.
        2 - local variables of executor's main thread
        3 - executor's global variables (which includes syn68K callback addresses)

    Block 0 is set up in ROMlib_InitZones.
    Block 1 is set up later, when video memory is allocated.
    Global variables are in block 3 so that we don't need to figure out
    the exact boundaries for that address range.
   */

    // mark the slot as occupied until we explicitly set it later
    ROMlib_offsets[1] = 0xFFFFFFFFFFFFFFFF - (1UL << 30);
    ROMlib_sizes[1] = 0;

    // assume an arbitrary maximum stack size of 16MB.
    ROMlib_offsets[2] = (uintptr_t)&thingOnStack - 16 * 1024 * 1024;
    ROMlib_offsets[2] -= ROMlib_offsets[2] & 3;
    ROMlib_offsets[2] -= (2UL << 30);
    ROMlib_sizes[2] = 16 * 1024 * 1024;

#if defined(LINUX)
    ROMlib_offsets[3] = (uintptr_t)&_etext;
    ROMlib_offsets[3] -= ROMlib_offsets[3] & 3;
    ROMlib_offsets[3] -= (3UL << 30);
    ROMlib_sizes[3] = &_end - &_etext;
#else
    /* Mac OS X doesn't have _etext and _end, and the functions in
       mach/getsect.h don't give the correct results when ASLR is active.
       Win32 might also have a way to get the addresses, or it might not.

       So we just use the address of a static variable and 512MB in each direction.
     */
    static char staticThing[32];
    ROMlib_offsets[3] = (uintptr_t)&staticThing - 0x20000000;
    ROMlib_offsets[3] -= ROMlib_offsets[2] & 3;
    ROMlib_offsets[3] -= (3UL << 30);
    ROMlib_sizes[3] = 0x3FFFFFFF;
#endif
#endif

    {
        uint32_t save_a7;

        save_a7 = EM_A7;
        /* Set up syn68k. */
        initialize_68k_emulator(vdriver_system_busy,
                                use_native_code_p,
                                (uint32_t *)SYN68K_TO_US(0),
                                0
                                );

        EM_A7 = save_a7;
    }

    if(opt_val(common_db, "keyboards", NULL))
        graphics_p = false;

    /* Block virtual interrupts, until the system is fully set up. */
    int_state = block_virtual_ints();

    if(graphics_p && !vdriver_init(0, 0, 0, false, &argc, argv))
    {
        fprintf(stderr, "Unable to initialize video driver.\n");
        exit(-12);
    }

    /* Save the trap vectors away. */
    memcpy(save_trap_vectors, SYN68K_TO_US(0), sizeof save_trap_vectors);

    opt_int_val(common_db, "sticky", &ROMlib_sticky_menus_p, &bad_arg_p);

    opt_int_val(common_db, "pceditkeys", &ROMlib_forward_del_p, &bad_arg_p);

    opt_int_val(common_db, "nobrowser", &ROMlib_nobrowser, &bad_arg_p);

    opt_int_val(common_db, "print", &ROMlib_print, &bad_arg_p);

#if defined(MACOSX_) || defined(LINUX)
    opt_int_val(common_db, "nodotfiles", &ROMlib_no_dot_files, &bad_arg_p);
#endif

#if defined(NOMOUSE_COMMAND_LINE_OPTION)
    opt_int_val(common_db, "nomouse", &ROMlib_no_mouse, &bad_arg_p);
#endif

#if 0
  opt_int_val (common_db, "noclock",     &ROMlib_noclock,   &bad_arg_p);
#endif

    {
        int no_auto = false;
        opt_int_val(common_db, "noautorefresh", &no_auto, &bad_arg_p);
        do_autorefresh_p = !no_auto;
    }

    opt_int_val(common_db, "refresh", &ROMlib_refresh, &bad_arg_p);
    check_arg("refresh", &ROMlib_refresh, 0, 60);

    opt_int_val(common_db, "grayscale", &grayscale_p, &bad_arg_p);

#if defined(LINUX)
    opt_int_val(common_db, "nodrivesearch", &nodrivesearch_p, &bad_arg_p);
#endif

    {
        string str;

        if(opt_val(common_db, "prvers", &str))
        {
            uint32_t vers;

            if(!ROMlib_parse_version(str, &vers))
                bad_arg_p = true;
            else
                ROMlib_PrDrvrVers = (vers >> 8) * 10 + ((vers >> 4) & 0xF);
        }
    }

#if defined(SUPPORT_LOG_ERR_TO_RAM)
    {
        int log;
        log = 0;
        opt_int_val(common_db, "ramlog", &log, &bad_arg_p);
        log_err_to_ram_p = (log != 0);
    }
#endif

    if(opt_val(common_db, "info", NULL))
    {
        print_info();
        exit(0);
    }

    /* If we failed to parse our arguments properly, exit now.
   * I don't think we should call ExitToShell yet because the
   * rest of the system isn't initialized.
   */
    if(argc >= 2)
    {
        int a;

        /* Only complain if we see something with a leading dash; anything
	 * else might be a file to launch.
	 */
        for(a = 1; a < argc; a++)
        {
            if(argv[a][0] == '-')
            {
                fprintf(stderr, "%s: unknown option `%s'\n",
                        program_name, argv[a]);
                bad_arg_p = true;
            }
        }
    }

    filltables();

    l = ostraptable[0x0FC];
    ((unsigned char *)jmpl_to_ResourceStub)[2] = l >> 24;
    ((unsigned char *)jmpl_to_ResourceStub)[3] = l >> 16;
    ((unsigned char *)jmpl_to_ResourceStub)[4] = l >> 8;
    ((unsigned char *)jmpl_to_ResourceStub)[5] = l;
    ostraptable[0xFC] = US_TO_SYN68K(jmpl_to_ResourceStub);
    osstuff[0xFC].orig = US_TO_SYN68K(jmpl_to_ResourceStub);

    saveSysZone = SysZone;
    saveApplZone = ApplZone;
    saveApplLimit = ApplLimit;
    memset(&nilhandle, ~0, (char *)&lastlowglobal - (char *)&nilhandle);

    Ticks = 0;
    nilhandle = 0; /* so nil dereferences "work" */

    memset(&EventQueue, 0, sizeof(EventQueue));
    memset(&VBLQueue, 0, sizeof(VBLQueue));
    memset(&DrvQHdr, 0, sizeof(DrvQHdr));
    memset(&VCBQHdr, 0, sizeof(VCBQHdr));
    memset(&FSQHdr, 0, sizeof(FSQHdr));
    TESysJust = 0;
    SysZone = saveSysZone;
    ApplZone = saveApplZone;
    ApplLimit = saveApplLimit;
    BootDrive = 0;
    DefVCBPtr = 0;
    CurMap = 0;
    TopMapHndl = 0;
    DSAlertTab = 0;
    ResumeProc = 0;
    SFSaveDisk = 0;
    GZRootHnd = 0;
    ANumber = 0;
    ResErrProc = 0;
    FractEnable = 0;
    SEvtEnb = 0;
    MenuList = 0;
    MBarEnable = 0;
    MenuFlash = 0;
    TheMenu = 0;
    MBarHook = 0;
    MenuHook = 0;
    MenuCInfo = NULL;
    HeapEnd = 0;
    ApplLimit = 0;
    SoundActive = soundactiveoff;
    PortBUse = 2; /* configured for Serial driver */
    memset(KeyMap, 0, sizeof_KeyMap);
    if(vdriver_grayscale_p || grayscale_p)
    {
        /* Choose a nice light gray hilite color. */
        HiliteRGB.red = CWC((unsigned short)0xAAAA);
        HiliteRGB.green = CWC((unsigned short)0xAAAA);
        HiliteRGB.blue = CWC((unsigned short)0xAAAA);
    }
    else
    {
        /* how about a nice yellow hilite color? no, it's ugly. */
        HiliteRGB.red = CWC((unsigned short)0xAAAA);
        HiliteRGB.green = CWC((unsigned short)0xAAAA);
        HiliteRGB.blue = CWC((unsigned short)0xFFFF);
    }

    {
        static GUEST<uint16_t> ret = CWC((unsigned short)0x4E75);

        JCrsrTask = RM((ProcPtr)&ret);
    }

    SET_HILITE_BIT();
    TheGDevice = MainDevice = DeviceList = CLC_NULL;

    OneOne = CLC(0x00010001);
    Lo3Bytes = CLC(0xFFFFFF);
    DragHook = 0;
    TopMapHndl = 0;
    SysMapHndl = 0;
    MBDFHndl = 0;
    MenuList = 0;
    MBSaveLoc = 0;

    SysVersion = CW(system_version);
    FSFCBLen = CWC(94);
    ScrapState = CWC(-1);

    TheZone = SysZone;
    UTableBase = RM((DCtlHandlePtr)NewPtr(4 * NDEVICES));
    memset(MR(UTableBase), 0, 4 * NDEVICES);
    UnitNtryCnt = CW(NDEVICES);
    TheZone = ApplZone;

    if(graphics_p)
    {
        /* Set up the current graphics mode appropriately. */
        if(!vdriver_set_mode(flag_width, flag_height, flag_bpp, grayscale_p))
            illegal_mode();

#if SIZEOF_CHAR_P > 4
        if(vdriver_fbuf == 0)
            abort();
        ROMlib_offsets[1] = (uintptr_t)vdriver_fbuf;
        ROMlib_offsets[1] -= (1UL << 30);
        ROMlib_sizes[1] = vdriver_row_bytes * vdriver_height;
#endif

        /* initialize the mac rgb_spec's */
        make_rgb_spec(&mac_16bpp_rgb_spec,
                      16, true, 0,
                      5, 10, 5, 5, 5, 0,
                      CL_RAW(GetCTSeed()));

        make_rgb_spec(&mac_32bpp_rgb_spec,
                      32, true, 0,
                      8, 16, 8, 8, 8, 0,
                      CL_RAW(GetCTSeed()));

        gd_allocate_main_device();
    }

    ROMlib_eventinit(graphics_p);
    hle_init();

    ROMlib_fileinit();

    InitUtil();

#if !defined(X) && !defined(SDL)
/* #warning "Hack so we don't smash mouse/keyboard m68k interrupt vectors." */
#if defined(M68K_EVENT_VECTOR)
    save_trap_vectors[M68K_EVENT_VECTOR]
        = *(syn68k_addr_t *)SYN68K_TO_US(M68K_EVENT_VECTOR * 4);
#endif
#if defined(M68K_MOUSE_MOVED_VECTOR)
    save_trap_vectors[M68K_MOUSE_MOVED_VECTOR]
        = *(syn68k_addr_t *)SYN68K_TO_US(M68K_MOUSE_MOVED_VECTOR * 4);
#endif
#endif

    {
        string appearance_str;

        if(opt_val(common_db, "appearance", &appearance_str))
            bad_arg_p |= !ROMlib_parse_appearance(appearance_str.c_str());
    }

    InitResources();

    /* parse the `-system' option */
    {
        string system_str;

        if(opt_val(common_db, "system", &system_str))
            bad_arg_p |= !parse_system_version(system_str);
        else
            ROMlib_set_system_version(system_version);
    }

    if(bad_arg_p)
    {
        fprintf(stderr,
                "Type \"%s -help\" for a list of command-line options.\n",
                program_name);
        exit(-10);
    }

    {
        bool keyboard_set_failed;

        if(opt_val(common_db, "keyboard", &arg))
        {
            keyboard_set_failed = !ROMlib_set_keyboard(arg.c_str());
            if(keyboard_set_failed)
                printf("``%s'' is not an available keyboard\n", arg.c_str());
        }
        else
            keyboard_set_failed = false;

        if(keyboard_set_failed || opt_val(common_db, "keyboards", NULL))
            display_keyboard_choices();
    }

    ROMlib_seginit(argc, argv);

    InitFonts();

#if !defined(NDEBUG)
    dump_init(NULL);
#endif

    /* see qColorutil.c */
    ROMlib_color_init();

    wind_color_init();
    /* must be called after `ROMlib_color_init ()' */
    image_inits();

    /* must be after `image_inits ()' */
    sb_ctl_init();

    AE_init();

    {
        INTEGER env = 0;
        ROMlib_Fsetenv(&env, 0);
    }

    TEDoText = RM((ProcPtr)P_ROMlib_dotext); /* where should this go ? */

    {
        LONGINT save58;

        save58 = *(LONGINT *)SYN68K_TO_US(0x58);
        /* Replace the trap vectors which got smashed during initialization. */
        memcpy(SYN68K_TO_US(0), save_trap_vectors, sizeof save_trap_vectors);
        *(LONGINT *)SYN68K_TO_US(0x58) = save58;

        setup_trap_vectors();
    }

    /* Set up timer interrupts.  We need to do this after everything else
   * has been initialized.
   */
    if(!syncint_init())
    {
        vdriver_shutdown();
        fputs("Fatal error:  unable to initialize timer.\n", stderr);
        exit(-11);
    }

    sound_init();

    set_refresh_rate(ROMlib_refresh);

    restore_virtual_ints(int_state);

    WWExist = QDExist = EXIST_NO;

#if defined(CYGWIN32)
    complain_if_no_ghostscript();
#endif

#ifdef MACOSX_
    NeXTMain();
#endif

    executor_main();

    if(!ROMlib_no_windows)
        ExitToShell();
    else
        exit(0);
    /* NOT REACHED */
    return 0;
}
