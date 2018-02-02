#if !defined(_mixed_mode_h_)
#define _mixed_mode_h_

#include <stdarg.h>
namespace Executor
{
typedef uint8_t ISAType;
typedef uint16_t CallingConventionType;
typedef uint32_t ProcInfoType;
typedef uint16_t RegisterSelectorType;
typedef uint16_t RoutineFlagsType;

enum
{
    kM68kISA = 0,
    kPowerPCISA = 1
};

enum
{
    kPascalStackBased,
    kCStackBased,
    kRegisterBased,
    kThinkCStackBased = 5,
    kD0DispatchedPascalStackBased = 8,
    kD0DispatchedCStackBased = 9,
    kD1DispatchedPascalStackBased = 12,
    kStackDispatchedPascalStackBased = 14,
    kSpecialCase,
};

typedef uint8_t RDFlagsType;

struct RoutineRecord
{
    GUEST_STRUCT;
    GUEST<ProcInfoType> procInfo;
    GUEST<uint8_t> reserved1;
    GUEST<ISAType> ISA;
    GUEST<RoutineFlagsType> routineFlags;
    GUEST<ProcPtr> procDescriptor;
    GUEST<uint32_t> reserved2;
    GUEST<uint32_t> selector;
};

struct RoutineDescriptor
{
    GUEST_STRUCT;
    GUEST<uint16_t> goMixedModeTrap;
    GUEST<uint8_t> version;
    GUEST<RDFlagsType> routineDescriptorFlags;
    GUEST<uint32_t> reserved1;
    GUEST<uint8_t> reserved2;
    GUEST<uint8_t> selectorInfo;
    GUEST<uint16_t> routineCount;
    GUEST<RoutineRecord[1]> routineRecords;
};

enum
{
    MIXED_MODE_TRAP = 0xAAFE
};
enum
{
    kRoutineDescriptorVersion = 7
};
enum
{
    kSelectorsAreNotIndexable = 0
};

enum
{
    kNoByteCode,
    kOneByteCode,
    kTwoByteCode,
    kFourByteCode,
};

enum
{
    kCallingConventionWidth = 4
};
enum
{
    kStackParameterPhase = 6
};
enum
{
    kStackParameterWidth = 2
};
enum
{
    kResultSizeWidth = 2
};

enum
{
    kRegisterD0 = 0,
    kRegisterD1, /* 1 */
    kRegisterD2, /* 2 */
    kRegisterD3, /* 3 */
    kRegisterA0, /* 4 */
    kRegisterA1, /* 5 */
    kRegisterA2, /* 6 */
    kRegisterA3, /* 7 */
    kRegisterD4, /* 8 */
    kRegisterD5, /* 9 */
    kRegisterD6, /* 10 */
    kREgisterD7, /* 11 */
    kRegisterA4, /* 12 */
    kRegisterA5, /* 13 */
    kRegisterA6, /* 14 */
    kCCRegisterCBit = 16,
    kCCRegisterVBit, /* 17 */
    kCCRegisterZBit, /* 18 */
    kCCRegisterNBit, /* 19 */
    kCCRegisterXBit /* 20 */
};

typedef RoutineDescriptor *UniversalProcPtr;

#define RESULT_SIZE(n) ((n) << kCallingConventionWidth)
#define STACK_ROUTINE_PARAMETER(arg, n) \
    ((n) << (kStackParameterPhase + ((arg)-1) * kStackParameterWidth))

extern long CallUniversalProc_from_native(UniversalProcPtr proc,
                                          ProcInfoType info, ...);

typedef enum {
    args_via_stdarg,
    args_via_68k_stack,
    args_via_68k_regs,
} where_args_t;

extern long
CallUniversalProc_from_native_common(va_list ap, where_args_t where,
                                     ProcPtr proc, ProcInfoType info);

DISPATCHER_TRAP(MixedModeDispatch, 0xAA59, D0W);

extern UniversalProcPtr C_NewRoutineDescriptor(ProcPtr proc,
                                               ProcInfoType info,
                                               ISAType isa);
PASCAL_SUBTRAP(NewRoutineDescriptor, 0xAA59, 0x0000, MixedModeDispatch);

extern void C_DisposeRoutineDescriptor(UniversalProcPtr ptr);
PASCAL_SUBTRAP(DisposeRoutineDescriptor, 0xAA59, 0x0001, MixedModeDispatch);

extern UniversalProcPtr C_NewFatRoutineDescriptor(ProcPtr m68k, ProcPtr ppc,
                                                  ProcInfoType info);
PASCAL_SUBTRAP(NewFatRoutineDescriptor, 0xAA59, 0x0002, MixedModeDispatch);

extern OSErr C_SaveMixedModeState(void *statep, uint32_t vers);
PASCAL_SUBTRAP(SaveMixedModeState, 0xAA59, 0x0003, MixedModeDispatch);

extern OSErr C_RestoreMixedModeState(void *statep, uint32_t vers);
PASCAL_SUBTRAP(RestoreMixedModeState, 0xAA59, 0x0004, MixedModeDispatch);

}
#endif
