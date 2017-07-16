#if !defined(_cfm_h_)
#define _cfm_h_

#include "FileMgr.h"

/*
 * Copyright 2000 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: cfm.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor {
enum
{
  kUnresolvedCFragSymbolAddress = 0
};

typedef struct PACKED
{
  uint32 reserved0;
  uint32 reserved1;
  uint32 version;
  uint32 reserved2;
  uint32 reserved3;
  uint32 reserved4;
  uint32 reserved5;
  int32 n_descripts;
}
cfrg_resource_t;

#define CFRG_VERSION_X(cfrg) ((cfgr)->version)
#define CFRG_VERSION(cfrg) (CL (CFRG_VERSION_X (cfgr)))

#define CFRG_N_DESCRIPTS_X(cfrg) ((cfrg)->n_descripts)
#define CFRG_N_DESCRIPTS(cfrg) (CL (CFRG_N_DESCRIPTS_X (cfrg)))

typedef struct PACKED
{
  OSType isa;
  uint32 update_level;
  uint32 current_version;
  uint32 oldest_definition_version;
  uint32 stack_size;
  int16 appl_library_dir;
  uint8 fragment_type;
  uint8 fragment_location;
  int32 offset_to_fragment;
  int32 fragment_length;
  uint32 reserved0;
  uint32 reserved1;
  uint16 cfir_length;
  unsigned char name[1];
}
cfir_t;

#define CFIR_ISA_X(cfir) ((cfir)->isa)
#define CFIR_ISA(cfir) (CL (CFIR_ISA_X (cfir)))

#define CFIR_TYPE_X(cfir) ((cfir)->fragment_type)
#define CFIR_TYPE(cfir) (CB (CFIR_TYPE_X (cfir)))

#define CFIR_LOCATION_X(cfir) ((cfir)->fragment_location)
#define CFIR_LOCATION(cfir) (CL (CFIR_LOCATION_X(cfir)))

#define CFIR_LENGTH_X(cfir) ((cfir)->cfir_length)
#define CFIR_LENGTH(cfir) (CL (CFIR_LENGTH_X(cfir)))

#define CFIR_OFFSET_TO_FRAGMENT_X(cfir) ((cfir)->offset_to_fragment)
#define CFIR_OFFSET_TO_FRAGMENT(cfir) (CL (CFIR_OFFSET_TO_FRAGMENT_X(cfir)))

#define CFIR_FRAGMENT_LENGTH_X(cfir) ((cfir)->fragment_length)
#define CFIR_FRAGMENT_LENGTH(cfir) (CL (CFIR_FRAGMENT_LENGTH_X(cfir)))

#define CFIR_NAME(cfir) ((cfir)->name)

enum
{
  kImportLibraryCFrag,
  kApplicationCFrag,
  kDropInAdditionCFrag,
  kStubLibraryCFrag,
  kWeakStubLibraryCFrag,
};


enum
{
  kWholeFork = 0
};

enum
{
  kInMem, kOnDiskFlat, kOnDiskSegmented,
};

enum
{
  kPowerPCArch     = T('p', 'w', 'p', 'c'),
  kMotorola68KArch = T('m', '6', '8', 'k'),
};

typedef enum
{
  kLoadLib = 1, /* deprecated */
  kReferenceCFrag = 1,
  kFindLib = 2,
  kLoadNewCopy = 5,
}
LoadFlags;

typedef struct PACKED MemFragment
{
  Ptr address;
  uint32 length;
  BOOLEAN inPlace;
}
MemFragment;

typedef struct PACKED DiskFragment
{
  FSSpecPtr fileSpec;
  uint32 offset;
  uint32 length;
}
DiskFragment;

typedef struct PACKED SegmentedFragment
{
  FSSpecPtr fileSpec;
  OSType rsrcType;
  INTEGER rsrcID;
}
SegmentedFragment;

