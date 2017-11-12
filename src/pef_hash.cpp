/* Copyright 2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

#include "MemoryMgr.h"

#include "rsys/pef.h"
#include "rsys/cfm.h"

#include <math.h>

using namespace Executor;

/*
 *
 * IM 8-43 provides C code that uses a loop to determine the exponent
 * for a given number of symbols.  This loop is equivalent:
 *
 * static UInt8
 * PEFComputeHashTableExponent (int32 exportCount)
 * {
 *   int exponent;
 * 
 *   for (exponent = 0;
 *        exponent < kExponentLimit;
 *        ++exponent)
 *     if ((exportCount / (1 << exponent)) < kAverageChainLimit)
 *       break;
 * 
 *   return exponent;
 * }
 *
 * We get the same values by doing a little math.
 * 
 */

#if !defined(__USE_ISOC9X)

PRIVATE uint32
floor_log2(double d)
{
    uint32 retval;

    if(d <= 1)
        retval = 0;
    else
    {
        uint32 u;

        u = d;
        retval = 31;
        while(!(u & 0x80000000))
        {
            u <<= 1;
            --retval;
        }
    }
    return retval;
}
#endif

PRIVATE uint8
PEFComputeHashTableExponent(int32 count)
{
    int retval;

    if(count < kAverageChainLimit)
        retval = 0;
    else if(count >= kAverageChainLimit * (1 << kExponentLimit))
        retval = kExponentLimit;
    else
#if defined(__USE_ISOC9X)
        retval = floor(log2((double)count / kAverageChainLimit)) + 1;
#else
        retval = floor_log2((double)count / kAverageChainLimit) + 1;
#endif

    return retval;
}

/* 8-41 has a hash-word function that will produce the same values as
   the following code */

#define PseudoRotate(x)         \
    ({                          \
        int32 _x;               \
                                \
        _x = (x);               \
        (_x << 1) - (_x >> 16); \
    })

static uint32
PEFComputeHashWord(const unsigned char *orig_p, uint32 namemax)
{
    uint32 retval;
    const unsigned char *p;
    unsigned char c;
    int32 val;

    val = 0;
    for(p = orig_p; (c = *p++) && namemax-- > 0;)
        val = PseudoRotate(val) ^ c;

    retval = (((p - orig_p - 1) << kPEFHashLengthShift) | ((val ^ (val >> 16)) & kPEFHashValueMask));

    return retval;
}

#define PEFHashTableIndex(fullHashWord, hashTablePower)                                      \
    ({                                                                                       \
        decltype(fullHashWord) _fullHashWord = (fullHashWord);                               \
        decltype(hashTablePower) _hashTablePower = (hashTablePower);                         \
                                                                                             \
        (_fullHashWord ^ (_fullHashWord >> _hashTablePower)) & ((1 << _hashTablePower) - 1); \
    })

/*
 * This is used for pseudo-libraries (MathLib, InterfaceLib)
 */

typedef struct
{
    int hash_index;
    uint32 hash_word;
    GUEST<uint32> class_and_name_x;
    GUEST<void *> value;
} sort_entry_t;

PRIVATE int
hash_index_compare(const void *p1, const void *p2)
{
    const sort_entry_t *sp1 = (const sort_entry_t *)p1;
    const sort_entry_t *sp2 = (const sort_entry_t *)p2;
    int retval;

    retval = sp1->hash_index - sp2->hash_index;
    return retval;
}

PRIVATE void
update_export_hash_table(uint32 *hashp, int hash_index, int first_index,
                         int run_count)
{
    uint32 new_value;

    new_value = ((run_count << CHAIN_COUNT_SHIFT) | (first_index & FIRST_INDEX_MASK));
    hashp[hash_index] = CL_RAW(new_value);
}

