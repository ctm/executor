#if !defined (__SOUND__)
#define __SOUND__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: SoundDvr.h 63 2004-12-24 18:19:43Z ctm $
 */


#define swMode	(-1)
#define ftMode	1
#define ffMode	0

typedef Byte FreeWave[30001];
typedef struct {
    INTEGER mode	PACKED;
    Fixed fcount	PACKED;
    FreeWave waveBytes	LPACKED;
} FFSynthRec;
typedef FFSynthRec *FFSynthPtr;

typedef struct {
    INTEGER tcount	PACKED;
    INTEGER amplitude	PACKED;
    INTEGER tduration	PACKED;
} Tone;
typedef Tone Tones[5001];

typedef struct {
    INTEGER mode	PACKED;
    Tones triplets	LPACKED;
} SWSynthRec;
typedef SWSynthRec *SWSynthPtr;

#if 1|| !defined(__alpha)
typedef Byte Wave[256];
#else /* defined(__alpha) */
typedef Byte Wave;
#warning improper Wave typedef
#endif /* defined(__alpha) */

typedef Wave *WavePtr;

typedef struct {
    INTEGER fduration	PACKED;
    Fixed sound1Rate	PACKED;
    LONGINT sound1Phase	PACKED;
    Fixed sound2Rate	PACKED;
    LONGINT sound2Phase	PACKED;
    Fixed sound3Rate	PACKED;
    LONGINT sound3Phase	PACKED;
    Fixed sound4Rate	PACKED;
    LONGINT sound4Phase	PACKED;
    WavePtr sound1Wave	PACKED_P;
    WavePtr sound2Wave	PACKED_P;
    WavePtr sound3Wave	PACKED_P;
    WavePtr sound4Wave	PACKED_P;
} FTSoundRec;
typedef FTSoundRec *FTSndRecPtr;
typedef struct { FTSndRecPtr p PACKED_P; } HIDDEN_FTSndRecPtr;

typedef struct {
    INTEGER mode	PACKED;
    FTSndRecPtr sndRec	PACKED_P;
} FTSynthRec;
typedef FTSynthRec *FTsynthPtr;


#if !defined (SoundBase_H)
extern HIDDEN_Ptr 	SoundBase_H;
extern Byte 	SdVolume;
extern Byte 	SoundActive;
extern INTEGER 	CurPitch;
#endif

#define SoundBase	(SoundBase_H.p)

#endif /* __SOUND__ */
