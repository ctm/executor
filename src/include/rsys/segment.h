#if !defined(_SEGMENT_H_)
#define _SEGMENT_H_

extern char ROMlib_exit;

#include "SegmentLdr.h"

namespace Executor
{
struct finderinfo
{
    GUEST_STRUCT;
    GUEST<INTEGER> message;
    GUEST<INTEGER> count;
    GUEST<AppFile[1]> files;
};

extern int ROMlib_print;
extern void flushcache(void);
extern void SFSaveDisk_Update(INTEGER vrefnum, Str255 filename);

extern char *ROMlib_undotdot(char *origp);

extern LONGINT ROMlib_appbit, ROMlib_whichapps;

extern void ROMlib_seginit(LONGINT argc, char **argv);
extern void empty_timer_queues(void);
}

#endif /* !_SEGMENT_H_ */
