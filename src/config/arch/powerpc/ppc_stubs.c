/* Copyright 2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "ppc_stubs.h"

/*
 * Eventually we'll want a ppc stub for every routine that can be reached
 * from the interfacelib.  Most of them should be automatically generated,
 * but we're experimenting with hand-crafted ones to make sure we run into
 * all the issues before we automate.  These hand-crafted stubs used to
 * live in cfm.c, but I'm putting at least some into a separate file so
 * I can see if we will automatically get the right arguments for routines
 * with more arguments than will fit in r3-r10.
 */

PUBLIC void
SFPGetFile_AIX(PointAsLong pal, StringPtr prompt, ProcPtr filef, INTEGER numt,
               SFTypeList tl, ProcPtr dh, SFReply *rep, INTEGER dig,
               ProcPtr fp)
{

    sfpgetfile_aixtosysv4 sfp;

    sfp.dig = dig;
    sfp.fp = fp;
    warning_trace_info("> 8 args");
    SFPGetFile_SYSV4(pal, prompt, filef, numt, tl, dh, rep, &sfp);
}

PUBLIC OSErr
HMShowMenuBalloon_AIX(INTEGER item, INTEGER menuid, LONGINT flags,
                      LONGINT itemreserved, PointAsLong pal,
                      RectPtr alternaterectp, Ptr tipproc, INTEGER proc,
                      INTEGER variant)
{
    OSErr retval;
    hmshowmenuballoon_aixtosysv4 args;

    warning_trace_info("> 8 args");
    args.proc = proc;
    args.variant = variant;
    retval = HMShowMenuBalloon_SYSV4(item, menuid, flags, itemreserved,
                                     pal, alternaterectp, tipproc, &args);
    return retval;
}

PUBLIC OSErr
HMGetIndHelpMsg_AIX(ResType type, INTEGER resid, INTEGER msg, INTEGER state,
                    LONGINT *options, PointAsLong pal, Rect *altrectp,
                    INTEGER *theprocp, INTEGER *variantp,
                    HMMessageRecord *helpmsgp, INTEGER *count)
{
    OSErr retval;
    hmgetindhelpmsg_aixtosysv4 args;

    warning_trace_info("> 8 args");
    args.theprocp = theprocp;
    args.variantp = variantp;
    args.helpmsgp = helpmsgp;
    args.count = count;
    retval = HMGetIndHelpMsg_SYSV4(type, resid, msg, state,
                                   options, pal, altrectp, &args);
    return retval;
}

PUBLIC ControlHandle
NewControl_AIX(WindowPtr wst, Rect *r, StringPtr title, BOOLEAN vis,
               INTEGER value, INTEGER min, INTEGER max, INTEGER procid,
               LONGINT rc)
{
    ControlHandle retval;
    newcontrol_aixtosysv4 args;

    args.procid = procid;
    args.rc = rc;
    retval = NewControl_SYSV4(wst, r, title, vis,
                              value, min, max, &args);
    return retval;
}

PUBLIC CDialogPtr
NewCDialog_AIX(Ptr p, Rect *rp, StringPtr sp, BOOLEAN b1, INTEGER i,
               WindowPtr wp, BOOLEAN b2, LONGINT l, Handle h)
{
    CDialogPtr retval;
    newcdialog_aixtosysv4 args;

    args.l = l;
    args.h = h;
    warning_trace_info("> 8 args");
    retval = NewCDialog_SYSV4(p, rp, sp, b1,
                              i, wp, b2, &args);
    return retval;
}

PUBLIC DialogPtr
NewDialog_AIX(Ptr dst, Rect *r, StringPtr tit, BOOLEAN vis, INTEGER procid,
              WindowPtr behind, BOOLEAN gaflag, LONGINT rc, Handle items)
{
    DialogPtr retval;
    newdialog_aixtosysv4 args;

    warning_trace_info("> 8 args");
    args.rc = rc;
    args.items = items;
    retval = NewDialog_SYSV4(dst, r, tit, vis,
                             procid, behind, gaflag, &args);
    return retval;
}

PUBLIC OSErr
OutlineMetrics_AIX(int16 byte_count, Ptr text, PointAsLong numerAL,
                   PointAsLong denomAL, int16 *y_max, int16 *y_min,
                   Fixed *aw_array, Fixed *lsb_array, Rect *bounds_array)
{
    OSErr retval;
    outlinemetrics_aixtosysv4 args;

    warning_trace_info("> 8 args");
    args.lsb_array = lsb_array;
    args.bounds_array = bounds_array;
    retval = OutlineMetrics_SYSV4(byte_count, text, numerAL, denomAL,
                                  y_max, y_min, aw_array, &args);
    return retval;
}

PUBLIC ListHandle
LNew_AIX(Rect *rview, Rect *bounds, PointAsLong pal, INTEGER proc,
         WindowPtr wind, BOOLEAN draw, BOOLEAN grow, BOOLEAN scrollh,
         BOOLEAN scrollv)
{
    ListHandle retval;
    lnew_aixtosysv4 args;

    warning_trace_info("> 8 args");
    args.scrollh = scrollh;
    args.scrollv = scrollv;
    retval = LNew_SYSV4(rview, bounds, pal, proc,
                        wind, draw, grow, &args);
    return retval;
}

PUBLIC INTEGER
PixelToChar_AIX(Ptr textBuf, LONGINT textLen, Fixed slop, Fixed pixelWidth,
                BOOLEAN *leadingEdgep, Fixed *widthRemainingp,
                JustStyleCode styleRunPosition, PointAsLong numerAL,
                PointAsLong denomAL)
{
    INTEGER retval;
    pixeltochar_aixtosysv4 args;

    warning_trace_info("> 8 args");
    args.numer.h = numerAL;
    args.numer.v = numerAL >> 16;
    args.denom.h = denomAL;
    args.denom.v = denomAL >> 16;
    retval = PixelToChar_SYSV4(textBuf, textLen, slop, pixelWidth,
                               leadingEdgep, widthRemainingp, styleRunPosition,
                               &args);
    return retval;
}

PUBLIC void
CustomPutFile_AIX(Str255 prompt, Str255 defaultName,
                  StandardFileReply *replyp, INTEGER dlgid, PointAsLong pal,
                  DlgHookYDProcPtr dlghook, ModalFilterYDProcPtr filterproc,
                  Ptr activeList, ActivateYDProcPtr activateproc,
                  UNIV Ptr yourdatap)
{
    customputfile_aixtosysv4 args;

    warning_trace_info("> 8 args");
    args.activeList = activeList;
    args.activateproc = activateproc;
    args.yourdatap = yourdatap;
    CustomPutFile_SYSV4(prompt, defaultName, replyp, dlgid,
                        pal, dlghook, filterproc, &args);
}

PUBLIC void
CustomGetFile_AIX(FileFilterYDProcPtr filefilter, INTEGER numtypes,
                  SFTypeList typelist, StandardFileReply *replyp,
                  INTEGER dlgid, PointAsLong pal, DlgHookYDProcPtr dlghook,
                  ModalFilterYDProcPtr filterproc, Ptr activeList,
                  ActivateYDProcPtr activateproc, UNIV Ptr yourdatap)

{
    customgetfile_aixtosysv4 args;

    warning_trace_info("> 8 args");
    args.filterproc = filterproc;
    args.activeList = activeList;
    args.activateproc = activateproc;
    args.yourdatap = yourdatap;
    CustomGetFile_SYSV4(filefilter, numtypes, typelist, replyp,
                        dlgid, pal, dlghook, &args);
}
