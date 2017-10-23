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

struct cfrg_resource_t { GUEST_STRUCT;
    GUEST< uint32> reserved0;
    GUEST< uint32> reserved1;
    GUEST< uint32> version;
    GUEST< uint32> reserved2;
    GUEST< uint32> reserved3;
    GUEST< uint32> reserved4;
    GUEST< uint32> reserved5;
    GUEST< int32> n_descripts;
};

#define CFRG_VERSION_X(cfrg) ((cfgr)->version)
#define CFRG_VERSION(cfrg) (CL (CFRG_VERSION_X (cfgr)))

#define CFRG_N_DESCRIPTS_X(cfrg) ((cfrg)->n_descripts)
#define CFRG_N_DESCRIPTS(cfrg) (CL (CFRG_N_DESCRIPTS_X (cfrg)))

struct cfir_t { GUEST_STRUCT;
    GUEST< OSType> isa;
    GUEST< uint32> update_level;
    GUEST< uint32> current_version;
    GUEST< uint32> oldest_definition_version;
    GUEST< uint32> stack_size;
    GUEST< int16> appl_library_dir;
    GUEST< uint8> fragment_type;
    GUEST< uint8> fragment_location;
    GUEST< int32> offset_to_fragment;
    GUEST< int32> fragment_length;
    GUEST< uint32> reserved0;
    GUEST< uint32> reserved1;
    GUEST< uint16> cfir_length;
    GUEST< unsigned char[1]> name;
};

#define CFIR_ISA_X(cfir) ((cfir)->isa)
#define CFIR_ISA(cfir) (CL (CFIR_ISA_X (cfir)))

#define CFIR_TYPE_X(cfir) ((cfir)->fragment_type)
#define CFIR_TYPE(cfir) (CB (CFIR_TYPE_X (cfir)))

#define CFIR_LOCATION_X(cfir) ((cfir)->fragment_location)
#define CFIR_LOCATION(cfir) (CL (CFIR_LOCATION_X(cfir)))

#define CFIR_LENGTH_X(cfir) ((cfir)->cfir_length)
#define CFIR_LENGTH(cfir) (CW (CFIR_LENGTH_X(cfir)))

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

struct MemFragment {
    GUEST_STRUCT;
    GUEST< Ptr> address;
    GUEST< uint32> length;
    GUEST< BOOLEAN> inPlace;
};

struct DiskFragment {
    GUEST_STRUCT;
    GUEST< FSSpecPtr> fileSpec;
    GUEST< uint32> offset;
    GUEST< uint32> length;
};

struct SegmentedFragment {
    GUEST_STRUCT;
    GUEST< FSSpecPtr> fileSpec;
    GUEST< OSType> rsrcType;
    GUEST< INTEGER> rsrcID;
};

typedef struct FragmentLocator
{
  GUEST_STRUCT;
  GUEST<uint32> where;
  union
  {
    MemFragment inMem;
    DiskFragment onDisk;
    SegmentedFragment inSegs;
  } u;
}
FragmentLocator;

struct InitBlock {
    GUEST_STRUCT;
    GUEST< uint32> contextID;
    GUEST< uint32> closureID;
    GUEST< FragmentLocator> fragLocator;
    GUEST< Ptr> libName;
    GUEST< uint32> reserved4a;
    GUEST< uint32> reserved4b;
    GUEST< uint32> reserved4c;
    GUEST< uint32> reserved4d;
};

typedef struct CFragConnection *ConnectionID;

extern OSErr C_GetDiskFragment (FSSpecPtr fsp, LONGINT offset, LONGINT length,
				Str63 fragname, LoadFlags flags,
				GUEST<ConnectionID> *connp, GUEST<Ptr> *mainAddrp,
				Str255 errname);

typedef uint8 SymClass;

extern OSErr C_FindSymbol (ConnectionID connID, Str255 symName, GUEST<Ptr> *symAddr,
			   SymClass *symClass);


extern char *ROMlib_p2cstr (StringPtr str);

/* NOTE: The following data structures are just for proof-of-concept
   use.  They are not going to be compatible with what is really on a
   Macintosh, but they may be good enough to get PS5.5 limping. */

struct section_info_t
{
    GUEST_STRUCT;
    GUEST<syn68k_addr_t> start;
    GUEST<uint32> length;
    GUEST<uint32> ref_count;
    GUEST<uint8> perms;
  /* TODO: should probably pad this with three bytes and then PACK the entire
     structure, but only after verifying that it works that way on a Mac. */
};


typedef struct CFragConnection { GUEST_STRUCT;
    GUEST< FragmentLocator> frag;
    GUEST< struct PEFLoaderInfoHeader*> lihp;
    GUEST< uint32> ref_count;
    GUEST< uint32> n_sects;
    GUEST< section_info_t[0]> sects;
} CFragConnection_t;

enum
{
  fragConnectionIDNotFound = -2801,
  fragSymbolNotFound = -2802,
  fragLibNotFound = -2804,
  fragNoMem = -2809
};

struct lib_t { GUEST_STRUCT;
    GUEST< ConnectionID> cid;
    GUEST< int32> n_symbols;
    GUEST< int32> first_symbol;
};

#define LIB_CID_X(l)  ((l)->cid)
#define LIB_CID(l)    (CL (LIB_CID_X (l)))

#define LIB_N_SYMBOLS_X(l) ((l)->n_symbols)
#define LIB_N_SYMBOLS(l) (CL (LIB_N_SYMBOLS_X (l)))

#define LIB_FIRST_SYMBOL_X(l) ((l)->first_symbol)
#define LIB_FIRST_SYMBOL(l) (CL (LIB_FIRST_SYMBOL_X (l)))

struct CFragClosure_t { GUEST_STRUCT;
    GUEST< uint32> n_libs;
    GUEST< lib_t[0]> libs;
};

#define N_LIBS_X(c)  ((c)->n_libs)
#define N_LIBS(c) (CL (N_LIBS_X (c)))

typedef CFragClosure_t *CFragClosureID;

struct map_entry_t { GUEST_STRUCT;
    const char* symbol_name;
    void* value;
};

extern cfir_t *ROMlib_find_cfrg (Handle cfrg, OSType arch, uint8 type,
				 Str255 name);

extern OSErr C_CloseConnection (ConnectionID *cidp);


extern OSErr C_GetSharedLibrary (Str63 library, OSType arch,
				 LoadFlags loadflags,
				 GUEST<ConnectionID> *cidp, GUEST<Ptr> *mainaddrp,
				 Str255 errName);

extern OSErr C_GetMemFragment (void * addr, uint32 length, Str63 fragname,
			       LoadFlags flags, GUEST<ConnectionID> *connp,
			       GUEST<Ptr> *mainAddrp, Str255 errname);

extern OSErr C_CountSymbols (ConnectionID id, GUEST<LONGINT> *countp);

extern OSErr C_GetIndSymbol (ConnectionID id, LONGINT index,
			     Str255 name, GUEST<Ptr> *addrp,
			     SymClass *classp);

extern ConnectionID ROMlib_new_connection (uint32 n_sects);
extern void ROMlib_release_tracking_values (void);
}

#endif
