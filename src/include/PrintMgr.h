#if !defined (__PRINTING__)
#define __PRINTING__

#include "DialogMgr.h"

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: PrintMgr.h 63 2004-12-24 18:19:43Z ctm $
 */

#define bDraftLoop	0
#define bSpoolLoop	1

#define bDevCItoh	1
#define bDevLaser	3

#define iPFMaxPgs	128

#define iPrSavPFil	(-1)
#define controlErr	(-17)
#define iIOAbort	(-27)
#define MemFullErr	(-108)
#define iPrAbort	128

#define iPrDevCtl	7
#define lPrReset	0x10000
#define lPrLineFeed	0x30000
#define lPrLFSixth	0x3FFFF
#define lPrPageEnd	0x20000
#define iPrBitsCtl	4
#define lScreenBits	0
#define lPaintBits	1
#define iPrIOCtl	5

#define sPrDrvr		".Print"
#define iPrDrvrRef	(-3)

typedef struct {
    GrafPort gPort	LPACKED;
    QDProcs saveprocs	LPACKED;
    LONGINT spare[4]	PACKED;
    BOOLEAN fOurPtr	LPACKED;
    BOOLEAN fOurBits	LPACKED;
} TPrPort;
typedef TPrPort *TPPrPort;

typedef struct {
    INTEGER iDev	PACKED;
    INTEGER iVRes	PACKED;
    INTEGER iHRes	PACKED;
    Rect rPage	LPACKED;
} TPrInfo;

typedef enum {feedCut, feedFanFold, feedMechCut, feedOther} TFeed;

typedef struct {
    INTEGER wDev	PACKED;
    INTEGER iPageV	PACKED;
    INTEGER iPageH	PACKED;
    SignedByte bPort	LPACKED;
    char feed	LPACKED;
} TPrStl;

typedef enum { scanTB,  scanBL,  scanLR,  scanRL } TScan;
typedef struct {
    INTEGER iRowBytes	PACKED;
    INTEGER iBandV	PACKED;
    INTEGER iBandH	PACKED;
    INTEGER iDevBytes	PACKED;
    INTEGER iBands	PACKED;
    SignedByte bPatScale	LPACKED;
    SignedByte bULThick	LPACKED;
    SignedByte bULOffset	LPACKED;
    SignedByte bULShadow	LPACKED;
    char scan	LPACKED;
    SignedByte bXInfoX	LPACKED;
} TPrXInfo;

typedef struct {
    INTEGER iFstPage	PACKED;
    INTEGER iLstPage	PACKED;
    INTEGER iCopies	PACKED;
    SignedByte bJDocLoop	LPACKED;
    BOOLEAN fFromUsr	LPACKED;
    ProcPtr pIdleProc	PACKED_P;
    StringPtr pFileName	PACKED_P;
    INTEGER iFileVol	PACKED;
    SignedByte bFileVers	LPACKED;
    SignedByte bJobX	LPACKED;
} TPrJob;

typedef struct {
    INTEGER iPrVersion	PACKED;
    TPrInfo prInfo	LPACKED;
    Rect rPaper	LPACKED;
    TPrStl prStl	LPACKED;
    TPrInfo prInfoPT	LPACKED;
    TPrXInfo prXInfo	LPACKED;
    TPrJob prJob	LPACKED;
    INTEGER printX[19]	PACKED;
} TPrint;
typedef TPrint *TPPrint;
typedef struct { TPPrint p PACKED_P; } HIDDEN_TPPrint;
typedef HIDDEN_TPPrint *THPrint;

typedef Rect *TPRect;

typedef struct {
    INTEGER iTotPages	PACKED;
    INTEGER iCurPage	PACKED;
    INTEGER iTotCopies	PACKED;
    INTEGER iCurCopy	PACKED;
    INTEGER iTotBands	PACKED;
    INTEGER iCurBand	PACKED;
    BOOLEAN fPgDirty	LPACKED;
    BOOLEAN fImaging	LPACKED;
    THPrint hPrint	PACKED_P;
    TPPrPort pPRPort	PACKED_P;
    PicHandle hPic	PACKED_P;
} TPrStatus;

