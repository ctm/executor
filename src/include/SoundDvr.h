#if !defined (__SOUND__)
#define __SOUND__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: SoundDvr.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor {
#define swMode	(-1)
#define ftMode	1
#define ffMode	0

typedef Byte FreeWave[30001];
struct FFSynthRec { GUEST_STRUCT;
    GUEST< INTEGER> mode;
    GUEST< Fixed> fcount;
    GUEST< FreeWave> waveBytes;
};
typedef FFSynthRec *FFSynthPtr;

struct Tone { GUEST_STRUCT;
    GUEST< INTEGER> tcount;
    GUEST< INTEGER> amplitude;
    GUEST< INTEGER> tduration;
};
typedef Tone Tones[5001];

struct SWSynthRec { GUEST_STRUCT;
    GUEST< INTEGER> mode;
    GUEST< Tones> triplets;
};
typedef SWSynthRec *SWSynthPtr;

#if 1|| !defined(__alpha)
typedef Byte Wave[256];
#else /* defined(__alpha) */
typedef Byte Wave;
#warning improper Wave typedef
#endif /* defined(__alpha) */

typedef Wave *WavePtr;

struct FTSoundRec { GUEST_STRUCT;
    GUEST< INTEGER> fduration;
    GUEST< Fixed> sound1Rate;
    GUEST< LONGINT> sound1Phase;
    GUEST< Fixed> sound2Rate;
    GUEST< LONGINT> sound2Phase;
    GUEST< Fixed> sound3Rate;
    GUEST< LONGINT> sound3Phase;
    GUEST< Fixed> sound4Rate;
    GUEST< LONGINT> sound4Phase;
    GUEST< WavePtr> sound1Wave;
    GUEST< WavePtr> sound2Wave;
    GUEST< WavePtr> sound3Wave;
    GUEST< WavePtr> sound4Wave;
};
typedef FTSoundRec *FTSndRecPtr;
MAKE_HIDDEN(FTSndRecPtr);

struct FTSynthRec { GUEST_STRUCT;
    GUEST< INTEGER> mode;
    GUEST< FTSndRecPtr> sndRec;
};
typedef FTSynthRec *FTsynthPtr;

#if 0
#if !defined (SoundBase_H)
extern HIDDEN_Ptr 	SoundBase_H;
extern Byte 	SdVolume;
extern Byte 	SoundActive;
extern INTEGER 	CurPitch;
#endif

#define SoundBase	(SoundBase_H.p)
#endif
}

#endif /* __SOUND__ */
