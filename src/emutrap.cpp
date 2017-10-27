/* Copyright 1994 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_emutrap[] =
		"$Id: emutrap.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"

#include <stdarg.h>
#include "rsys/trapglue.h"
#include "rsys/mixed_mode.h"

using namespace Executor;

/*
 * NOTE: the code below is "mostly" portable.  It relies on all arguments
 *	 being passed in the same size (we don't ever pass doubles or
 *	 long longs) and it also relies on the ability to pick up a return
 *	 value as a LONGINT, even if it is a pointer return value.  The latter
 *	 definitely wouldn't work with some early eighties C compilers, but
 *	 to heck with them.
 */

/*
 * The first time this code was running on a PowerPC was on an old
 * version of Yellow Dog Linux.  The calling conventions were
 * different than what we get when compiling on Mac OS X.  As such,
 * there are some places where we check for the preprocessing macro
 * "powerpc" and are really concerned with the old calling convention.
 * There are othe times when we check for either powerpc or __ppc__ in
 * which case the calling convention shouldn't matter.
 *
 * TODO: switch to a modern version of the GNU build system and use it
 *       to determine calling convention issues.
 *
 * The Yellow Dog version was actually able to run a few PPC binaries,
 * including (IIRC) the demo versions of a couple of Adobe products
 * (Photoshop and Illustrator?).  However, none of the code fragment
 * manager or mixed-mode code has been tested in years and never under
 * Mac OS X.  In the incredibly unlikely circumstance that someone
 * wants to play with that code and has questions, I'll be happy to
 * answer them to the best of my ability, but it was a long time ago
 * and was short-lived, so my memory is a bit weak.
 *
 */

#define SYN68K_TO_US_CHECK0_CHECKNEG1(addr)			\
({								\
  syn68k_addr_t __t;						\
								\
  __t = addr;							\
  (__t == (syn68k_addr_t)-1) ? (uint16 *) -1 : SYN68K_TO_US_CHECK0(__t);	\
})

#define US_TO_SYN68K_CHECK0_CHECKNEG1(addr)			\
({								\
  void* __t;						        \
								\
  __t = addr;							\
  (__t == (void*)-1) ? (syn68k_addr_t) -1 : US_TO_SYN68K_CHECK0(__t);	\
})