typedef struct {
/* From Technote 095 */
    DialogRecord dlg	LPACKED;
    ProcPtr pFltrProc	PACKED_P;
    ProcPtr pItemProc	PACKED_P;
    THPrint hPrintUsr	PACKED_P;
    BOOLEAN fDoIt	LPACKED;
    BOOLEAN fDone	LPACKED;
    LONGINT lUser1	PACKED;
    LONGINT lUser2	PACKED;
    LONGINT lUser3	PACKED;
    LONGINT lUser4	PACKED;
    INTEGER iNumFst	PACKED;
    INTEGER iNumLst	PACKED;
    /* more stuff may be here */
} TPrDlg, *TPPrDlg;


#if !defined (PrintErr)
extern INTEGER 	PrintErr;
#endif

#if !defined (__STDC__)
extern INTEGER PrError(); 
extern void PrSetError(); 
extern void PrOpen(); 
extern void PrClose(); 
extern void PrDrvrOpen(); 
extern void PrDrvrClose(); 
extern void PrCtlCall(); 
extern Handle PrDrvrDCE(); 
extern INTEGER PrDrvrVers(); 
extern void C_ROMlib_myjobproc(); 
extern pascal BOOLEAN C_ROMlib_stlfilterproc(); 
extern pascal BOOLEAN C_ROMlib_numsonlyfilterproc(); 
extern void C_ROMlib_mystlproc(); 
extern TPPrDlg PrJobInit(); 
extern TPPrDlg PrStlInit(); 
extern BOOLEAN PrDlgMain(); 
extern void PrGeneral(); 
extern void donotPrArc(); 
extern void PrArc(); 
extern void donotPrBits(); 
extern void PrBits(); 
extern void donotPrLine(); 
extern void PrLine(); 
extern void donotPrOval(); 
extern void PrOval(); 
extern void textasPS(); 
extern void donotPrGetPic(); 
extern void PrGetPic(); 
extern void donotPrPutPic(); 
extern void PrPutPic(); 
extern void donotPrPoly(); 
extern void PrPoly(); 
extern void donotPrRRect(); 
extern void PrRRect(); 
extern void donotPrRect(); 
extern void PrRect(); 
extern void donotPrRgn(); 
extern void PrRgn(); 
extern INTEGER PrTxMeas(); 
extern void donotPrText(); 
extern void PrText(); 
extern void PrComment(); 
extern TPPrPort PrOpenDoc(); 
extern void PrOpenPage(); 
extern void PrClosePage(); 
extern void PrCloseDoc(); 
extern void PrPicFile(); 
extern void PrintDefault(); 
extern BOOLEAN PrValidate(); 
extern BOOLEAN PrStlDialog(); 
extern BOOLEAN PrJobDialog(); 
extern void PrJobMerge(); 
#else /* __STDC__ */
extern pascal trap INTEGER C_PrError( void  );
extern pascal trap void C_PrSetError( INTEGER iErr );
extern pascal trap void C_PrOpen( void  );
extern pascal trap void C_PrClose( void  );
extern pascal trap void C_PrDrvrOpen( void  );
extern pascal trap void C_PrDrvrClose( void  );
extern pascal trap void C_PrCtlCall( INTEGER iWhichCtl, LONGINT lParam1, 
				    LONGINT lParam2, LONGINT lParam3 ); 
extern pascal trap Handle C_PrDrvrDCE( void  );
extern pascal trap INTEGER C_PrDrvrVers( void  );
extern pascal void C_ROMlib_myjobproc( DialogPtr dp, INTEGER itemno );
extern pascal BOOLEAN C_ROMlib_stlfilterproc( DialogPeek dp, 
					     EventRecord *evt, INTEGER *ith );

extern pascal BOOLEAN C_ROMlib_numsonlyfilterproc( DialogPeek dp,
						  EventRecord *evt,
						  INTEGER *ith );