typedef struct PACKED FragmentLocator
{
  uint32 where;
  union
  {
    MemFragment inMem;
    DiskFragment onDisk;
    SegmentedFragment inSegs;
  } u;
}
FragmentLocator;

typedef struct PACKED InitBlock
{
  uint32 contextID;
  uint32 closureID;
  FragmentLocator fragLocator;
  Ptr libName;
  uint32 reserved4a;
  uint32 reserved4b;
  uint32 reserved4c;
  uint32 reserved4d;
}
InitBlock;

typedef struct CFragConnection *ConnectionID;

extern OSErr C_GetDiskFragment (FSSpecPtr fsp, LONGINT offset, LONGINT length,
				Str63 fragname, LoadFlags flags,
				ConnectionID *connp, Ptr *mainAddrp,
				Str255 errname);

typedef uint8 SymClass;

extern OSErr C_FindSymbol (ConnectionID connID, Str255 symName, Ptr *symAddr,
			   SymClass *symClass);


extern char *ROMlib_p2cstr (StringPtr str);

/* NOTE: The following data structures are just for proof-of-concept
   use.  They are not going to be compatible with what is really on a
   Macintosh, but they may be good enough to get PS5.5 limping. */

typedef struct
{
  syn68k_addr_t start;
  uint32 length;
  uint32 ref_count;
  uint8 perms;
  /* TODO: should probably pad this with three bytes and then PACK the entire
     structure, but only after verifying that it works that way on a Mac. */
}
section_info_t;

typedef struct PACKED CFragConnection
{
  FragmentLocator frag;
  struct PEFLoaderInfoHeader *lihp;
  uint32 ref_count;
  uint32 n_sects;
  section_info_t sects[0];
}
CFragConnection_t;

enum
{
  fragConnectionIDNotFound = -2801,
  fragSymbolNotFound = -2802,
  fragLibNotFound = -2804,
  fragNoMem = -2809
};

typedef struct PACKED
{
  ConnectionID cid;
  int32 n_symbols;
  int32 first_symbol;
}
lib_t;

#define LIB_CID_X(l)  ((l)->cid)
#define LIB_CID(l)    (CL (LIB_CID_X (l)))

#define LIB_N_SYMBOLS_X(l) ((l)->n_symbols)
#define LIB_N_SYMBOLS(l) (CL (LIB_N_SYMBOLS_X (l)))

#define LIB_FIRST_SYMBOL_X(l) ((l)->first_symbol)
#define LIB_FIRST_SYMBOL(l) (CL (LIB_FIRST_SYMBOL_X (l)))

typedef struct PACKED
{
  uint32 n_libs;
  lib_t libs[0];
}
CFragClosure_t;

#define N_LIBS_X(c)  ((c)->n_libs)
#define N_LIBS(c) (CL (N_LIBS_X (c)))

typedef CFragClosure_t *CFragClosureID;

typedef struct PACKED
{
  const char *symbol_name;
  void *value;
}
map_entry_t;

extern cfir_t *ROMlib_find_cfrg (Handle cfrg, OSType arch, uint8 type,
				 Str255 name);

extern OSErr C_CloseConnection (ConnectionID *cidp);


extern OSErr C_GetSharedLibrary (Str63 library, OSType arch,
				 LoadFlags loadflags,
				 ConnectionID *cidp, Ptr *mainaddrp,
				 Str255 errName);

extern OSErr C_GetMemFragment (void * addr, uint32 length, Str63 fragname,
			       LoadFlags flags, ConnectionID *connp,
			       Ptr *mainAddrp, Str255 errname);

extern OSErr C_CountSymbols (ConnectionID id, LONGINT *countp);

extern OSErr C_GetIndSymbol (ConnectionID id, LONGINT index,
			     Str255 name, Ptr *addrp,
			     SymClass *classp);

extern ConnectionID ROMlib_new_connection (uint32 n_sects);
extern void ROMlib_release_tracking_values (void);
}

#endif