PUBLIC PEFLoaderInfoHeader_t *
ROMlib_build_pef_hash(const map_entry_t table[], int count)
{
    PEFLoaderInfoHeader_t *retval;
    uint32 hash_power;
    int n_hash_entries;
    uint32 hash_offset, export_offset, symbol_table_offset, string_table_offset;
    int hash_length, export_length, symbol_table_length, string_table_length;
    Size n_bytes_needed;
    uint32 *hashp;
    uint32 *exportp;
    PEFExportedSymbol *symbol_tablep;
    char *string_tablep;
    int i;

    hash_power = PEFComputeHashTableExponent(count);
    n_hash_entries = 1 << hash_power;

    hash_offset = sizeof(PEFLoaderInfoHeader_t);
    hash_length = sizeof(*hashp) * n_hash_entries;

    export_offset = hash_offset + hash_length;
    export_length = sizeof(*exportp) * count;

    symbol_table_offset = export_offset + export_length;
    symbol_table_length = sizeof(*symbol_tablep) * count;

    string_table_offset = symbol_table_offset + symbol_table_length;
    string_table_length = 0;
    for(i = 0; i < count; ++i)
        string_table_length += strlen(table[i].symbol_name);

    n_bytes_needed = (string_table_offset + string_table_length);

    retval = (decltype(retval))NewPtrSysClear(n_bytes_needed);
    if(retval)
    {
        sort_entry_t *sorted;
        int previous_hash_index;
        int run_count;
        int previous_index_of_first_element;
        uint32 name_offset;

        PEFLIH_MAIN_SECTION_X(retval) = CLC(-1);
        PEFLIH_MAIN_OFFSET_X(retval) = CLC(-1);
        PEFLIH_INIT_SECTION_X(retval) = CLC(-1);
        PEFLIH_INIT_OFFSET_X(retval) = CLC(-1);
        PEFLIH_TERM_SECTION_X(retval) = CLC(-1);
        PEFLIH_TERM_OFFSET_X(retval) = CLC(-1);
        /* totalImportedSymbolCount, relocSectionCount, relocInstrOffset
	 are all already 0 */
        PEFLIH_STRINGS_OFFSET_X(retval) = CL(string_table_offset);
        PEFLIH_HASH_OFFSET_X(retval) = CL(hash_offset);
        PEFLIH_HASH_TABLE_POWER_X(retval) = CL(hash_power);
        PEFLIH_SYMBOL_COUNT_X(retval) = CL(count);

        hashp = (decltype(hashp))((char *)retval + hash_offset);
        exportp = (decltype(exportp))((char *)retval + export_offset);
        symbol_tablep = ((decltype(symbol_tablep))((char *)retval + symbol_table_offset));
        string_tablep = ((decltype(string_tablep))((char *)retval + string_table_offset));
        sorted = (sort_entry_t *)alloca(sizeof *sorted * count);
        name_offset = 0;
        for(i = 0; i < count; ++i)
        {
            int length;

            sorted[i].hash_word = PEFComputeHashWord((unsigned char *)table[i].symbol_name,
                                                     strlen(table[i].symbol_name));
            sorted[i].hash_index = PEFHashTableIndex(sorted[i].hash_word,
                                                     hash_power);
            sorted[i].class_and_name_x = CL((kPEFTVectSymbol << 24) | name_offset);
            sorted[i].value = guest_cast<void *>(RM(&table[i].value)); // ### ???
            length = strlen(table[i].symbol_name);
            memcpy(string_tablep + name_offset, table[i].symbol_name, length);
            name_offset += length;
        }
        qsort(sorted, count, sizeof *sorted, hash_index_compare);
        previous_hash_index = -1;
#if !defined(LETGCCWAIL)
        previous_index_of_first_element = 0;
        run_count = 0;
#endif
        for(i = 0; i < count; ++i)
        {
            if(sorted[i].hash_index == previous_hash_index)
                ++run_count;
            else
            {
                if(previous_hash_index >= 0)
                    update_export_hash_table(hashp, previous_hash_index,
                                             previous_index_of_first_element,
                                             run_count);
                run_count = 1;
                previous_hash_index = sorted[i].hash_index;
                previous_index_of_first_element = i;
            }
            exportp[i] = sorted[i].hash_word;
            PEFEXS_CLASS_AND_NAME_X(&symbol_tablep[i]) = sorted[i].class_and_name_x;
            PEFEXS_SYMBOL_VALUE_X(&symbol_tablep[i]) = guest_cast<uint32_t>(sorted[i].value);
            PEFEXS_SECTION_INDEX_X(&symbol_tablep[i]) = CWC(-2);
        }
        update_export_hash_table(hashp, previous_hash_index,
                                 previous_index_of_first_element,
                                 run_count);
    }
    return retval;
}

#if 0
PEFExportedSymbol *
lookup_by_index (const pef_hash_t *hashp, int index,
		 const char **namep, int *namelen)
{
  PEFExportedSymbol *retval;

  if (index >= hashp->n_symbols)
    retval = NULL;
  else
    {
      int name_offset;

      retval = &hashp->symbol_table[index];
      name_offset = CL (retval->classAndName) & NAME_MASK;
      *namep = hashp->symbol_names + name_offset;
      *namelen = CL (hashp->export_key_table[index]) >> 16;
    }
  
  return retval;
}
#endif