extern pascal void C_ROMlib_mystlproc( DialogPtr dp, INTEGER itemno );
extern pascal trap TPPrDlg C_PrJobInit( THPrint hPrint );
extern pascal trap TPPrDlg C_PrStlInit( THPrint hPrint );
extern pascal trap BOOLEAN C_PrDlgMain( THPrint hPrint, ProcPtr initfptr );
extern pascal trap void C_PrGeneral( Ptr pData );
extern pascal trap void C_donotPrArc( GrafVerb verb, Rect *r,
				     INTEGER starta, INTEGER arca );
extern pascal trap void C_PrArc( GrafVerb verb, Rect *r, INTEGER starta,
				INTEGER arca );
extern pascal trap void C_donotPrBits( BitMap *srcbmp, Rect *srcrp,
				      Rect *dstrp, INTEGER mode,
				      RgnHandle mask );
extern pascal trap void C_PrBits( BitMap *srcbmp, Rect *srcrp,
				 Rect *dstrp, INTEGER mode, RgnHandle mask );
extern pascal trap void C_donotPrLine( Point p );
extern pascal trap void C_PrLine( Point p );
extern pascal trap void C_donotPrOval( GrafVerb v, Rect *rp );
extern pascal trap void C_PrOval( GrafVerb v, Rect *rp );
extern pascal trap void C_textasPS( INTEGER n, Ptr textbufp,
				   Point num, Point den );
extern pascal trap void C_donotPrGetPic( Ptr dp, INTEGER bc );
extern pascal trap void C_PrGetPic( Ptr dp, INTEGER bc );
extern pascal trap void C_donotPrPutPic( Ptr sp, INTEGER bc );
extern pascal trap void C_PrPutPic( Ptr sp, INTEGER bc );
extern pascal trap void C_donotPrPoly( GrafVerb verb, PolyHandle ph );
extern pascal trap void C_PrPoly( GrafVerb verb, PolyHandle ph );
extern pascal trap void C_donotPrRRect( GrafVerb verb, Rect *r,
				       INTEGER width, INTEGER height );
extern pascal trap void C_PrRRect( GrafVerb verb, Rect *r, INTEGER width,
				  INTEGER height );

extern pascal trap void C_donotPrRect( GrafVerb v, Rect *rp );
extern pascal trap void C_PrRect( GrafVerb v, Rect *rp );
extern pascal trap void C_donotPrRgn( GrafVerb verb, RgnHandle rgn );
extern pascal trap void C_PrRgn( GrafVerb verb, RgnHandle rgn );
extern pascal trap INTEGER C_PrTxMeas( INTEGER n, Ptr p, Point *nump,
				      Point *denp, FontInfo *finfop );
extern pascal trap void C_donotPrText( INTEGER n, Ptr textbufp, Point num,
				      Point den );
extern pascal trap void C_PrText( INTEGER n, Ptr textbufp, Point num,
				 Point den );
extern pascal trap void C_PrComment( INTEGER kind, INTEGER size, Handle hand );
extern pascal trap TPPrPort C_PrOpenDoc( THPrint hPrint, TPPrPort port,
					Ptr pIOBuf );
extern pascal trap void C_PrOpenPage( TPPrPort port, TPRect pPageFrame );
extern pascal trap void C_PrClosePage( TPPrPort pPrPort );
extern pascal trap void C_PrCloseDoc( TPPrPort port );
extern pascal trap void C_PrPicFile( THPrint hPrint, TPPrPort pPrPort,
				    Ptr pIOBuf, Ptr pDevBuf,
				    TPrStatus *prStatus ); 
extern pascal trap void C_PrintDefault( THPrint hPrint );
extern pascal trap BOOLEAN C_PrValidate( THPrint hPrint );
extern pascal trap BOOLEAN C_PrStlDialog( THPrint hPrint );
extern pascal trap BOOLEAN C_PrJobDialog( THPrint hPrint );
extern pascal trap void C_PrJobMerge( THPrint hPrintSrc, THPrint hPrintDst );
#endif /* __STDC__ */
#endif /* __PRINTING__ */
