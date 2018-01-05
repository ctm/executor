#if !defined(__SOUND__)
#define __SOUND__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
enum
{
    swMode = (-1),
    ftMode = 1,
    ffMode = 0,
};

typedef Byte FreeWave[30001];
struct FFSynthRec
{
    GUEST_STRUCT;
    GUEST<INTEGER> mode;
    GUEST<Fixed> fcount;
    GUEST<FreeWave> waveBytes;
};
typedef FFSynthRec *FFSynthPtr;

struct Tone
{
    GUEST_STRUCT;
    GUEST<INTEGER> tcount;
    GUEST<INTEGER> amplitude;
    GUEST<INTEGER> tduration;
};
typedef Tone Tones[5001];

struct SWSynthRec
{
    GUEST_STRUCT;
    GUEST<INTEGER> mode;
    GUEST<Tones> triplets;
};
typedef SWSynthRec *SWSynthPtr;

#if 1 || !defined(__alpha)
typedef Byte Wave[256];
#else /* defined(__alpha) */
typedef Byte Wave;
// FIXME: #warning improper Wave typedef
#endif /* defined(__alpha) */

typedef Wave *WavePtr;

struct FTSoundRec
{
    GUEST_STRUCT;
    GUEST<INTEGER> fduration;
    GUEST<Fixed> sound1Rate;
    GUEST<LONGINT> sound1Phase;
    GUEST<Fixed> sound2Rate;
    GUEST<LONGINT> sound2Phase;
    GUEST<Fixed> sound3Rate;
    GUEST<LONGINT> sound3Phase;
    GUEST<Fixed> sound4Rate;
    GUEST<LONGINT> sound4Phase;
    GUEST<WavePtr> sound1Wave;
    GUEST<WavePtr> sound2Wave;
    GUEST<WavePtr> sound3Wave;
    GUEST<WavePtr> sound4Wave;
};
typedef FTSoundRec *FTSndRecPtr;

struct FTSynthRec
{
    GUEST_STRUCT;
    GUEST<INTEGER> mode;
    GUEST<FTSndRecPtr> sndRec;
};
typedef FTSynthRec *FTsynthPtr;

const LowMemGlobal<Byte> SdVolume { 0x260 }; // SoundDvr IMII-232 (true-b);
const LowMemGlobal<FTSndRecPtr> SoundPtr { 0x262 }; // SoundDvr IMII-227 (false);
const LowMemGlobal<Ptr> SoundBase { 0x266 }; // SoundDvr IMIII-21 (true-b);
const LowMemGlobal<Byte> SoundLevel { 0x27F }; // SoundDvr IMII-234 (false);
const LowMemGlobal<INTEGER> CurPitch { 0x280 }; // SoundDvr IMII-226 (true-b);

}

#endif /* __SOUND__ */
