//
//  SpeechManager.h
//  CocoaExecutor
//
//  Created by C.W. Betts on 8/4/14.
//  Copyright (c) 2014 C.W. Betts. All rights reserved.
//

#ifndef __CocoaExecutor__SpeechManager__
#define __CocoaExecutor__SpeechManager__

#include "SoundMgr.h"
#include "FileMgr.h"

namespace Executor
{
#pragma pack(push, 2)
typedef struct VoiceSpec
{
    GUEST_STRUCT;
    GUEST<OSType> creator;
    GUEST<OSType> id;
} VoiceSpec, *VoiceSpecPtr;

typedef struct VoiceFileInfo
{
    GUEST_STRUCT;
    FSSpec fileSpec;
    GUEST<uint16_t> resID;
} VoiceFileInfo;

typedef struct SpeechStatusInfo
{
    GUEST_STRUCT;
    GUEST<Boolean> outputBusy;
    GUEST<Boolean> outputPaused;
    GUEST<int32_t> inputBytesLeft;
    GUEST<int16_t> phonemeCode;
} SpeechStatusInfo;

typedef struct VoiceDescription
{
    GUEST_STRUCT;
    GUEST<int32_t> length;
    GUEST<VoiceSpec> voice;
    GUEST<int32_t> version;
    GUEST<Str63> name;
    GUEST<Str255> comment;
    GUEST<int16_t> gender;
    GUEST<int16_t> age;
    GUEST<int16_t> script;
    GUEST<int16_t> language;
    GUEST<int16_t> region;
    GUEST<int32_t> reserved[4];
} VoiceDescription;

typedef struct SpeechChannelRecord
{
    GUEST_STRUCT;
    LONGINT data[1];
} SpeechChannelRecord, *SpeechChannel;

typedef struct PhonemeInfo
{
    GUEST_STRUCT;
    GUEST<int16_t> opcode;
    GUEST<Str15> phStr;
    GUEST<Str31> exampleStr;
    GUEST<int16_t> hiliteStart;
    GUEST<int16_t> hiliteEnd;
} PhonemeInfo;

typedef struct PhonemeDescriptor
{
    GUEST_STRUCT;
    GUEST<int16_t> phonemeCount;
    GUEST<PhonemeInfo> thePhonemes[1];
} PhonemeDescriptor;

typedef struct SpeechXtndData
{
    GUEST_STRUCT;
    GUEST<OSType> synthCreator;
    GUEST<Byte> synthData[2];
} SpeechXtndData;

typedef struct DelimiterInfo
{
    GUEST_STRUCT;
    GUEST<Byte> startDelimiter[2];
    GUEST<Byte> endDelimiter[2];
} DelimiterInfo;

#pragma pack(pop)

PUBLIC pascal NumVersion C_SpeechManagerVersion(void);
PUBLIC pascal int16_t C_SpeechBusy(void);
PUBLIC pascal int16_t C_SpeechBusySystemWide(void);
PUBLIC pascal OSErr C_CountVoices(int16_t *numVoices);
PUBLIC pascal OSErr C_DisposeSpeechChannel(SpeechChannel chan);
PUBLIC pascal OSErr C_SpeakString(Str255 textToBeSpoken);

PUBLIC pascal OSErr C_StopSpeech(SpeechChannel chan);
PUBLIC pascal OSErr C_ContinueSpeech(SpeechChannel chan);

PUBLIC pascal OSErr C_GetIndVoice(int16_t index, VoiceSpec *voice);
PUBLIC pascal OSErr C_NewSpeechChannel(VoiceSpec *voice, SpeechChannel *chan);
PUBLIC pascal OSErr C_StopSpeechAt(SpeechChannel chan, int32_t whereToStop);
PUBLIC pascal OSErr C_PauseSpeechAt(SpeechChannel chan, int32_t whereToPause);
PUBLIC pascal OSErr C_SetSpeechRate(SpeechChannel chan, Fixed rate);
PUBLIC pascal OSErr C_GetSpeechRate(SpeechChannel chan, Fixed *rate);
PUBLIC pascal OSErr C_SetSpeechPitch(SpeechChannel chan, Fixed pitch);
PUBLIC pascal OSErr C_GetSpeechPitch(SpeechChannel chan, Fixed *pitch);
PUBLIC pascal OSErr C_UseDictionary(SpeechChannel chan, Handle dictionary);
PUBLIC pascal OSErr C_MakeVoiceSpec(OSType creator, OSType id, VoiceSpec *voice);
PUBLIC pascal OSErr C_GetVoiceDescription(const VoiceSpec *voice, VoiceDescription *info, LONGINT infoLength);
PUBLIC pascal OSErr C_GetVoiceInfo(const VoiceSpec *voice, OSType selector, void *voiceInfo);
PUBLIC pascal OSErr C_SpeakText(SpeechChannel chan, const void *textBuf, ULONGINT textBytes);
PUBLIC pascal OSErr C_SetSpeechInfo(SpeechChannel chan, OSType selector, const void *speechInfo);
PUBLIC pascal OSErr C_GetSpeechInfo(SpeechChannel chan, OSType selector, void *speechInfo);
PUBLIC pascal OSErr C_SpeakBuffer(SpeechChannel chan, const void *textBuf, ULONGINT textBytes, int32_t controlFlags);
PUBLIC pascal OSErr C_TextToPhonemes(SpeechChannel chan, const void *textBuf, ULONGINT textBytes, Handle phonemeBuf, GUEST<LONGINT> *phonemeBytes);
}

#endif /* defined(__CocoaExecutor__SpeechManager__) */
