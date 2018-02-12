namespace Executor
{
/* false if we are blitting straight to screen memory, true if we are
   blitting to a shadow screen */
extern INTEGER ROMlib_shadow_screen_p;

/* true if the cursor needs to be saved when calling `ROMlib_blt_rgn ()',
   otherwise the cursor is saved when the shadow buffer is
   flushed */
extern INTEGER ROMlib_bltrgn_save_cursor_p;

/* do not check for diskette when under dos */
extern int ROMlib_drive_check;

/* 0 means "use default". */
extern INTEGER flag_width, flag_height;

/* 0 means "use default". */
extern int flag_bpp;

/* *argv; name executor was invoked with */
extern char *program_name;

/* Approximate command line; argv[] elements separated by spaces. */
extern const char *ROMlib_command_line;

/* 0 means try running browser, 1 means don't */
extern int ROMlib_nobrowser;

/* 0 means normal, 1 means special gestalt values for photoshop */
extern int ROMlib_photoshop_hack;

/* 1 means map '\n' to '\r' in newline mode */
extern int ROMlib_newlinetocr;

/* true if there is a version skew between the system file version and
   the required system version.  set by `InitResources ()', and used
   by `InitWindows ()' */
extern bool system_file_version_skew_p;
}
