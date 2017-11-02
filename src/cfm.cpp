/* Copyright 2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */
#if defined (CFM_PROBLEMS)

#warning "No CFM support for now, even though it limped previously."

#elif defined (powerpc)

/*
 * In addition to fleshing this out, it probably makes sense to use mmap
 * to pull in certain sections (including temporarily the loader section).
 */

#include "rsys/common.h"

#include <sys/mman.h>
#include <assert.h>

#include "FileMgr.h"
#include "OSUtil.h"
#include "MemoryMgr.h"
#include "SegmentLdr.h"
#include "AliasMgr.h"

#include "rsys/cfm.h"
#include "rsys/pef.h"
#include "rsys/file.h"
#include "rsys/interfacelib.h"
#include "rsys/mathlib.h"
#include "rsys/launch.h"
#include "rsys/hfs.h"
#include "rsys/string.h"

#include "ppc_stubs.h"

using namespace Executor;

typedef enum
{
  process_share = 1,
  global_share = 4,
  protected_share = 5,
}
share_kind_t;

enum
{
  code_section_type = 0,
  unpacked_data_section_type,
  pattern_data_section_type,
  constant_section_type,
  loader_section_type,
  debug_section_type,
  executable_data_section_type,
  exception_section_type,
  traceback_section_type,
};

#undef roundup
#define roundup(n, m)				\
({						\
  typeof (n) __n;				\
  typeof (m) __m;				\
						\
  __n = (n);					\
  __m = (m);					\
  (__n + __m - 1) / m * m;			\
})

#undef rounddown
#define rounddown(n, m)				\
({						\
  typeof (n) __n;				\
  typeof (m) __m;				\
						\
  __n = (n);					\
  __m = (m);					\
  __n / m * m;					\
})

