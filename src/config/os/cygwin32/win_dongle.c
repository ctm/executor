/* Copyright 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_win_dongle[] = "$Id: win_dongle.c 139 2006-07-11 23:35:04Z ctm $";
#endif

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include "rsys/common.h"
#include "rsys/error.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "win_dongle.h"
#include "win_sentpro.h"
#include "win_hasp.h"
#include "win_dll.h"

PRIVATE HINSTANCE sentinel_lib;
PRIVATE HINSTANCE hasp_lib;

#define DLL_DECLARE(x) PRIVATE decltype (x) *D ## x

DLL_DECLARE (RNBOproQuery);
DLL_DECLARE (RNBOproGetFullStatus);
DLL_DECLARE (RNBOproFormatPacket);
DLL_DECLARE (RNBOproInitialize);

extern void PASCAL hasp (uint32 service, uint32 seed, uint32 lptnum, uint32 pass1,
		  uint32 pass2, int32 *p1, int32 *p2, int32 *p3, int32 *p4);

DLL_DECLARE (hasp);

#define GETPROCADDRESS(lib, func)			\
do							\
{							\
  if (lib)						\
    {							\
      D ## func = (void *) GetProcAddress (lib, #func);	\
      if (!D ## func)					\
	{						\
	  FreeLibrary (lib);				\
	  lib = NULL;					\
	}						\
    }							\
}							\
while (0)

PRIVATE bool
load_sent (const char *dll_name)
{
  bool retval;

  sentinel_lib = LoadLibrary (dll_name);
  GETPROCADDRESS (sentinel_lib, RNBOproQuery);
  GETPROCADDRESS (sentinel_lib, RNBOproGetFullStatus);
  GETPROCADDRESS (sentinel_lib, RNBOproFormatPacket);
  GETPROCADDRESS (sentinel_lib, RNBOproInitialize);
  retval = !!sentinel_lib;
  return retval;
}

PRIVATE bool
load_hasp (const char *dll_name)
{
  bool retval;

  hasp_lib = LoadLibrary (dll_name);
  GETPROCADDRESS (hasp_lib, hasp);
  retval = !!hasp_lib;
  return retval;
}

PRIVATE PRO_STATUS
raw_query (RB_WORD *full_statusp, RB_WORD *resultp,
	   RB_PRO_APIPACKET *packetp, const char *string)
{
  PRO_STATUS retval;
  int len;
  char *retstring;
  RB_WORD word;

  len = strlen (string);
  retstring = alloca (len + 1);
  strcpy (retstring, string);
  retval =  DRNBOproQuery (packetp, retstring, &word, len);
  if (retval == SENTPRO_SUCCESS)
    {
      if (full_statusp)
	*full_statusp = 0;
      if (resultp)
	*resultp = word;
    }
  else
    {
      if (full_statusp)
	*full_statusp = DRNBOproGetFullStatus( packetp );
      if (resultp)
	*resultp = 0;
    }
  
  return retval;
}

enum
{
  FAMILY_COMMAND_PREFIX_0 = '3',
  FAMILY_COMMAND_PREFIX_1 = '4',
};

PRIVATE PRO_STATUS
family_query (const char *family, RB_WORD *full_statusp, RB_WORD *resultp,
	      RB_PRO_APIPACKET *packetp, const char *string)
{
  const char family_command_prefixes[] = { '3', '4' };
  static char current_family[2];
  int i;
  PRO_STATUS retval;

  retval = SENTPRO_SUCCESS;
  for (i = 0; (retval == SENTPRO_SUCCESS &&
	       i < (int) NELEM (family_command_prefixes)); ++i)
    {
#if 0 /* THIS BREAKS THINGS -- I HAVE NO IDEA WHY -- ctm */
      if (current_family[i] != family[i])
#endif
	{
	  char command[3];

	  command[0] = family_command_prefixes[i];
	  command[1] = family[i];
	  command[2] = 0;
	  retval = raw_query (full_statusp, resultp, packetp, command);
	  if (retval == SENTPRO_SUCCESS)
	    current_family[i] = family[i];
	}
    }

  if (retval == SENTPRO_SUCCESS)
    retval = raw_query (full_statusp, resultp, packetp, string);
  return retval;
}

PRIVATE int
sentinel_dongle_query (uint32 *valuep)
{
  int retval;

  if (!sentinel_lib) {
    warning_trace_info("about to load sentinel dll");
    load_sent (SENTINEL_DLL);
    warning_trace_info("loaded sentinel dll");
  }
  if (!sentinel_lib)
    retval = SENTPRO_DRIVER_NOT_INSTALLED;
  else
    {
      PRO_STATUS status;
      RB_PRO_APIPACKET packet;

      status = DRNBOproFormatPacket (&packet, sizeof packet);
      if (status == SENTPRO_SUCCESS)
	{
	  status  = DRNBOproInitialize (&packet);
	  if (status == SENTPRO_SUCCESS)
	    {
	      RB_WORD word;

	      status = family_query ((ROMlib_DongleFamily ?
				      ROMlib_DongleFamily :
				      SENTINEL_FAMILY), NULL, &word, &packet,
				     SENTINEL_STRING);
	      if (status == SENTPRO_SUCCESS)
		{
		  status = 0;
		  *valuep = CL ( (uint32) word);
		}
	    }
	}
      retval = status;
    }
  retval = -retval;
  return retval;
}