#define SYMBOL_NAME(pefexp, str) (str + PEFEXS_NAME(pefexp))

PRIVATE PEFExportedSymbol *
lookup_by_name(const ConnectionID connp,
               const char *name, int name_len)
{
    uint32 hash_word;
    uint32 hash_index;
    uint32 chain_count_and_first_index;
    int chain_count;
    int index;
    int past_index;
    uint32 hash_word_swapped;
    PEFExportedSymbol *retval;
    PEFLoaderInfoHeader_t *lihp;
    uint32 *hash_entries;
    uint32 *export_key_table;
    PEFExportedSymbol *symbol_table;
    uint32 offset;
    const char *string_tablep;

#if 1
#warning get rid of this eventually

    if(connp == (ConnectionID)0x12348765)
        return NULL;

#endif

    lihp = MR(connp->lihp);

    hash_word = PEFComputeHashWord((unsigned char *)name, name_len);
    hash_index = PEFHashTableIndex(hash_word, PEFLIH_HASH_TABLE_POWER(lihp));

    offset = PEFLIH_HASH_OFFSET(lihp);
    hash_entries = (decltype(hash_entries))((char *)lihp + offset);

    offset += sizeof *hash_entries * (1 << PEFLIH_HASH_TABLE_POWER(lihp));
    export_key_table = (decltype(export_key_table))((char *)lihp + offset);

    offset += sizeof *export_key_table * PEFLIH_SYMBOL_COUNT(lihp);
    symbol_table = (decltype(symbol_table))((char *)lihp + offset);

    offset = PEFLIH_STRINGS_OFFSET(lihp);
    string_tablep = (decltype(string_tablep))((char *)lihp + offset);

    chain_count_and_first_index = CL_RAW(hash_entries[hash_index]);
    chain_count = ((chain_count_and_first_index >> CHAIN_COUNT_SHIFT)
                   & CHAIN_COUNT_MASK);
    index = ((chain_count_and_first_index >> FIRST_INDEX_SHIFT)
             & FIRST_INDEX_MASK);
    hash_word_swapped = CL_RAW(hash_word);
    for(past_index = index + chain_count;
        index < past_index && (export_key_table[index] != hash_word_swapped || strncmp(name, SYMBOL_NAME(&symbol_table[index], string_tablep), name_len) != 0);
        ++index)
        ;
    if(index >= past_index)
        retval = NULL;
    else
        retval = &symbol_table[index];
    return retval;
}

P2(PUBLIC pascal trap, OSErr, CountSymbols, ConnectionID, id,
   GUEST<LONGINT> *, countp)
{
    OSErr retval;

    /* TODO */
    warning_unimplemented(NULL_STRING);
    retval = paramErr;
    return retval;
}

P5(PUBLIC pascal trap, OSErr, GetIndSymbol, ConnectionID, id,
   LONGINT, index, Str255, name, GUEST<Ptr> *, addrp, SymClass *, classp)
{
    OSErr retval;

    /* TODO */
    warning_unimplemented(NULL_STRING);
    retval = paramErr;
    return retval;
}

P4(PUBLIC pascal trap, OSErr, FindSymbol, ConnectionID, connID,
   Str255, symName, GUEST<Ptr> *, symAddr, SymClass *, symClass)
{
    OSErr retval;
    PEFExportedSymbol *pefs;

    pefs = lookup_by_name(connID, (char *)symName + 1, symName[0]);
    if(!pefs)
        retval = fragSymbolNotFound;
    else
    {
        int section_index;
        uint32 val;

        if(symClass)
            *symClass = *(SymClass *)&PEFEXS_CLASS_AND_NAME_X(pefs);
        section_index = PEFEXS_SECTION_INDEX(pefs);
        val = PEFEXS_SYMBOL_VALUE(pefs);
        switch(section_index)
        {
            case -2: /* absolute address */
                *symAddr = guest_cast<Ptr>(CL(val));
                break;
            case -3: /* re-exported */
                warning_unimplemented("name = '%.*s', val = 0x%x", symName[0],
                                      symName + 1, val);
                *symAddr = guest_cast<Ptr>(CL(val));
                break;
            default:
            {
                GUEST<uint32> sect_start = connID->sects[section_index].start;
                *symAddr = RM(val + MR(guest_cast<Ptr>(sect_start)));
            }
            break;
        }

        retval = noErr;
    }
    return retval;
}