PRIVATE OSErr
try_to_get_memory (void **addrp, syn68k_addr_t default_syn_address,
		   uint32 total_size, int alignment)
{
  OSErr retval;
#if !defined(linux)
  retval = paramErr;
#else
  void *default_address;
  void *received_address;

  default_address = SYN68K_TO_US (default_syn_address);
  total_size = roundup (total_size, getpagesize());
  received_address = mmap (default_address, total_size,
			   PROT_EXEC|PROT_READ|PROT_WRITE,
			   MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
  if (received_address == (void *) -1)
    retval = memFullErr;
  else
    {
      retval = noErr;
      *addrp = received_address;
    }
#endif  
  return retval;
}

enum
{
  readable_section   = (1 << 0),
  writable_section   = (1 << 1),
  executable_section = (1 << 2),
};

PRIVATE OSErr
load_unpacked_section (const void *mmapped_addr,
		       syn68k_addr_t default_address, uint32 total_size,
		       uint32 packed_size, uint32 unpacked_size,
		       uint32 section_offset, share_kind_t share_kind,
		       int alignment, section_info_t *infop)
{
  OSErr retval;
  void *addr;

  if (packed_size != unpacked_size)
    warning_unexpected ("%d %d", packed_size, unpacked_size);
  if (!(infop->perms & writable_section) && total_size == unpacked_size)
    {
      retval = noErr;
      addr = (char *) mmapped_addr + section_offset;
    }
  else
    {
      retval = try_to_get_memory (&addr, default_address, total_size,
				  alignment);
      if (retval == noErr)
	{
	  memcpy (addr, (char *) mmapped_addr + section_offset, packed_size);
	  memset (addr + unpacked_size, 0, total_size - unpacked_size);
	}
    }
  if (retval == noErr)
    {
      infop->start = US_TO_SYN68K (addr);
      infop->length = total_size;
    }

  return retval;
}

PRIVATE OSErr
repeat_block (uint32 repeat_count, uint32 block_size, uint8 **destpp,
	      const uint8 **srcpp, uint32 *dest_lengthp, uint32 *src_lengthp)
{
  OSErr retval;
  uint32 total_bytes_to_write;

  total_bytes_to_write = repeat_count * block_size;
  if (total_bytes_to_write > *dest_lengthp || block_size > *src_lengthp)
    retval = paramErr;
  else
    {
      while (repeat_count-- > 0)
	{
	  memcpy (*destpp, *srcpp, block_size);
	  *destpp += block_size;
	}
      *srcpp += block_size;
      *dest_lengthp -= total_bytes_to_write;
      *src_lengthp -= block_size;
      retval = noErr;
    }
  return retval;
}

PRIVATE OSErr
extract_count (const uint8 **srcpp, uint32 *lengthp, uint32 *countp)
{
  OSErr retval;
  uint32 count;
  uint8 next_piece;
  
  count = *countp;
  do
    {
      retval = (*lengthp > 0) ? noErr : paramErr;
      if (retval == noErr)
	{
	  next_piece = **srcpp;
	  ++*srcpp;
	  --*lengthp;
	  count = (count << 7) | (next_piece & 0x7F);
	}
    }
  while (retval == noErr && (next_piece & 0x80));
  if (retval == noErr)
    *countp = count;

  return retval;
}

PRIVATE OSErr
interleave (uint32 repeat_count, uint32 custom_size, uint8 **destpp,
	    const uint8 **srcpp, uint32 *dest_lengthp, uint32 *src_lengthp,
	    uint32 common_size, const uint8 *common_mem)
{
  OSErr retval;
  uint32 total_size;

  total_size = repeat_count * (common_size + custom_size) + common_size;
  if (total_size > *dest_lengthp || custom_size * repeat_count > *src_lengthp)
    retval = paramErr;
  else
    {
      ++repeat_count;
      while (repeat_count-- > 0)
	{
	  if (common_mem)
	    memcpy (*destpp, common_mem, common_size);
	  else
	    memset (*destpp, 0, common_size);
	  *destpp += common_size;
	  *dest_lengthp -= common_size;
	  if (repeat_count > 0)
	    {
	      memcpy (*destpp, *srcpp, custom_size);
	      *destpp += custom_size;
	      *dest_lengthp -= custom_size;
	      *srcpp += custom_size;
	      *src_lengthp -= custom_size;
	    }
	}
      retval = noErr;
    }
  return retval;
}

PRIVATE OSErr
pattern_initialize (uint8 *addr, const uint8 *patmem, uint32 packed_size,
		    uint32 unpacked_size)
{
  OSErr retval;

  retval = noErr;
  while (retval == noErr && packed_size > 0)
    {
      uint8 opcode;
      uint32 count;

      opcode = *patmem >> 5;
      count = (*patmem & 0x1f);
      ++patmem;
      --packed_size;
      if (!count)
	retval = extract_count (&patmem, &packed_size, &count);
      if (retval == noErr)
	{
	  switch (opcode)
	    {
	    case 0:
	      if (count > unpacked_size)
		retval = paramErr;
	      else
		{
		  memset (addr, 0, count);
		  addr += count;
		  unpacked_size -= count;
		}
	      break;
	    case 1:
	      retval = repeat_block (1, count, &addr, &patmem, &unpacked_size,
				     &packed_size);
	      break;
	    case 2:
	      {
		uint32 repeat_count;

		repeat_count = 0;
		retval = extract_count (&patmem, &packed_size, &repeat_count);
		if (retval == noErr)
		  retval = repeat_block (repeat_count + 1, count, &addr,
					 &patmem, &unpacked_size,
					 &packed_size);
	      }
	      break;
	    case 3:
	    case 4:
	      {
		uint32 common_size;
		uint32 custom_size;

		common_size = count;
		custom_size = 0;
		retval = extract_count (&patmem, &packed_size, &custom_size);
		if (retval == noErr)
		  {
		    uint32 repeat_count;

		    repeat_count = 0;
		    retval = extract_count (&patmem, &packed_size,
					    &repeat_count);
		    if (retval == noErr)
		      {
			const uint8 *common_mem;

			if (opcode == 4)
			  common_mem = 0;
			else
			  {
			    common_mem = patmem;
			    if (common_size > packed_size)
			      retval = paramErr;
			    else
			      {
				patmem += common_size;
				packed_size -= common_size;
			      }
			  }
			if (retval == noErr)
			  interleave (repeat_count, custom_size, &addr,
				      &patmem, &unpacked_size, &packed_size,
				      common_size, common_mem);
		      }
		  }
	      }
	      break;
	    default:
	      warning_unexpected ("%d", opcode);
	      assert (0);
	      break;
	    }
	}
    }
  if (retval == noErr && (packed_size || unpacked_size))
    {
      retval = paramErr;
      warning_unexpected ("%d %d", packed_size, unpacked_size);
      assert (0); 
    }
  return retval;
}

PRIVATE OSErr
load_pattern_section (const void *mmapped_addr,
		     syn68k_addr_t default_address, uint32 total_size,
		     uint32 packed_size, uint32 unpacked_size,
		     uint32 section_offset, share_kind_t share_kind,
		     int alignment, section_info_t *infop)
{
  OSErr retval;
  void *addr;

  retval = try_to_get_memory (&addr, default_address, total_size, alignment);
  if (retval == noErr)
    {
      uint8 *patmem;


      patmem = (typeof (patmem)) ((char *) mmapped_addr + section_offset);
      retval = pattern_initialize (addr, patmem, packed_size, unpacked_size);
      if (retval == noErr)
	{
	  memset (addr + unpacked_size, 0,
		  total_size - unpacked_size);
	  infop->start = US_TO_SYN68K (addr);
	  infop->length = total_size;
	}
    }

  return retval;
}

PRIVATE void
repeatedly_relocate (uint32 count, uint8 **relocAddressp, uint32 val)
{
  uint32 *p;
  p = (uint32 *) *relocAddressp;
  while (count-- > 0)
    {
      *p = CL (CL (*p) + val);
      ++p;
    } 
  *relocAddressp = (uint8 *) p;
}

PRIVATE OSErr
check_existing_connections (Str63 library, OSType arch, LoadFlags loadflags,
			    ConnectionID *cidp, Ptr *mainaddrp, Str255 errName)
{
  /* TODO */
#warning TODO
  return fragLibNotFound;
}

PRIVATE void
get_root_and_app (INTEGER *root_vrefp, LONGINT *root_diridp,
		  INTEGER *app_vrefp , LONGINT *app_diridp)
{
#warning TODO
  /* TODO */
}

PRIVATE OSErr
check_file (INTEGER vref, LONGINT dirid, Str255 file, bool shlb_test_p,
	    Str63 library, OSType arch, LoadFlags loadflags,
	    ConnectionID *cidp, Ptr *mainaddrp, Str255 errName)
{
  OSErr retval;
  FSSpec fs;

  retval = FSMakeFSSpec (vref, dirid, file, &fs);
  if (retval != noErr)
    retval = fragLibNotFound;
  else
    {
      if (shlb_test_p)
	{
	  OSErr err;
	  FInfo finfo;

	  err = FSpGetFInfo (&fs, &finfo);
	  if (err != noErr || finfo.fdType != TICK("shlb"))
	    retval = fragLibNotFound;
	}
      if (retval == noErr)
	{
	  INTEGER rn;
	  
	  rn = FSpOpenResFile (&fs, fsRdPerm);
	  if (rn == -1)
	    retval = fragLibNotFound;
	  else
	    {
	      Handle cfrg0;

	      cfrg0 = Get1Resource (T('c','f','r','g'), 0);
	      if (!cfrg0)
		retval = fragLibNotFound;
	      else
		{
		  cfir_t *cfirp;

		  cfirp = ROMlib_find_cfrg (cfrg0, arch, kImportLibraryCFrag,
					    library);
		  if (!cfirp)
		    retval = fragLibNotFound;
		  else
		    retval = GetDiskFragment (&fs,
					      CFIR_OFFSET_TO_FRAGMENT (cfirp),
					      CFIR_FRAGMENT_LENGTH (cfirp), "",
					      loadflags, cidp, mainaddrp,
					      errName);
		}
	      CloseResFile (rn);
	    }
	}
    }

  return retval;
}

PRIVATE OSErr
check_vanddir (INTEGER vref, LONGINT dirid, int descend_count, Str63 library,
	       OSType arch, LoadFlags loadflags, ConnectionID *cidp,
	       Ptr *mainaddrp, Str255 errName)
{
  CInfoPBRec pb;
  Str255 s;
  OSErr retval;
  INTEGER dirindex;
  OSErr err;
  int errcount;

  retval = fragLibNotFound;
  pb.hFileInfo.ioNamePtr = RM (&s[0]);
  pb.hFileInfo.ioVRefNum = CW (vref);
  err = noErr;
  errcount = 0;
  for (dirindex = 1;
       retval != noErr && err != fnfErr && errcount != 3;
       dirindex++)
    {
      pb.hFileInfo.ioFDirIndex = CW (dirindex);
      pb.hFileInfo.ioDirID   = CL (dirid);
      err = PBGetCatInfo(&pb, false);
      if (err)
	{
	  if (err != fnfErr)
	    {
	      warning_unexpected ("PBGetCatInfo err = %d\n", err);
	      ++errcount;
	    }
	}
      else
	{
	  errcount = 0;
	  if (pb.hFileInfo.ioFlAttrib & ATTRIB_ISADIR)
	    {
	      if (descend_count > 0)
		{
		  retval = check_vanddir (vref, CL (pb.hFileInfo.ioDirID),
					  descend_count-1, library,
					  arch, loadflags, cidp, mainaddrp,
					  errName);
		}
	    }
	  else if (pb.hFileInfo.ioFlFndrInfo.fdType == TICKX ("shlb"))
	    retval = check_file (vref, dirid, s, false, library, arch,
				 loadflags, cidp, mainaddrp, errName);
	}
    }
  return retval;
}

/*
 * This sort of implements the "Searching for Import Libraries" algorithm
 * described on 1-17 and 1-18 of the Mac Runtime Architectures manual.  The
 * biggest difference is that we don't search for the best fit between
 * everything available in the Extensions folder, the ROM registry and the
 * file registry.  Instead we currently stop when we find any fit.
 *
 * Additionally, we don't really have a ROM registry per-se.  Instead
 * we have the home-brewed InterfaceLib and MathLib files.  We don't
 * have anything corresponding to the file registry.
 *
 */

P6 (PUBLIC pascal trap, OSErr, GetSharedLibrary, Str63, library, OSType, arch,
    LoadFlags, loadflags, ConnectionID *, cidp, Ptr *, mainaddrp, Str255,
    errName)
{
  OSErr retval;

  warning_trace_info ("GetSharedLibrary (\"%.*s\")\n", library[0],
		      library+1);

  retval = check_existing_connections (library, arch, loadflags, cidp,
				       mainaddrp, errName);
  if (retval == fragLibNotFound)
    {
      INTEGER root_vref, app_vref;
      LONGINT root_dirid, app_dirid;
	  
      get_root_and_app (&root_vref, &root_dirid, &app_vref, &app_dirid);
      if (root_vref != app_vref || root_dirid != app_dirid)
	retval = check_vanddir (root_vref, root_dirid, 0, library, arch,
				loadflags, cidp, mainaddrp, errName);
      if (retval != noErr)
	retval = check_file (ROMlib_exevrefnum, 0, ROMlib_exefname, false,
			     library, arch, loadflags, cidp, mainaddrp,
			     errName);
      if (retval != noErr)
	retval = check_vanddir (app_vref, app_dirid, 0, library, arch,
				loadflags, cidp, mainaddrp, errName);
      if (retval != noErr)
	retval = check_vanddir (ROMlib_exevrefnum, 0, 0, library, arch,
				loadflags, cidp, mainaddrp, errName);
      if (retval != noErr)
	{
	  INTEGER extensions_vref;
	  LONGINT extensions_dirid;

	  if (FindFolder (0, kExtensionFolderType, false, &extensions_vref,
	                  &extensions_dirid) == noErr)
	    retval = check_vanddir (extensions_vref, extensions_dirid, 1,
				    library, arch, loadflags, cidp,
				    mainaddrp, errName);
	}

      if (retval != noErr)
	{
	  if (EqualString (library, "\7MathLib", false, true))
	    retval = ROMlib_GetMathLib (library, arch, loadflags, cidp,
				mainaddrp, errName);
	  else if (EqualString (library, "\14InterfaceLib", false, true))
	    retval = ROMlib_GetInterfaceLib (library, arch, loadflags, cidp,
				     mainaddrp, errName);
	}
    }
  return retval;
}

P1 (PUBLIC pascal trap, OSErr, CloseConnection, ConnectionID *, cidp)
{
  warning_trace_info ("cidp = %p, cid = 0x%x", cidp, (uint32) *cidp);
  return noErr;
}

enum { tracking_val_start = 0x88123456 };
PRIVATE uint32 num_tracking_vals;
PRIVATE char **tracking_symbols;

PRIVATE void
tracking_handler (int signum, struct sigcontext sc)
{
  uint32 r12;
  uint32 val;

  r12 = sc.regs->gpr[PT_R12];
  val = r12 - (uint32) tracking_val_start;
  if (val < num_tracking_vals)
    fprintf (stderr, "Need glue for '%s'\n", tracking_symbols[val]);
  ExitToShell ();
}

PUBLIC void
ROMlib_release_tracking_values (void)
{
  if (num_tracking_vals > 0)
    {
      int i;

      for (i = 0; i < num_tracking_vals; ++i)
	free (tracking_symbols[i]);
      tracking_symbols = realloc (tracking_symbols, 0);
      num_tracking_vals = 0;
    }
  signal (SIGSEGV, (void *) tracking_handler);
}

PRIVATE uint32
tracking_value (const char *symbol_name)
{
  uint32 retval;

  tracking_symbols = realloc (tracking_symbols,
			      (num_tracking_vals+1) *
			      sizeof *tracking_symbols);
  tracking_symbols[num_tracking_vals] = strdup (symbol_name);
  retval = tracking_val_start + num_tracking_vals++;
  warning_trace_info ("name = '%s' (%p)\n", symbol_name, (Ptr) retval);
  return retval;
}


PRIVATE OSErr
symbol_lookup (uint32 *indexp, Ptr *valp, uint8 imports[][4],
	       const char *symbol_names, CFragClosureID closure_id)
{
  OSErr retval;
  int index;
  uint8 flags;
  uint8 class;
  const char *symbol_name;

  index = *indexp;
  flags = imports[index][0] >> 4;
  class = imports[index][0] & 0xf;
  symbol_name = (symbol_names +
		 (imports[index][1] << 16) +
		 (imports[index][2] << 8) +
		 (imports[index][3] << 0));

  {
    int i;
    int n_libs;

    n_libs = N_LIBS (closure_id);
    for (i = 0; i < n_libs; ++i)
      {
	const lib_t *l;
	uint32 first_symbol;

	l = &closure_id->libs[i];
	first_symbol = LIB_FIRST_SYMBOL (l);
	if (index >= first_symbol &&
	    index < first_symbol + LIB_N_SYMBOLS (l))
	  {
	    Str255 sym255;
	    OSErr err;

	    sym255[0] = MIN (strlen (symbol_name), 255);
	    memcpy (sym255+1, symbol_name, sym255[0]);
	    err = FindSymbol (LIB_CID (l), sym255, valp, 0);
	    if (err != noErr)
	      {
		if (flags & 8)
		  *valp = kUnresolvedCFragSymbolAddress;
		else
		  *valp = (Ptr) tracking_value (symbol_name);
	      }
/*-->*/	    break;
	  }
      }
  }
  ++*indexp;
  retval = noErr;
  return retval;
}

PRIVATE OSErr
relocate (const PEFLoaderRelocationHeader_t reloc_headers[],
	  int section, uint32 reloc_count, uint8 reloc_instrs[][2],
	  uint8 imports[][4], const char *symbol_names,
	  CFragClosureID closure_id, ConnectionID connp)
{
  OSErr retval;
  uint8 *relocAddress;
  uint32 importIndex;
  syn68k_addr_t sectionC;
  syn68k_addr_t sectionD;
  int32 repeat_remaining;

  repeat_remaining = -1; /* i.e. not currently processing RelocSmRepeat or
			    RelocLgRepeat */
  
  relocAddress = (uint8 *) SYN68K_TO_US (connp->sects[section].start);
  importIndex = 0;
  sectionC = connp->sects[0].start;
  sectionD = connp->sects[1].start;

  retval = noErr;
  while (reloc_count-- > 0)
    {
      uint8 msb;

      msb = reloc_instrs[0][0];
      if ((msb >> 6) == 0)
	{
	  uint8 lsb;
	  int skipCount;
	  int relocCount;

	  lsb = reloc_instrs[0][1];
	  skipCount = (msb << 2) | (lsb >> 6);
	  relocCount = (lsb & 0x3f);
	  relocAddress += skipCount * 4;
	  repeatedly_relocate (relocCount, &relocAddress, sectionD);
	}
      else
	{
	  switch ((msb >> 5))
	    {
	    case 2:
	      {
		int sub_op;
		int run_length;
		sub_op = (msb >> 1) & 0xf;
		run_length = (((msb & 1) << 8)
			      | reloc_instrs[0][1]);
		++run_length;
		switch (sub_op)
		  {
		  case 0: /* RelocBySectC */
		    repeatedly_relocate (run_length, &relocAddress, sectionC);
		    break;
		  case 1: /* RelocBySectD */
		    repeatedly_relocate (run_length, &relocAddress, sectionD);
		    break;
		  case 2:
		    while (run_length-- > 0)
		      {
			repeatedly_relocate (1, &relocAddress, sectionC);
			repeatedly_relocate (1, &relocAddress, sectionD);
			relocAddress += 4;
		      }
		    break;
		  case 3: /* RelocTVector8 */
		    while (run_length-- > 0)
		      {
			repeatedly_relocate (1, &relocAddress, sectionC);
			repeatedly_relocate (1, &relocAddress, sectionD);
		      }
		    break;
		  case 4:
		    while (run_length-- > 0)
		      {
			repeatedly_relocate (1, &relocAddress, sectionD);
			relocAddress += 4;
		      }
		    break;
		  case 5: /* RelocImportRun */
		    while (retval == noErr && run_length-- > 0)
		      {
			Ptr symbol_val;

			retval = symbol_lookup (&importIndex, &symbol_val,
						 imports, symbol_names,
						closure_id);
			if (retval == noErr)
			  repeatedly_relocate (1, &relocAddress,
					       (uint32) symbol_val);
		      }
		    break;
		  default:
		    warning_unexpected ("%d", sub_op);
		    assert (0);
		    retval = paramErr;
		  }
	      }
	    break;
	    case 3:
	      {
		int sub_op;
		int index;
		Ptr symbol_val;

		sub_op = (msb >> 1) & 0xf;
		index = (((msb & 1) << 8)
			 | reloc_instrs[0][1]);
		switch (sub_op)
		  {
		  case 0:
		    importIndex = index;
		    retval = symbol_lookup (&importIndex, &symbol_val,
					    imports, symbol_names,
					    closure_id);
		    if (retval == noErr)
		      repeatedly_relocate (1, &relocAddress,
					   (uint32) symbol_val);
		    break;
		  case 1:
		    sectionC = connp->sects[index].start;
		    warning_unimplemented ("RelocSmSetSectC not tested much");
		    assert (0);
		    break;
		  case 2:
		    sectionD = connp->sects[index].start;
		    warning_unimplemented ("RelocSmSetSectD not tested much");
		    break;
		  case 3:
		    fprintf (stderr, "RelocSmBySection\n");
		    assert (0);
		    break;
		  default:
		    warning_unexpected ("sub_op");
		    fprintf (stderr, "Relocte By Index sub_op = %d\n", sub_op);
		    assert (0);
		    break;
		  }
	      }
	      break;
	    default:
	      switch ((msb >> 4))
		{
		case 8:
		  {
		    uint32 offset;

		    offset = ((msb & 0xf) << 8) | (reloc_instrs[0][1]);
		    ++offset;
		    relocAddress += offset;
		  }
		  break;
		case 9:
		  {
		    warning_unimplemented ("RelocSmRepeat not tested much");
		    if (repeat_remaining != -1)
		      --repeat_remaining;
		    else
		      {
			uint8 lsb;

			lsb = reloc_instrs[0][1];
			repeat_remaining = lsb + 1;
		      }
		    if (repeat_remaining > 0)
		      {
			int blockCount;

			blockCount = (msb & 0xF) + 2;
			reloc_count  += blockCount;
			reloc_instrs -= blockCount;
		      }
		    else
		      repeat_remaining = -1;
		  }
		  break;
		default:
		  switch (msb >> 2)
		    {
		    case 0x28:
		      {
			uint32 offset;

			offset = ((msb & 3) << 24 |
				  (reloc_instrs[0][1]) << 16 |
				  (reloc_instrs[0][2]) << 8 |
				  (reloc_instrs[0][3]));

			relocAddress
			  = (uint8 *)
			  SYN68K_TO_US (connp->sects[section].start) + offset;
		      }
		      --reloc_count;
		      ++reloc_instrs;
		      break;
		    case 0x29:
		      {
			Ptr symbol_val;

			importIndex = ((msb & 3) << 24 |
				       (reloc_instrs[0][1]) << 16 |
				       (reloc_instrs[0][2]) << 8 |
				       (reloc_instrs[0][3]));
			retval = symbol_lookup (&importIndex, &symbol_val,
						imports, symbol_names,
						closure_id);
			if (retval == noErr)
			  repeatedly_relocate (1, &relocAddress,
					       (uint32) symbol_val);
			

		      }
		      --reloc_count;
		      ++reloc_instrs;
		      break;
		    case 0x2c:
		      fprintf (stderr, "RelocLgRepeat\n");
		      assert (0);
		      --reloc_count;
		      ++reloc_instrs;
		      break;
		    case 0x2d:
		      fprintf (stderr, "RelocLgSetOrBySection\n");
		      assert (0);
		      --reloc_count;
		      ++reloc_instrs;
		      break;
		    default:
		      warning_unexpected ("0x%x", msb);
		      retval = paramErr;
		      break;
		    }
		}
	    }
	}
      ++reloc_instrs;
    }
  return retval;
}

PRIVATE CFragClosureID
begin_closure (uint32 n_libs, PEFImportedLibrary_t *libs,
	       const char *symbol_names, OSType arch)
{
  CFragClosureID retval;
  int i;
  OSErr err;

  retval = (typeof (retval)) NewPtr (sizeof *retval + n_libs * sizeof (lib_t));
  N_LIBS_X (retval) = CL (n_libs);

#warning eventually need to worry about errors  

  for (err = noErr, i = 0; /* err == noErr && */ i < n_libs; ++i)
    {
      Str63 libName;
      Ptr mainAddr;
      Str255 errName;
      int offset;
      const char *cname;

      offset = PEFIL_NAME_OFFSET (&libs[i]);
      cname = symbol_names + offset;
      libName[0] = MIN(strlen (cname), 63);
      memcpy (libName+1, cname, libName[0]);
      err = GetSharedLibrary (libName, arch, kReferenceCFrag,
			      &LIB_CID_X (&retval->libs[i]),
			      &mainAddr, errName);
      if (err != noErr)
	{
	  warning_unexpected ("%.*s", libName[0], libName+1);
	  LIB_CID_X (&retval->libs[i]) = (void *) 0x12348765;
	}
      LIB_N_SYMBOLS_X (&retval->libs[i]) = PEFIL_SYMBOL_COUNT_X (&libs[i]);
      LIB_FIRST_SYMBOL_X (&retval->libs[i]) = PEFIL_FIRST_SYMBOL_X (&libs[i]);
    }
  return retval;
}

PRIVATE OSErr
load_loader_section (const void *addr,
		     syn68k_addr_t default_address, uint32 total_size,
		     uint32 packed_size, uint32 unpacked_size,
		     uint32 section_offset, share_kind_t share_kind,
		     int alignment, syn68k_addr_t *mainAddrp, OSType arch,
		     ConnectionID connp)
{
  OSErr retval;
  char *loader_section_bytes;
  PEFLoaderInfoHeader_t *lihp;
  uint32 n_libs;
  uint32 n_imports;
  uint32 n_reloc_headers;
  PEFImportedLibrary_t *libs;
  uint8 (*imports)[4];
  PEFLoaderRelocationHeader_t *reloc_headers;
  uint8 *relocation_area;
  char *symbol_names;
  int i;
  CFragClosureID closure_id;

  loader_section_bytes = (char *)addr + section_offset;
  lihp = (PEFLoaderInfoHeader_t *) loader_section_bytes;
  connp->lihp = lihp;
  n_libs = PEFLIH_IMPORTED_LIBRARY_COUNT (lihp);
  libs = (PEFImportedLibrary_t *) &lihp[1];
  n_imports = PEFLIH_IMPORTED_SYMBOL_COUNT (lihp);
  imports = (uint8 (*)[4])&libs[n_libs];
  n_reloc_headers = PEFLIH_RELOC_SECTION_COUNT (lihp);
  reloc_headers = (PEFLoaderRelocationHeader_t *) imports[n_imports];
  
  relocation_area = (char *)
    (loader_section_bytes + PEFLIH_RELOC_INSTR_OFFSET (lihp));

  symbol_names = (typeof (symbol_names))
    (loader_section_bytes + PEFLIH_STRINGS_OFFSET (lihp));

  closure_id = begin_closure (n_libs, libs, symbol_names, arch);

  for (i = 0, retval = noErr; retval == noErr && i < n_reloc_headers; ++i)
    {
      uint32 reloc_count;
      uint8 (*reloc_instrs)[2];

      reloc_count = PEFRLH_RELOC_COUNT (&reloc_headers[i]);
      reloc_instrs = (typeof (reloc_instrs))
	(relocation_area
	 + PEFRLH_FIRST_RELOC_OFFSET (&reloc_headers[i]));
      retval = relocate (reloc_headers,
			 PEFRLH_SECTION_INDEX (&reloc_headers[i]),
			 reloc_count,
			 reloc_instrs, imports, symbol_names,
			 closure_id, connp);
    }
  if (retval == noErr && lihp->initSection != 0xffffffff)
    {
      uint32 *init_addr;
      uint32 init_toc;
      uint32 (*init_routine) (uint32);
      InitBlock init_block;

      warning_unimplemented ("register preservation, bad init_block");
      // #warning this code has a lot of problems (register preservation, bad init_block)

      init_addr = (uint32 *) SYN68K_TO_US
	(connp->sects[PEFLIH_INIT_SECTION (lihp)].start +
	 PEFLIH_INIT_OFFSET (lihp));

      memset (&init_block, 0xFA, sizeof init_block);
      init_routine = (uint32 (*)(uint32)) SYN68K_TO_US (init_addr[0]);
      init_toc = (uint32) SYN68K_TO_US (init_addr[1]);
#if defined (powerpc) || defined (__ppc__)
      retval = ppc_call (init_toc, init_routine, (uint32) &init_block);
#else
      warning_unexpected (NULL_STRING);
      retval = paramErr;
#endif
      if (retval)
	{
	  warning_unexpected ("%d", retval);
	  retval = noErr;
	}
    }
  if (retval == noErr)
    *mainAddrp = (connp->sects[PEFLIH_MAIN_SECTION (lihp)].start +
		  PEFLIH_MAIN_OFFSET (lihp));
  return retval;
}

PRIVATE OSErr
do_pef_section (ConnectionID connp, const void *addr,
		const PEFSectionHeader_t *sections, int i,
		bool instantiate_p,
		syn68k_addr_t *mainAddrp, OSType arch)
{
  OSErr retval;
  const PEFSectionHeader_t *shp;
  syn68k_addr_t default_address;
  uint32 total_size;
  uint32 packed_size;
  uint32 unpacked_size;
  uint32 section_offset;
  int share_kind;
  int alignment;

  shp = &sections[i];

#if 1
  {
    uint32 def;

    def = PEFSH_DEFAULT_ADDRESS (shp);
    if (def)
      fprintf (stderr, "***def = 0x%x***\n", def);
  }
#endif

#if 0
  default_address = PEFSH_DEFAULT_ADDRESS (shp);
#else
  default_address = 0;
// #warning defaultAddress ignored -- dont implement without testing
#endif
  total_size = PEFSH_TOTAL_SIZE (shp);
  packed_size = PEFSH_PACKED_SIZE (shp);
  unpacked_size = PEFSH_UNPACKED_SIZE (shp);
  section_offset = PEFSH_CONTAINER_OFFSET (shp);
  share_kind = PEFSH_SHARE_KIND (shp);
  alignment = PEFSH_ALIGNMENT (shp);

  switch (PEFSH_SECTION_KIND (shp))
    {
    case code_section_type:
      connp->sects[i].perms = readable_section | executable_section;
      goto unpacked_common;

    case unpacked_data_section_type:
      connp->sects[i].perms = readable_section | writable_section;
      goto unpacked_common;

    case constant_section_type:
      connp->sects[i].perms = readable_section;
      goto unpacked_common;

    case executable_data_section_type:
      connp->sects[i].perms = (readable_section | writable_section |
			       executable_section);
      goto unpacked_common;

unpacked_common:
      retval = load_unpacked_section (addr, default_address, total_size,
				      packed_size, unpacked_size,
				      section_offset, share_kind, alignment, 
				      &connp->sects[i]);
      break;
    case pattern_data_section_type:
      connp->sects[i].perms = readable_section | writable_section;
      retval = load_pattern_section (addr, default_address, total_size,
				     packed_size, unpacked_size,
				     section_offset, share_kind, alignment,
				     &connp->sects[i]);
      break;
    case loader_section_type:
      retval = load_loader_section (addr, default_address, total_size,
				    packed_size, unpacked_size,
				    section_offset, share_kind, alignment,
				    mainAddrp, arch, connp);
      break;
    default:
      warning_unexpected ("%d", PEFSH_SECTION_KIND (shp));
      retval = noErr;
      break;
    }

  return retval;
}

/*
 * NOTE: it would be nice if someone else provided code to flush the
 * instruction cache.  I'm a little nervous that my code below will fail on 
 * multi-processor systems.
 */

typedef enum { ICACHE } flush_type_t;

PRIVATE void
cacheflush (void *start, uint32 length, flush_type_t flush)
{
#if defined (powerpc) || defined (__ppc__)
  enum { CACHE_LINE_SIZE = 32, };
  char *p, *ep; 

  switch (flush)
    {
    case ICACHE:
      for (p = start, ep = p + roundup (length, CACHE_LINE_SIZE);
	   p != ep;
	   p += CACHE_LINE_SIZE)
	{
	  asm volatile ("dcbf 0,%0" : : "r" (p) : "memory");
	  asm volatile ("sync" : : : "memory");
	  asm volatile ("icbi 0,%0" : : "r"(p) : "memory");
	}
      asm volatile ("isync" : : : "memory");
      break;
    default:
      warning_unexpected ("%d", flush);
      break;
    }
#endif
}

PRIVATE OSErr
do_pef_sections (ConnectionID connp, const PEFContainerHeader_t *headp,
		 syn68k_addr_t *mainAddrp, OSType arch)
{
  OSErr retval;
  PEFSectionHeader_t *sections;
  int n_sects;
  int i;

  n_sects = connp->n_sects;
  sections = (typeof (sections)) ((char *) headp + sizeof *headp);

  memset (connp->sects, 0, sizeof connp->sects[0] * n_sects);
  for (i = 0, retval = noErr; retval == noErr && i < n_sects; ++i)
    retval = do_pef_section (connp, headp, sections, i,
			     i < PEF_CONTAINER_INSTSECTION_COUNT(headp),
			     mainAddrp, arch);
// #warning need to back out cleanly if a section fails to load
  
#if defined (linux)
  if (retval == noErr)
    {
      int i;

      for (i = 0; i < n_sects; ++i)
	{
	  if (connp->sects[i].length)
	    {
	      int prot;

	      prot = 0;
	      if (connp->sects[i].perms & executable_section)
		{
		  cacheflush (SYN68K_TO_US (connp->sects[i].start),
			      connp->sects[i].length, ICACHE);
		  prot |= PROT_EXEC;
		}
	      if (connp->sects[i].perms & readable_section)
		prot |= PROT_READ;
	      if (connp->sects[i].perms & writable_section)
		prot |= PROT_WRITE;
	      if (!prot)
		prot |= PROT_NONE;
	      if (mprotect (SYN68K_TO_US (connp->sects[i].start),
			    roundup (connp->sects[i].length, getpagesize ()),
			    prot) != 0)
		warning_unexpected ("%d", errno);
	    }
	}
    }
#endif

  return retval;
}

PUBLIC ConnectionID
ROMlib_new_connection (uint32 n_sects)
{
  ConnectionID retval;
  Size n_bytes;

  n_bytes = sizeof *retval + n_sects * sizeof (section_info_t);
  retval = (ConnectionID) NewPtrSysClear (n_bytes);
  if (retval)
    retval->n_sects = n_sects;
  return retval;
}

P7 (PUBLIC pascal trap, OSErr, GetMemFragment, void *, addr,
    uint32, length, Str63, fragname, LoadFlags, flags,
    ConnectionID *, connp, Ptr *, mainAddrp, Str255, errname)
{
  OSErr retval;

  syn68k_addr_t main_addr;
  PEFContainerHeader_t *headp;

  warning_unimplemented ("ignoring flags = 0x%x\n", flags);

  main_addr = 0;
  *connp = 0;

  headp = addr;

  if (PEF_CONTAINER_TAG1_X(headp) != CLC (T('J','o','y','!')))
    warning_unexpected ("0x%x", PEF_CONTAINER_TAG1 (headp));

  if (PEF_CONTAINER_TAG2_X(headp) != CLC (T('p','e','f','f')))
    warning_unexpected ("0x%x", PEF_CONTAINER_TAG2 (headp));

  if (PEF_CONTAINER_ARCHITECTURE_X(headp)
      != CLC (T('p','w','p','c')))
    warning_unexpected ("0x%x",
			PEF_CONTAINER_ARCHITECTURE (headp));

  if (PEF_CONTAINER_FORMAT_VERSION_X(headp) != CLC (1))
    warning_unexpected ("0x%x",
			PEF_CONTAINER_FORMAT_VERSION (headp));

// #warning ignoring (old_dev, old_imp, current) version

  *connp = ROMlib_new_connection (PEF_CONTAINER_SECTION_COUNT (headp));
  if (!*connp)
    retval = fragNoMem;
  else
    retval = do_pef_sections (*connp, headp, &main_addr,
			      PEF_CONTAINER_ARCHITECTURE (headp));

  if (retval == noErr)
    *mainAddrp = (Ptr) SYN68K_TO_US (main_addr);

  return retval;
}

typedef struct
{
  void *addr;     /* virtual address */
  FSSpec fs;     /* canonicalized file tht this came from */
  LONGINT offset_req;
  LONGINT length_req;
  off_t offset_act;
  size_t length_act;
  bool mapped_to_eof_p;
  int refcount;
}
context_t;

PRIVATE int n_context_slots = 0;
PRIVATE int n_active_contexts = 0;
PRIVATE context_t *contexts = 0;

PRIVATE bool
fsmatch (FSSpecPtr fsp1, FSSpecPtr fsp2)
{
  bool retval;

  retval = (fsp1->vRefNum == fsp2->vRefNum &&
	    fsp1->parID == fsp2->parID &&
	    EqualString (fsp1->name, fsp2->name, false, true));
  return retval;
}

PRIVATE bool
match (FSSpecPtr fsp, LONGINT offset, LONGINT length, const context_t *cp)
{
  bool retval;

  retval = (cp->refcount > 0 &&
	    fsmatch (fsp, (FSSpecPtr) &cp->fs) &&
	    offset >= cp->offset_act &&
	    ((length == kWholeFork && cp->mapped_to_eof_p) ||
	     (offset + length <= cp->offset_act + cp->length_act)));

  return retval;
}

PRIVATE OSErr
get_context (int *contextidp, bool *exists_pp, FSSpecPtr fsp,
	     LONGINT offset, LONGINT length)
{
  PRIVATE OSErr retval;
  int i;
  int free_slot;

  retval = noErr;
  free_slot = -1;
  for (i = 0;
       i < n_context_slots && !match (fsp, offset, length, &contexts[i]);
       ++i)
    if (contexts[i].refcount == 0)
      free_slot = i;
  if (i < n_context_slots)
    {
      ++contexts[i].refcount;
      *contextidp = i;
      *exists_pp = true;
    }
  else
    {
      if (free_slot == -1)
	{
	  size_t new_size;
	  context_t *new_contexts;

	  if (n_context_slots != n_active_contexts)
	    warning_unexpected ("%d %d", n_context_slots, n_active_contexts);
	  new_size = sizeof *new_contexts * (n_active_contexts + 1);
	  new_contexts = realloc (contexts, new_size);
	  if (new_contexts == NULL)
	      retval = memFullErr;
	  else
	    {
	      free_slot = n_active_contexts;
	      ++n_active_contexts;
	      ++n_context_slots;
	      contexts = new_contexts;
	    }
	}
      if (retval == noErr)
	{
	  context_t *cp;

	  cp = &contexts[free_slot];
	  cp->fs = *fsp;
	  cp->offset_req = offset;
	  cp->length_req = length;
	  cp->refcount = 1;
	  *contextidp = free_slot;
	  *exists_pp = false;
	}
    }

  return retval;
}

PRIVATE OSErr
release_context (int context)
{
  OSErr retval;

  if (context >= n_context_slots || contexts[context].refcount <= 0)
    retval = paramErr;
  else
    {
      retval = noErr;
      if (--contexts[context].refcount == 0)
	{
	  --n_active_contexts;
	  munmap (contexts[context].addr, contexts[context].length_act);
	}
    }
  return retval;
}

PRIVATE context_t *
contextp_from_id (int context)
{
  context_t *retval;

  if (context >= n_context_slots || contexts[context].refcount <= 0)
    retval = 0;
  else
    retval = &contexts[context];
  return retval;
}

PRIVATE OSErr
try_to_mmap_file (FSSpecPtr fsp, LONGINT offset, LONGINT length,
		  int *contextidp)
{
  OSErr retval;
  INTEGER vref;

  retval = noErr;
  /* canonicalize fsp to make sure */
  vref = CW (fsp->vRefNum);
  if (ISWDNUM (vref) || pstr_index_after (fsp->name, ':', 0))
    {
      FSSpecPtr newfsp;

      newfsp = alloca (sizeof *newfsp);
      retval = FSMakeFSSpec (vref, CL (fsp->parID), fsp->name, newfsp);
      if (retval == noErr)
	fsp = newfsp;
    }

  if (retval == noErr)
    {
      bool exists_p;
      int context;

      retval = get_context (&context, &exists_p, fsp, offset, length);
      if (retval == noErr)
	{
	  if (exists_p)
	    *contextidp = context;
	  else
	    {
	      INTEGER rn;

	      retval = FSpOpenDF (fsp, fsRdPerm, &rn);
	      if (retval != noErr)
		release_context (context);
	      else
		{
		  const fcbrec *fp;
		  context_t *cp;
		  size_t pagesize;

		  pagesize = getpagesize ();
		  fp = (const fcbrec *) (MR(FCBSPtr) + rn);
		  /* NOTE: right now we let them place it anywhere they
		     want -- eventually it probably makes sense to try to
		     get it placed high */
		  cp = contextp_from_id (context);
		  cp->offset_act = rounddown (offset, pagesize);
		  cp->mapped_to_eof_p = length == kWholeFork;
		  if (!cp->mapped_to_eof_p)
		    cp->length_act = length + (offset - cp->offset_act);
		  else
		    {
		      LONGINT eof;
		      OSErr err;

		      err = GetEOF (rn, &eof);
		      if (err == noErr)
			cp->length_act = eof - cp->offset_act;
		      else
			{
			  warning_unexpected ("err = %d", err);
			  cp->length_act = -1; /* force mmap to fail */
			}
		    }
		  cp->addr = mmap (0, cp->length_act, PROT_READ, MAP_PRIVATE,
				   fp->fcfd, cp->offset_act);
		  if (cp->addr != MAP_FAILED)
		    *contextidp = context;
		  else
		    {
		      retval = ROMlib_maperrno ();
		      release_context (context);
		    }
		  FSClose (rn);
		}
	    }
	}
    }

  return retval;

}

P8 (PUBLIC pascal trap, OSErr, GetDiskFragment, FSSpecPtr, fsp,
    LONGINT, offset, LONGINT, length, Str63, fragname, LoadFlags, flags,
		 ConnectionID *, connp, Ptr *, mainAddrp, Str255, errname)
{
  OSErr retval;
  int context;

  warning_unimplemented ("ignoring flags = 0x%x\n", flags);

  retval = try_to_mmap_file (fsp, offset, length, &context);
  if (retval == noErr)
    {
      const context_t *cp;
      void *addr;

      cp = contextp_from_id (context);

      addr = (char *) cp->addr + offset - cp->offset_act;
      retval = GetMemFragment (addr, length, fragname, flags, connp,
			       mainAddrp, errname);
    }
  return retval;
}

#endif
