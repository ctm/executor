#if !defined(_pef_h_)
#define _pef_h_

#include "rsys/cfm.h"

/*
 * Copyright 1999-2000 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: pef.h 63 2004-12-24 18:19:43Z ctm $
 */
namespace Executor {
typedef struct PACKED PEFContainerHeader
{
  OSType tag1;
  OSType tag2;
  OSType architecture;
  uint32 formatVersion;
  uint32 dateTimeStamp;
  uint32 oldDefVersion;
  uint32 oldImpVersion;
  uint32 currentVersion;
  uint16 sectionCount;
  uint16 instSectionCount;
  uint32 reservedA;
}
PEFContainerHeader_t;

#define PEF_CONTAINER_TAG1_X(p) ((p)->tag1)
#define PEF_CONTAINER_TAG1(p) (CL (PEF_CONTAINER_TAG1_X (p)))

#define PEF_CONTAINER_TAG2_X(p) ((p)->tag2)
#define PEF_CONTAINER_TAG2(p) (CL (PEF_CONTAINER_TAG2_X (p)))

#define PEF_CONTAINER_ARCHITECTURE_X(p) ((p)->architecture)
#define PEF_CONTAINER_ARCHITECTURE(p) (CL (PEF_CONTAINER_ARCHITECTURE_X (p)))

#define PEF_CONTAINER_FORMAT_VERSION_X(p) ((p)->formatVersion)
#define PEF_CONTAINER_FORMAT_VERSION(p) (CL (PEF_CONTAINER_FORMAT_VERSION_X (p)))

#define PEF_CONTAINER_SECTION_COUNT_X(p) ((p)->sectionCount)
#define PEF_CONTAINER_SECTION_COUNT(p) (CW (PEF_CONTAINER_SECTION_COUNT_X (p)))

#define PEF_CONTAINER_INSTSECTION_COUNT_X(p) ((p)->instSectionCount)
#define PEF_CONTAINER_INSTSECTION_COUNT(p) (CW (PEF_CONTAINER_INSTSECTION_COUNT_X (p)))

#define PEF_CONTAINER_DATE_X(p) ((p)->dateTimeStamp)
#define PEF_CONTAINER_DATE(p) (CL (PEF_CONTAINER_DATE_X (p))

#define PEF_CONTAINER_OLD_DEV_VERS_X(p) ((p)->oldDefVersion)
#define PEF_CONTAINER_OLD_DEV_VERS(p) (CL (PEF_CONTAINER_OLD_DEV_VERS_X(p)))

#define PEF_CONTAINER_OLD_IMP_VERS_X(p) ((p)->oldImpVersion)
#define PEF_CONTAINER_OLD_IMP_VERS(p) (CL (PEF_CONTAINER_OLD_IMP_VERS_X(p)))

#define PEF_CONTAINER_CURRENT_VERS_X(p) ((p)->currentVersion)
#define PEF_CONTAINER_CURRENT_VERS(p) (CL (PEF_CONTAINER_CURRENT_VERS_X(p)))

typedef struct PACKED PEFSectionHeader
{
  int32 nameOffset;
  uint32 defaultAddress;
  uint32 totalSize;
  uint32 unpackedSize;
  uint32 packedSize;
  uint32 containerOffset;
  uint8 sectionKind;
  uint8 shareKind;
  uint8 alignment;
  uint8 reservedA;
}
PEFSectionHeader_t;

#define PEFSH_DEFAULT_ADDRESS_X(p) ((p)->defaultAddress)
#define PEFSH_DEFAULT_ADDRESS(p) (CL (PEFSH_DEFAULT_ADDRESS_X (p)))

#define PEFSH_TOTAL_SIZE_X(p) ((p)->totalSize)
#define PEFSH_TOTAL_SIZE(p) (CL (PEFSH_TOTAL_SIZE_X (p)))

#define PEFSH_UNPACKED_SIZE_X(p) ((p)->unpackedSize)
#define PEFSH_UNPACKED_SIZE(p) (CL (PEFSH_UNPACKED_SIZE_X (p)))

#define PEFSH_PACKED_SIZE_X(p) ((p)->packedSize)
#define PEFSH_PACKED_SIZE(p) (CL (PEFSH_PACKED_SIZE_X (p)))

#define PEFSH_CONTAINER_OFFSET_X(p) ((p)->containerOffset)
#define PEFSH_CONTAINER_OFFSET(p) (CL (PEFSH_CONTAINER_OFFSET_X (p)))

#define PEFSH_SECTION_KIND(p) ((p)->sectionKind)
#define PEFSH_SHARE_KIND(p) ((p)->shareKind)
#define PEFSH_ALIGNMENT(p) (1 << (p)->alignment)

typedef struct PACKED PEFLoaderInfoHeader
{
  int32 mainSection;
  uint32 mainOffset;
  int32 initSection;
  uint32 initOffset;
  int32 termSection;
  uint32 termOffset;
  uint32 importedLibraryCount;
  uint32 totalImportedSymbolCount;
  uint32 relocSectionCount;
  uint32 relocInstrOffset;
  uint32 loaderStringsOffset;
  uint32 exportHashOffset;
  uint32 exportHashTablePower;
  uint32 exportedSymbolCount;
}
PEFLoaderInfoHeader_t;

#define PEFLIH_MAIN_SECTION_X(p) ((p)->mainSection)
#define PEFLIH_MAIN_SECTION(p) (CL (PEFLIH_MAIN_SECTION_X(p)))

#define PEFLIH_MAIN_OFFSET_X(p) ((p)->mainOffset)
#define PEFLIH_MAIN_OFFSET(p) (CL (PEFLIH_MAIN_OFFSET_X(p)))

#define PEFLIH_INIT_SECTION_X(p) ((p)->initSection)
#define PEFLIH_INIT_SECTION(p) (CL (PEFLIH_INIT_SECTION_X(p)))

#define PEFLIH_INIT_OFFSET_X(p) ((p)->initOffset)
#define PEFLIH_INIT_OFFSET(p) (CL (PEFLIH_INIT_OFFSET_X(p)))

#define PEFLIH_TERM_SECTION_X(p) ((p)->termSection)
#define PEFLIH_TERM_SECTION(p) (CL (PEFLIH_TERM_SECTION_X(p)))

#define PEFLIH_TERM_OFFSET_X(p) ((p)->termOffset)
#define PEFLIH_TERM_OFFSET(p) (CL (PEFLIH_TERM_OFFSET_X(p)))

#define PEFLIH_IMPORTED_LIBRARY_COUNT_X(p) ((p)->importedLibraryCount)
#define PEFLIH_IMPORTED_LIBRARY_COUNT(p) (CL (PEFLIH_IMPORTED_LIBRARY_COUNT_X(p)))

#define PEFLIH_IMPORTED_SYMBOL_COUNT_X(p) ((p)->totalImportedSymbolCount)
#define PEFLIH_IMPORTED_SYMBOL_COUNT(p) (CL (PEFLIH_IMPORTED_SYMBOL_COUNT_X(p)))

#define PEFLIH_RELOC_SECTION_COUNT_X(p) ((p)->relocSectionCount)
#define PEFLIH_RELOC_SECTION_COUNT(p) (CL (PEFLIH_RELOC_SECTION_COUNT_X(p)))

#define PEFLIH_RELOC_INSTR_OFFSET_X(p) ((p)->relocInstrOffset)
#define PEFLIH_RELOC_INSTR_OFFSET(p) (CL (PEFLIH_RELOC_INSTR_OFFSET_X(p)))

#define PEFLIH_STRINGS_OFFSET_X(p) ((p)->loaderStringsOffset)
#define PEFLIH_STRINGS_OFFSET(p) (CL (PEFLIH_STRINGS_OFFSET_X(p)))

#define PEFLIH_HASH_OFFSET_X(p)	((p)->exportHashOffset)
#define PEFLIH_HASH_OFFSET(p)	(CL (PEFLIH_HASH_OFFSET_X(p)))

#define PEFLIH_HASH_TABLE_POWER_X(p)	((p)->exportHashTablePower)
#define PEFLIH_HASH_TABLE_POWER(p) (CL (PEFLIH_HASH_TABLE_POWER_X (p)))

#define PEFLIH_SYMBOL_COUNT_X(p)	((p)->exportedSymbolCount)
#define PEFLIH_SYMBOL_COUNT(p) (CL (PEFLIH_SYMBOL_COUNT_X (p)))

typedef struct PACKED PEFImportedLibrary
{
  uint32 nameOffset;
  uint32 oldImpVersion;
  uint32 currentVersion;
  uint32 importedSymbolCount;
  uint32 firstImportedSymbol;
  uint8 options;
  uint8 reservedA;
  uint16 reservedB;
}
PEFImportedLibrary_t;

#define PEFIL_NAME_OFFSET_X(p) ((p)->nameOffset)
#define PEFIL_NAME_OFFSET(p) (CL (PEFIL_NAME_OFFSET_X (p)))

#define PEFIL_SYMBOL_COUNT_X(p) ((p)->importedSymbolCount)

#define PEFIL_FIRST_SYMBOL_X(p) ((p)->firstImportedSymbol)

typedef struct PACKED PEFLoaderRelocationHeader
{
  uint16 sectionIndex;
  uint16 reservedA;
  uint32 relocCount;
  uint32 firstRelocOffset;
}
PEFLoaderRelocationHeader_t;

#define PEFRLH_RELOC_COUNT_X(p) ((p)->relocCount)
#define PEFRLH_RELOC_COUNT(p) (CL (PEFRLH_RELOC_COUNT_X(p)))

#define PEFRLH_FIRST_RELOC_OFFSET_X(p) ((p)->firstRelocOffset)
#define PEFRLH_FIRST_RELOC_OFFSET(p) (CL (PEFRLH_FIRST_RELOC_OFFSET_X(p)))

#define PEFRLH_SECTION_INDEX_X(p) ((p)->sectionIndex)
#define PEFRLH_SECTION_INDEX(p) (CW (PEFRLH_SECTION_INDEX_X(p)))

enum
{
  kExponentLimit = 16,
  kAverageChainLimit = 10,
};

enum
{
  kPEFHashLengthShift = 16,
  kPEFHashValueMask = 0xFFFF,
};

typedef uint32 hash_table_entry_t;

enum
{
  FIRST_INDEX_SHIFT = 0,
  FIRST_INDEX_MASK = 0x3FFFF,
  CHAIN_COUNT_SHIFT = 18,
  CHAIN_COUNT_MASK = 0x3FFF,
};

typedef struct PACKED PEFExportedSymbol
{
  uint32 classAndName;
  uint32 symbolValue;
  int16 sectionIndex;
}
PEFExportedSymbol;

#define PEFEXS_CLASS_AND_NAME_X(p) ((p)->classAndName)
#define PEFEXS_CLASS_AND_NAME(p) (CL (PEFEXS_CLASS_AND_NAME_X (p)))

#define PEFEXS_NAME(p) (PEFEXS_CLASS_AND_NAME (p) & 0xffffff)

#define PEFEXS_SYMBOL_VALUE_X(p) ((p)->symbolValue)
#define PEFEXS_SYMBOL_VALUE(p) (CL (PEFEXS_SYMBOL_VALUE_X(p)))

#define PEFEXS_SECTION_INDEX_X(p) ((p)->sectionIndex)
#define PEFEXS_SECTION_INDEX(p) (CW (PEFEXS_SECTION_INDEX_X(p)))

enum
{
  NAME_MASK = 0xFFFFFF,
};

enum
{
  kPEFCodeSymbol,
  kPEFDataSymbol,
  kPEFTVectSymbol,
  kPEFTOCSymbol,
  kPEFGlueSymbol,
};

#if 0
typedef struct pef_hash
{
  uint32 n_symbols; /* exportedSymbolCount */
  uint32 n_hash_entries; /* 1 << exportHashTablePower */
  hash_table_entry_t *hash_entries; /* exportHashOffset */
  uint32 *export_key_table; /* hash_entries + n_hash_entries */
  PEFExportedSymbol *symbol_table; /* hash_entries + 2 * n_hash_entries */
  const char *symbol_names; /* loaderStringsOffset */
}
pef_hash_t;
#endif

extern PEFLoaderInfoHeader_t *ROMlib_build_pef_hash (const map_entry_t table[],
						     int count);
}
#endif