PRIVATE int
hasp_dongle_query (hasp_param_block *valuep)
{
  int retval;

  if (valuep->magic != CLC (HASP_MAGIC))
    retval = HASP_BAD_MAGIC_ERROR;
  else if (CL (valuep->size) < sizeof (hasp_param_block))
    retval = HASP_BAD_LENGTH_ERROR;
  else
    {
      if (!hasp_lib) {
	warning_trace_info("About to load hasp dll");
	load_hasp (HASP_DLL);
	warning_trace_info("hasp dll loaded");
      }
      if (!hasp_lib)
	retval = HASP_NO_LIBRARY_ERROR;
      else
	{
	  int32 service;
	  int32 seed;
	  int32 lptnum;
	  int32 pass1;
	  int32 pass2;
	  int32 p1;
	  int32 p2;
	  int32 p3;
	  int32 p4;

	  service = CL (valuep->Service);
	  seed    = CL (valuep->SeedCode);
	  lptnum  = CL (valuep->LptNum);
	  pass1   = CL (valuep->Password1);
	  pass2   = CL (valuep->Password2);
	  p1 = CL (valuep->Par1);
	  p2 = CL (valuep->Par2);
	  p3 = CL (valuep->Par3);
	  p4 = CL (valuep->Par4);
	  if (service == ReadBlock || service == WriteBlock)
	    p4 = (int32) SYN68K_TO_US (p4);


	  warning_trace_info("about to call Dhasp");
	  Dhasp (service, seed, lptnum, pass1, pass2, &p1, &p2, &p3, &p4);
	  warning_trace_info("still alive");
	  
	  switch (service)
	    {
	    case IsHasp:
	      valuep->Par1 = CL (p1);
	      valuep->Par3 = CL (p3);
	      break;
	    case HaspCode:
	    default:
	      valuep->Par1 = CL (p1);
	      valuep->Par2 = CL (p2);
	      valuep->Par3 = CL (p3);
	      valuep->Par4 = CL (p4);
	      break;
	    case HaspStatus:
	    case HaspID:
	      valuep->Par1 = CL (p1);
	      valuep->Par2 = CL (p2);
	      valuep->Par3 = CL (p3);
	      break;
	    case ReadWord:
	      valuep->Par2 = CL (p2);
	      valuep->Par3 = CL (p3);
	      break;
	    case WriteWord:
	    case ReadBlock:
	    case WriteBlock:
	      valuep->Par3 = CL (p3);
	      break;
	    }
	  retval = HASP_NO_ERROR;
	}
    }
  warning_trace_info("returning %d", retval);
  return retval;
}

PRIVATE int
dll_query (dll_param_block *dp)
{
  int retval;

  if (dp->magic != CLC (DLL_MAGIC))
    retval = DLL_BAD_MAGIC_ERROR;
  else if (CL (dp->size) < sizeof *dp)
    retval = DLL_BAD_LENGTH_ERROR;
  else
    {
      const char *dll_name;
      const char *func_name;
      void *arg;
      HINSTANCE dll_lib;
      void (*funcp) (void *arg);

      dll_name = MR (dp->dll_name);
      func_name = MR (dp->function_name);
      arg = MR (dp->arg_to_function);
      dll_lib = LoadLibrary (dll_name);
      if (!dll_lib)
	retval = DLL_NO_LIBRARY_ERROR;
      else
	{
	  funcp = (decltype (funcp)) GetProcAddress (dll_lib, func_name);
	  if (!funcp)
	    retval = DLL_NO_FUNCTION_ERROR;
	  else
	    {
	      funcp (arg);
	      retval = DLL_NO_ERROR;
	    }
	  FreeLibrary (dll_lib);
	}
    }

  return retval;
}

/*
 * "Dongle_query" now a misnomer, since this is also where we give Jim Pittman
 * access to the x86, but this can be cleaned up later.
 */

PUBLIC int
dongle_query (uint32 *valuep)
{
  int retval;

  switch (*valuep)
    {
    case CLC (HASP_TYPE):
      warning_trace_info("hasp type");
      retval = hasp_dongle_query ((hasp_param_block *) valuep);
      break;
      
    case CLC (DLL_TYPE):
      warning_trace_info("dll type");
      retval = dll_query ((dll_param_block *) valuep);
      break;
      
    default:
      warning_trace_info("default");
      retval = sentinel_dongle_query (valuep);
      break;
    }

  return retval;
}
