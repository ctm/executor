#if !defined(__RSYS_MISC__)
#define __RSYS_MISC__

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: misc.h 63 2004-12-24 18:19:43Z ctm $
 */
#if 0
#if !defined(nilhandle_H)
extern GUEST<Ptr> nilhandle_H;
extern GUEST<Ptr> dodusesit_H;
extern LONGINT trapvectors;
extern LONGINT hyperlong;
extern LONGINT mathones;
extern INTEGER graphlooksat;
extern LONGINT macwritespace;
extern LONGINT LastSPExtra;
extern LONGINT lastlowglobal;
extern INTEGER MCLKPCmiss1;
extern INTEGER MCLKPCmiss2;
#endif

#define nilhandle (nilhandle_H.p)
#define dodusesit (dodusesit_H.p)
#endif

#endif /* !defined(__RSYS_MISC__) */