PUBLIC syn68k_addr_t Executor::PascalToCCall(syn68k_addr_t ignoreme, ptocblock_t *infop)
{
    unsigned short pth, ptv;
    long args[11], retval;
#if defined (powerpc)
    Point points[11];
    int point_count = 0;
#endif
    unsigned long long magic;
    int count, rettype;
    void *funcp;
    int sizeflag;
    typedef long (*func0argsp_t)(void);
    typedef long (*func1argsp_t)(long);
    typedef long (*func2argsp_t)(long,long);
    typedef long (*func3argsp_t)(long,long,long);
    typedef long (*func4argsp_t)(long,long,long,long);
    typedef long (*func5argsp_t)(long,long,long,long,long);
    typedef long (*func6argsp_t)(long,long,long,long,long,long);
    typedef long (*func7argsp_t)(long,long,long,long,long,long,long);
    typedef long (*func8argsp_t)(long,long,long,long,long,long,long,long);
    typedef long (*func9argsp_t)(long,long,long,long,long,long,long,long,long);
    typedef long (*func10argsp_t)(long,long,long,long,long,long,long,long,long,long);
    typedef long (*func11argsp_t)(long,long,long,long,long,long,long,long,long,long,long);
    syn68k_addr_t retaddr;

    count = 0;

    retaddr = POPADDR();
    switch (infop->magic)
      {
      case PTOC_A10_MAGIC:
	magic = PTOC_SPECIAL_A10_VALUE;
	break;
      case PTOC_A11_MAGIC:
	magic = PTOC_SPECIAL_A11_VALUE;
	break;
      default:
	magic = infop->magic;
	break;
      }

    rettype = magic & 7;
    magic >>= 3;
    while ((sizeflag = magic & 7)) {
	switch(sizeflag) {
	case 1:
	    args[count++] = POPSB();
	    break;
	case 2:
	    args[count++] = POPSW();
	    break;
	case 3:
	    ptv = POPSW();
	    pth = POPSW();
#if !defined(LITTLEENDIAN)
#if !defined (powerpc)
	    args[count++] = (ptv << 16) | pth;
#else
	    points[point_count].h = pth;
	    points[point_count].v = ptv;
	    args[count++] = (long) &points[point_count];
	    ++point_count;
#endif
#else /* defined(LITTLEENDIAN) */
	    args[count++] = (pth << 16) | ptv;
#endif /* defined(LITTLEENDIAN) */
	    break;
	case 4:
	    args[count++] = POPSL();
	    break;
	case 5:
	    args[count++] = (long) SYN68K_TO_US_CHECK0_CHECKNEG1(POPSL());
	  break;
	}
	magic >>= 3;
    }

    funcp = infop->wheretogo;
    switch (count) {
    case 0:
	retval = (*(func0argsp_t)funcp)();
	break;
    case 1:
	retval = (*(func1argsp_t)funcp)(args[0]);
	break;
    case 2:
	retval = (*(func2argsp_t)funcp)(args[1], args[0]);
	break;
    case 3:
	retval = (*(func3argsp_t)funcp)(args[2], args[1], args[0]);
	break;
    case 4:
	retval = (*(func4argsp_t)funcp)(args[3], args[2], args[1], args[0]);
	break;
    case 5:
	retval = (*(func5argsp_t)funcp)(args[4], args[3], args[2], args[1],
								      args[0]);
	break;
    case 6:
	retval = (*(func6argsp_t)funcp)(args[5], args[4], args[3], args[2],
							     args[1], args[0]);
	break;
    case 7:
	retval = (*(func7argsp_t)funcp)(args[6], args[5], args[4], args[3],
						    args[2], args[1], args[0]);
	break;
    case 8:
	retval = (*(func8argsp_t)funcp)(args[7], args[6], args[5], args[4],
					   args[3], args[2], args[1], args[0]);
	break;
    case 9:
	retval = (*(func9argsp_t)funcp)(args[8], args[7], args[6], args[5],
			          args[4], args[3], args[2], args[1], args[0]);
	break;
    case 10:
	retval = (*(func10argsp_t)funcp)(args[9], args[8], args[7], args[6],
					 args[5], args[4], args[3], args[2],
					 args[1], args[0]);
	break;
    case 11:
	retval = (*(func11argsp_t)funcp)(args[10], args[9], args[8], args[7],
					 args[6],  args[5], args[4], args[3],
					 args[2], args[1], args[0]);
	break;
#if !defined(LETGCCWAIL)
    default:
        retval = 0;
#endif
    }

    switch (rettype)
      {
      case 1:
	WRITEUW(EM_A7, 0);	/* needed for WordPerfect */
	WRITEUB(EM_A7, retval);
	break;
      case 2:
	WRITEUW(EM_A7, retval);
	break;
      case 4:
	WRITEUL(EM_A7, retval);
	break;
      case 5:
	WRITEUL(EM_A7, US_TO_SYN68K_CHECK0_CHECKNEG1((void*)retval));
	break;
      }
    return retaddr;
}

