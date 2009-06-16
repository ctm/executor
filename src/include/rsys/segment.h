#if !defined (_SEGMENT_H_)
#define _SEGMENT_H_

extern char ROMlib_exit;


#if !defined (USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES)

#include <SegmentLdr.h>

typedef struct {
    INTEGER message;
    INTEGER count;
    AppFile files[1];
} finderinfo;


extern int ROMlib_print;
extern void flushcache( void );
extern void SFSaveDisk_Update (INTEGER vrefnum, Str255 filename);

extern char *ROMlib_undotdot (char *origp);

#if defined(ONLY_DESTROY_BETWEEN_CODE_SEGMENTS)
extern INTEGER ROMlib_num_code_resources;
#endif /* ONLY_DESTROY_BETWEEN_CODE_SEGMENTS */

#if defined (MSDOS)
extern char ROMlib_savecwd[];
#endif

#if defined (NEXTSTEP)
extern LONGINT ROMlib_appbit, ROMlib_whichapps;
extern INTEGER ROMlib_acceptsanotherfile;
#endif

extern void ROMlib_seginit (LONGINT argc, char **argv);
extern void empty_timer_queues (void);
extern BOOLEAN ROMlib_startupscreen;
#endif

#endif /* !_SEGMENT_H_ */
