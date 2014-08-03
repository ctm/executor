#if !defined(__RSYS_PREFS__)
#define __RSYS_PREFS__

#ifdef __cplusplus
namespace Executor {
#endif

typedef enum {
  WriteAlways,
  WriteInBltrgn,
  WriteInOSEvent,
  WriteAtEndOfTrap,
  WriteNever
} WriteWhenType;	/* This is an extension */

#ifdef __cplusplus
extern WriteWhenType ROMlib_when;
extern int ROMlib_PretendSound;
extern int ROMlib_cacheheuristic;
extern int ROMlib_clock;
extern int ROMlib_directdiskaccess;
extern int ROMlib_flushoften;
extern int ROMlib_fontsubstitution;
extern int ROMlib_newlinetocr;
extern int ROMlib_nowarn32;
extern int ROMlib_passpostscript;
extern int ROMlib_refresh;
extern int ROMlib_delay;
extern int ROMlib_noclock;

extern int ROMlib_pretend_help;
extern int ROMlib_pretend_alias;
extern int ROMlib_pretend_script;
extern int ROMlib_pretend_edition;


extern uint32 system_version;
#endif

#define ROMLIB_DEBUG_BIT                (1 <<  1)

#ifdef __cplusplus
extern void ROMlib_WriteWhen (WriteWhenType when); 

extern void do_dump_screen (void);
}
extern "C" {
#endif
extern char *ROMlib_configfilename;
extern FILE *configfile;
#ifdef __cplusplus
}
#endif

#endif /* !defined(__RSYS_PREFS__) */