PRIVATE long
CToPascalCall_m68k(void *wheretogo, unsigned long long magic, va_list ap)
{
    long retval;
    unsigned long ul;
    int retvaltype;
    M68kReg saveregs[14];
  
    memcpy (saveregs, &EM_D1, sizeof(saveregs));	/* d1-d7/a0-a6 */

    retvaltype = magic & 7;
    switch(retvaltype) {
    case 1:
    case 2:
	PUSHUW(0);
	break;
    case 4:
    case 5:
	PUSHUL(0);
	break;
    }
    magic >>= 3;
    while (magic) {
	switch (magic & 7) {
	case 1:
	    PUSHUB((unsigned char) va_arg(ap, unsigned long));
	    break;
	case 2:
 	    PUSHUW((unsigned short) va_arg(ap, unsigned long));
	    break;
	case 3:
	    ul = va_arg(ap, unsigned long);
#if !defined(LITTLEENDIAN)
#if !defined (powerpc)
	    PUSHUW(ul);
	    PUSHUW(ul >> 16);
#else
	    {
	      unsigned long new_ul;

	      new_ul = *(unsigned long *)ul;
	      PUSHUW (new_ul);
	      PUSHUW (new_ul >> 16);
	    }
#endif
#else /* defined(LITTLEENDIAN) */
	    PUSHUW(ul >> 16);
	    PUSHUW(ul);
#endif /* defined(LITTLEENDIAN) */
	    break;
	case 4:
	    PUSHUL(va_arg(ap, unsigned long));
	    break;
	case 5:
	    PUSHUL(US_TO_SYN68K_CHECK0_CHECKNEG1((void*) va_arg(ap, unsigned long)));
	    break;
	}
	magic >>= 3;
    }
    va_end(ap);

    CALL_EMULATOR(US_TO_SYN68K(wheretogo));

    switch (retvaltype) {
    case 0:
	retval = 0;
	break;
    case 1:
	retval = POPSB();
	break;
    case 2:
	retval = POPSW();
	break;
    case 4:
	retval = POPSL();
	break;
    case 5:
	retval = (unsigned long) SYN68K_TO_US_CHECK0_CHECKNEG1(POPSL());
	break;
#if !defined(LETGCCWAIL)
    default:
        retval = 0;
        break;
#endif
    }
    memcpy(&EM_D1, saveregs, sizeof(saveregs));	/* d1-d7/a0-a6 */
    return retval;
}

#if defined (powerpc) || defined (__ppc__)

