#if !defined (_PPC_H_)
#define _PPC_H_

/* Copyright 1996 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: PPC.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor {

typedef int16 PPCPortKinds;
typedef int16 PPCLocationKind;

typedef struct EntityName
{
  /* #### bogus */
} EntityName;


// ### Struct needs manual conversion to GUEST<...>
//   union
typedef struct PACKED LocationNameRec
{
  PPCLocationKind locationKindSelector;
  
  union
    {
      EntityName npbEntity;
      Str32 npbType;
  } u;
} LocationNameRec;



// ### Struct needs manual conversion to GUEST<...>
//   union
typedef struct PACKED PPCPortRec
{
  ScriptCode nameScript;
  Str32 name;
  
  PPCPortKinds portKindsSelector;
  
  union
  {
    Str32 portTypeStr;
    struct PACKED
    {
      OSType creator;
      OSType type;
    } port;
  } u;
} PPCPortRec, *PPCPortPtr;
}

#endif /* !_PPC_H_ */
