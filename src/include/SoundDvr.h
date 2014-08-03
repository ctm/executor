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
typedef struct PACKED {
  INTEGER mode;
  Fixed fcount;
  FreeWave waveBytes;
} FFSynthRec;
typedef FFSynthRec *FFSynthPtr;

typedef struct PACKED {
  INTEGER tcount;
  INTEGER amplitude;
  INTEGER tduration;
} Tone;
typedef Tone Tones[5001];

typedef struct PACKED {
  INTEGER mode;
  Tones triplets;
} SWSynthRec;
typedef SWSynthRec *SWSynthPtr;

#if 1|| !defined(__alpha)
typedef Byte Wave[256];
#else /* defined(__alpha) */
typedef Byte Wave;
#warning improper Wave typedef
#endif /* defined(__alpha) */

typedef Wave *WavePtr;

typedef struct PACKED {
  INTEGER fduration;
  Fixed sound1Rate;
  LONGINT sound1Phase;
  Fixed sound2Rate;
  LONGINT sound2Phase;
  Fixed sound3Rate;
  LONGINT sound3Phase;
  Fixed sound4Rate;
  LONGINT sound4Phase;
  PACKED_MEMBER(WavePtr, sound1Wave);
  PACKED_MEMBER(WavePtr, sound2Wave);
  PACKED_MEMBER(WavePtr, sound3Wave);
  PACKED_MEMBER(WavePtr, sound4Wave);
} FTSoundRec;
typedef FTSoundRec *FTSndRecPtr;
MAKE_HIDDEN(FTSndRecPtr);

typedef struct PACKED {
  INTEGER mode;
  PACKED_MEMBER(FTSndRecPtr, sndRec);
} FTSynthRec;
typedef FTSynthRec *FTsynthPtr;


#if !defined (SoundBase_H)
extern HIDDEN_Ptr 	SoundBase_H;
extern Byte 	SdVolume;
extern Byte 	SoundActive;
extern INTEGER 	CurPitch;
#endif

#define SoundBase	(SoundBase_H.p)
}

#endif /* __SOUND__ */