PRIVATE long
CToRoutineDescriptorCall (const RoutineDescriptor *p, unsigned long long magic,
			  va_list ap)
{
  uint32 args[11];
  int n_args;
  ProcInfoType procinfo;
  UniversalProcPtr up;
  uint32 *argsp;
  long retval;
  int retvaltype;

  procinfo = kCStackBased;

  argsp = args;
  up = (UniversalProcPtr) p;
  n_args = 0;
  retvaltype = magic & 7;

  switch (retvaltype) /* procinfo to take return value into consideration */
    {
    case 0:
      procinfo |= RESULT_SIZE (kNoByteCode);
      break;
    case 1:
      procinfo |= RESULT_SIZE (kOneByteCode);
      break;
    case 2:
      procinfo |= RESULT_SIZE (kTwoByteCode);
      break;
    case 3:
    case 4:
    case 5:
      procinfo |= RESULT_SIZE (kFourByteCode);
      break;
    }

  /* we can use this while loop because we don't need to know the return
     type, so we don't need the first 3 bits, and each time through the
     argument type is another 3 bits */

  while (magic >>= 3)
    {
      uint32 arg;

      ++n_args;
      switch (magic & 7)
	{
	case 1:
	  {
	    arg = (uint8) va_arg (ap, unsigned long);
	    arg = CB (arg);
	    procinfo |= STACK_ROUTINE_PARAMETER (n_args, kOneByteCode);
	  }
	  break;
	case 2:
	  {
	    arg = (uint16) va_arg (ap, unsigned long);
	    arg = CW (arg);
	    procinfo |= STACK_ROUTINE_PARAMETER (n_args, kTwoByteCode);
	  }
	  break;
	case 3: /* point */
	  {
	    arg = (uint32) va_arg (ap, unsigned long);
#if defined (powerpc)
	    arg = *(uint32 *)arg;
#endif
	    arg = (CW ((uint16) arg) | 
		   (CW (arg >> 16) << 16));
	    procinfo |= STACK_ROUTINE_PARAMETER (n_args, kFourByteCode);
	  }
	  break;
	case 4:
	  {
	    arg = (uint32) va_arg (ap, unsigned long);
	    arg = CL (arg);
	    procinfo |= STACK_ROUTINE_PARAMETER (n_args, kFourByteCode);
	  }
	  break;
	case 5:
	  {
	    arg = (uint32) va_arg (ap, unsigned long);
	    arg = (uint32) SYN68K_TO_US_CHECK0_CHECKNEG1 (arg);
	    procinfo |= STACK_ROUTINE_PARAMETER (n_args, kFourByteCode);
	  }
	  break;
	default:
	  warning_unexpected ("%d", (int) magic & 7);
	  arg = 0;
	  break;
	}
      *argsp++ = arg;
    }

  switch (n_args)
    {
    case 0:
      retval = CallUniversalProc_from_native (up, procinfo);
      break;
    case 1:
      retval = CallUniversalProc_from_native (up, procinfo, args[0]);
      break;
    case 2:
      retval = CallUniversalProc_from_native (up, procinfo, args[0], args[1]);
      break;
    case 3:
      retval = CallUniversalProc_from_native (up, procinfo, args[0], args[1],
					      args[2]);
      break;
    case 4:
      retval = CallUniversalProc_from_native (up, procinfo, args[0], args[1],
					      args[2], args[3]);
      break;
    case 5:
      retval = CallUniversalProc_from_native (up, procinfo, args[0], args[1],
					      args[2], args[3], args[4]);
      break;
    case 6:
      retval = CallUniversalProc_from_native (up, procinfo, args[0], args[1],
					      args[2], args[3], args[4],
					      args[5]);
      break;
    case 7:
      retval = CallUniversalProc_from_native (up, procinfo, args[0], args[1],
					      args[2], args[3], args[4],
					      args[5], args[6]);
      break;
    case 8:
      retval = CallUniversalProc_from_native (up, procinfo, args[0], args[1],
					      args[2], args[3], args[4],
					      args[5], args[6], args[7]);
      break;
    case 9:
      retval = CallUniversalProc_from_native (up, procinfo, args[0], args[1],
					      args[2], args[3], args[4],
					      args[5], args[6], args[7],
					      args[8]);
      break;
    case 10:
      retval = CallUniversalProc_from_native (up, procinfo, args[0], args[1],
					      args[2], args[3], args[4],
					      args[5], args[6], args[7],
					      args[8], args[9]);
	break;
    case 11:
      retval = CallUniversalProc_from_native (up, procinfo, args[0], args[1],
					      args[2], args[3], args[4],
					      args[5], args[6], args[7],
					      args[8], args[9], args[10]);
      break;
#if !defined(LETGCCWAIL)
    default:
      retval = 0;
#endif
    }
  switch (retvaltype)
    {
    case 1:
      retval = CB (retval);
      break;
    case 2:
      retval = CW (retval);
      break;
    case 4:
      retval = CL (retval);
      break;
    case 5:
      retval = (long) SYN68K_TO_US_CHECK0_CHECKNEG1 (retval);
      break;
    }
  return retval;
}

PRIVATE boolean_t
is_routine_descriptor_ptr (uint16 *addr)
{
  boolean_t retval;

  retval = (*addr == (uint16) CWC (MIXED_MODE_TRAP));
  return retval;
}
#endif

PUBLIC long Executor::CToPascalCall(void *wheretogo, unsigned long magic_in, ...)
{
  va_list ap;
  LONGINT retval;
  unsigned long long magic;
  
  switch (magic_in)
    {
    case CTOP_A10_MAGIC:
      magic = CTOP_SPECIAL_A10_VALUE;
      break;
    case CTOP_A11_MAGIC:
      magic = CTOP_SPECIAL_A11_VALUE;
      break;
    default:
      magic = magic_in;
      break;
    }
  va_start(ap, magic_in);

#if defined (powerpc) || defined (__ppc__)
  if (is_routine_descriptor_ptr (wheretogo))
    retval = CToRoutineDescriptorCall ((RoutineDescriptor *) wheretogo,
				       magic, ap);
  else
#endif
    retval = CToPascalCall_m68k (wheretogo, magic, ap);
  va_end (ap);
  return retval;
}
