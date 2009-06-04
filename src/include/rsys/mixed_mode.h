#if !defined(_mixed_mode_h_)
#define _mixed_mode_h_

#include <stdarg.h>

typedef uint8 ISAType; 
typedef uint16 CallingConventionType;
typedef uint32 ProcInfoType;
typedef uint16 RegisterSelectorType;
typedef uint16 RoutineFlagsType;

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

typedef uint8 RDFlagsType;

typedef struct RoutineRecord
{
  ProcInfoType procInfo PACKED;
  uint8 reserved1 LPACKED;
  ISAType ISA LPACKED;
  RoutineFlagsType routineFlags PACKED;
  ProcPtr procDescriptor PACKED;
  uint32 reserved2 PACKED;
  uint32 selector PACKED;
}
RoutineRecord;

typedef struct RoutineDescriptor
{
  uint16 goMixedModeTrap PACKED;
  uint8 version LPACKED;
  RDFlagsType routineDescriptorFlags LPACKED;
  uint32 reserved1 PACKED;
  uint8 reserved2 LPACKED;
  uint8 selectorInfo LPACKED;
  uint16 routineCount PACKED;
  RoutineRecord routineRecords[1] LPACKED;
}
RoutineDescriptor;

enum { MIXED_MODE_TRAP = 0xAAFE };
enum { kRoutineDescriptorVersion = 7 };
enum { kSelectorsAreNotIndexable = 0 };

enum
{
  kNoByteCode,
  kOneByteCode,
  kTwoByteCode,
  kFourByteCode,
};

enum { kCallingConventionWidth = 4 };
enum { kStackParameterPhase = 6 };
enum { kStackParameterWidth = 2 };
enum { kResultSizeWidth = 2 };

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

extern long CallUniversalProc_from_native (UniversalProcPtr proc,
					   ProcInfoType info, ...);

typedef enum
{
  args_via_stdarg,
  args_via_68k_stack,
  args_via_68k_regs,
}
where_args_t;

extern long
CallUniversalProc_from_native_common (va_list ap, where_args_t where,
				      ProcPtr proc, ProcInfoType info);

extern UniversalProcPtr C_NewRoutineDescriptor (ProcPtr proc,
						ProcInfoType info,
						ISAType isa);

extern void C_DisposeRoutineDescriptor (UniversalProcPtr ptr);

extern UniversalProcPtr C_NewFatRoutineDescriptor (ProcPtr m68k, ProcPtr ppc,
						   ProcInfoType info);

extern OSErr C_SaveMixedModeState (void *statep, uint32 vers);

extern OSErr C_RestoreMixedModeState (void *statep, uint32 vers);

#endif
