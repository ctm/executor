#if !defined(__RSYS_SOUNDOPTS__)
#define __RSYS_SOUNDOPTS__

#include "SoundMgr.h"
#include "rsys/pstuff.h"
/* to get extern for `ROMlib_PretendSound' */
#include "rsys/prefs.h"

namespace Executor {
typedef enum {
    soundoff,
    soundpretend,
    soundon
} sound_t;

#if defined (MACOSX_)
extern void ROMlib_outbuffer( char *buf, LONGINT nsamp, LONGINT rate, void *chanp );
extern void ROMlib_callcompletion( void *chanp );
#else /* !MACOSX_ */
#define ROMlib_outbuffer(buf, nsamp, rate, chanp) \
	ROMlib_soundcomplete (chanp)
#define ROMlib_callcompletion(chanp) \
	ROMlib_soundcomplete (chanp)
#endif /* !MACOSX_ */

extern void ROMlib_soundcomplete( void *chanp );

#define CHAN_ALLOC_FLAG		(1 << 0)
#define CHAN_BUSY_FLAG		(1 << 1)
#define CHAN_IMMEDIATE_FLAG	(1 << 2)
#define CHAN_CMDINPROG_FLAG     (1 << 3)
#define CHAN_DBINPROG_FLAG      (1 << 4)

extern void ROMlib_MUTEX_CONDITION_CREATE( LONGINT *lp );
extern void ROMlib_MUTEX_LOCK( LONGINT lp );
extern void ROMlib_MUTEX_UNLOCK( LONGINT lp );
extern void ROMlib_CONDITION_SIGNAL( LONGINT lp );
extern void ROMlib_CONDITION_WAIT( LONGINT lp );
extern void ROMlib_MUTEX_CONDITION_DESTROY( LONGINT lp );

extern void C_sound_timer_handler (void);
extern void clear_pending_sounds (void);

/* patl stuff */

extern HIDDEN_SndChannelPtr allchans;

typedef uint64_t snd_time;

#define SND_PROMOTE(x) (((snd_time)x) << (4 * sizeof (snd_time)))
#define SND_DEMOTE(x) (((snd_time)x) >> (4 * sizeof (snd_time)))

typedef LONGINT TimeL;

typedef struct _ModifierStub : GuestStruct {
    GUEST< struct _ModifierStub*> nextStub;
    GUEST< ProcPtr> code;
    GUEST< LONGINT> userInfo;
    GUEST< TimeL> count;
    GUEST< TimeL> every;
    GUEST< SignedByte> flags;
    GUEST< SignedByte> hState;
    GUEST< snd_time> current_start;
    GUEST< snd_time> time;
    GUEST< uint8> prev_samp;
    GUEST< SndDoubleBufferHeader*> dbhp;
    GUEST< int> current_db;
} ModifierStub, *ModifierStubPtr;


#define SND_CHAN_FIRSTMOD(c) MR ((ModifierStubPtr)c->firstMod)
#define SND_CHAN_CURRENT_START(c) (SND_CHAN_FIRSTMOD (c)->current_start)
#define SND_CHAN_TIME(c) (SND_CHAN_FIRSTMOD (c)->time)
#define SND_CHAN_PREV_SAMP(c) (SND_CHAN_FIRSTMOD (c)->prev_samp)
#define SND_CHAN_CURRENT_DB(c) (SND_CHAN_FIRSTMOD (c)->current_db)
#define SND_CHAN_DBHP(c) (SND_CHAN_FIRSTMOD (c)->dbhp)

#define SND_CHAN_CMDINPROG_P(c) (SND_CHAN_FLAGS_X (c) & CWC (CHAN_CMDINPROG_FLAG))
#define SND_CHAN_DBINPROG_P(c) (SND_CHAN_FLAGS_X (c) & CWC (CHAN_DBINPROG_FLAG))

extern int ROMlib_SND_RATE;
#define SND_RATE ROMlib_SND_RATE

struct hunger_info
{
  snd_time t2;  /* Time of earliest sample which can be provided */
  snd_time t3;  /* Time of latest sample which must be provided */
  snd_time t4;  /* Time of latest sample which can be provided */
  unsigned char *buf;	/* NULL means there is no buffer; just "pretend" */
  int bufsize;		/* to fill it in; (!buf && bufsize) is possible! */
};
  
typedef struct hunger_info HungerInfo;

extern syn68k_addr_t sound_callback (syn68k_addr_t, void *);

extern bool sound_disabled_p;

extern int ROMlib_get_snd_cmds (Handle sndh, SndCommand **cmdsp);
	
BOOLEAN C_snth5( SndChannelPtr, SndCommand *, ModifierStubPtr );
}


#endif /* !defined(__RSYS_SOUNDOPTS__) */
