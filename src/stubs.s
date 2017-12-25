/*
 * NS3.1 BROKEN ASSEMBLER NOTE:  There are several instances of
 *				 displacements in our dispatch tables
 *				 where we use ":w" instead of ":b" because
 *				 the NS3.1 assembler botches.  It's not worth
 *				 reverting once they fix.  Who cares?
 *				 [In addition to :w I had to move the tables
 *				 forward]
 */

.text

.ascii "Copyright 1992 by Abacus Research and Development, Inc.\
  All rights reserved."



	.even

.globl __GetDefaultStartup
__GetDefaultStartup:
	movel	#-1,	a0@
	moveb	#-33,	a0@(3)
	rts

.globl __SetDefaultStartup
__SetDefaultStartup:
	rts

.globl __GetVideoDefault
__GetVideoDefault:
	clrb	a0@
	moveb	#-56,	a0@(1)
	rts

.globl __SetVideoDefault
__SetVideoDefault:
	rts

.globl __GetOSDefault
__GetOSDefault:
	clrb	a0@
	moveb	#1,	a0@(1)
	rts

.globl __SetOSDefault
__SetOSDefault:
	rts

.globl	__SlotVInstall
__SlotVInstall:
	movel	d0,	sp@-
	movel	a0,	sp@-
	jsr	_SlotVInstall
	addql	#8,	sp
	rts

.globl __SlotVRemove
__SlotVRemove:
	movel	d0,	sp@-
	movel	a0,	sp@-
	jsr	_SlotVInstall
	addql	#8,	sp
	rts

.globl __SwapMMUMode
__SwapMMUMode:
	clrb	d0
	addqb	#1,	d0
	moveb	d0,	_MMU32Bit
	rts


.globl __Launch
__Launch:
	clrl		sp@-	/* should have vrefnum */
	movel	a0@,	sp@-
	jsr	_Launch
	addql	#8,	sp	/* shouldn't get here */
	rts


.globl __Chain
__Chain:
	clrl		sp@-	/* should have vrefnum */
	movel	a0@,	sp@-
	jsr	_Chain
	addql	#8,	sp	/* shouldn't get here */
	rts


.globl _IMVI_LowerText
_IMVI_LowerText:
	movel	#-192,	d0
	rts

.globl __SCSIDispatch
__SCSIDispatch:
	movel	sp@,	sp@(2)	/* blast selector */
	addql	#2,	sp
	movew	#7,	d0	/* scMgrBusyErr */
	movew	d0,	sp@(4)
	rts

.globl __LoadSeg
__LoadSeg:
	link	a6,		#0
	moveml	d0/d1/a0/a1,	sp@-
	movew	a6@(8),		d0
	movel	d0,		sp@-
	jbsr	_C_LoadSeg
	addql	#4,		sp
	subl	#6,		a6@(4)
	moveml	sp@+,		d0/d1/a0/a1
	unlk	a6
	rtd	#2

.globl	_zerod0SetCtlValue
_zerod0SetCtlValue:
	link	a6,		#0
	movew	a6@(8),		d0
	movel	d0,		sp@-
	movel	a6@(10),	sp@-
	jbsr	_C_SetCtlValue
	unlk	a6
	clrl	d0
	rtd	#6

.globl __HWPriv
__HWPriv:
	movel	a0,	sp@-
	movel	d0,	sp@-
	jbsr	_HWPriv
	addql	#8,	sp	/* pop our args */
	rts

.globl	__ResourceStub
__ResourceStub:
	moveml	d0/d1/d2/a1,	sp@-
	movel	a2,		sp@-
	movel	a4,		sp@-
	jsr	_ROMlib_mgetres2
	addql	#8,		sp
	movel	d0,		a0
	moveml	sp@+,		d0/d1/d2/a1
	rts

.globl _P_WackyQD32Trap
_P_WackyQD32Trap:	
	.word 0x4AFC
			
.globl	__UNKNOWN
__UNKNOWN:
	.word 0x4AFC

.globl	__CountADBs
__CountADBs:
	clrb	d0
	rts

.globl	__GetIndADB
.globl	__GetADBInfo
.globl	__SetADBInfo
.globl	__ADBReInit
.globl	__ADBOp
.globl	__DrvrInstall	/* Not Supported */
.globl	__DrvrRemove	/* Not Supported */
.globl	__RDrvrInstall	/* Not Supported */
__GetIndADB:
__GetADBInfo:
__SetADBInfo:
__ADBOp:
__DrvrInstall:
__DrvrRemove:
__RDrvrInstall:
	moveql	#-1,	d0
__ADBReInit:
	rts

.globl ___GetResource
___GetResource:
	link	a6,	#0
	clrl	sp@-			/* room for the return value */
	movel	a6@(10),	sp@-
	movew	a6@(8),		sp@-
	jbsr	_P_GetResource
	movel	sp@+,		a6@(14)	/* move return value to proper place */
	clrl	d0	/* for Excel 4.0 Help subsystem, no shit! */
	unlk	a6
	rtd	#6

.globl _PaletteDispatch
_PaletteDispatch:
	cmpb	#0x19,	d0
	bls	2f
1:
	.word	0x4AFC
2:
	subql	#4,	sp
	movew	d0,	sp@-
	extbl	d0
PDL1:
	movel	pc@(PDL2-PDL1-2:w,d0:w:4),	sp@(2)
	beq	1b
	movew	sp@+,	d0
	rts
PDL2:	
	.long	_P_Entry2Index		/* 0x00 */
	.long	0			/* 0x01 */
	.long	_P_RestoreClutDevice	/* 0x02 */
	.long	_P_ResizePalette	/* 0x03 */
	.long	0			/* 0x04 */
	.long	0			/* 0x05 */
	.long	0			/* 0x06 */
	.long	0			/* 0x07 */
	.long	0			/* 0x08 */
	.long	0			/* 0x09 */
	.long	0			/* 0x0A */
	.long	0			/* 0x0B */
	.long	0			/* 0x0C */
	.long	_P_SaveFore		/* 0x40D */
	.long	_P_SaveBack		/* 0x40E */
	.long	_P_RestoreFore		/* 0x40F */
	.long	_P_RestoreBack		/* 0x410 */
	.long	0			/* 0x11 */
	.long	0			/* 0x12 */
	.long	_P_SetDepth		/* 0xA13 */
	.long	_P_HasDepth		/* 0xA14 */
	.long	_P_PMgrVersion		/* 0x15 */
	.long	_P_SetPaletteUpdates	/* 0x616 */
	.long	_P_GetPaletteUpdates	/* 0x417 */
	.long	0			/* 0x18 */
	.long	_P_GetGray		/* 0x1219 */


.globl	_QDExtensions
_QDExtensions:
	cmpw	#0x17,	d0
	bls	QDL1
	.word	0x4AFC
QDL1:
	movel	pc@(QDL2-QDL1-2:w,d0:w:4),	sp@-
	rts
QDL2:	
	.long	_P_NewGWorld
	.long	_P_LockPixels
	.long	_P_UnlockPixels
	.long	_P_UpdateGWorld
	.long	_P_DisposeGWorld
	.long	_P_GetGWorld
	.long	_P_SetGWorld
	.long	_P_CTabChanged
	.long	_P_PixPatChanged
	.long	_P_PortChanged
	.long	_P_GDeviceChanged
	.long	_P_AllowPurgePixels
	.long	_P_NoPurgePixels
	.long	_P_GetPixelsState
	.long	_P_SetPixelsState
	.long	_P_GetPixBaseAddr
	.long	_P_NewScreenBuffer
	.long	_P_DisposeScreenBuffer
	.long	_P_GetGWorldDevice
	.long	_P_QDDone
	.long	_P_OffscreenVersion
	.long	_P_NewTempScreenBuffer
	.long	_P_PixMap32Bit
	.long	_P_GetGWorldPixMap

.globl	_SoundDispatch
_SoundDispatch:
	subql	#4,	sp	/* make room for address */
	movel	a0,	sp@-	/* save a0 */

	movel	#SDTAB-4,	a0
1:
	addql	#4,	a0
	cmpl	a0@+,	d0
	bcs	1b
	beq	1f
	.word	0x4AFC
1:
	movel	a0@,	sp@(4)
	movel	sp@+,	a0
	rts

SDTAB:
	.long	0x00000010
	.long	_P_MACEVersion
	.long	0x00000014
	.long	_P_SPBVersion
	.long	0x00040010
	.long	_P_Comp3to1
	.long	0x00080010
	.long	_P_Exp1to3
	.long	0x000C0008
	.long	_P_SndSoundManagerVersion
	.long	0x000C0010
	.long	_P_Comp6to1
	.long	0x00100008
	.long	_P_SndChannelStatus
	.long	0x00100010
	.long	_P_Exp1to6
	.long	0x00140008
	.long	_P_SndManagerStatus
	.long	0x00180008
	.long	_P_SndGetSysBeepState
	.long	0x001C0008
	.long	_P_SndSetSysBeepState
	.long	0x00200008
	.long	_P_SndPlayDoubleBuffer
	.long	0x01100014
	.long	_P_SPBSignOutDevice
	.long	0x02040008
	.long	_P_SndPauseFilePlay
	.long	0x021C0014
	.long	_P_SPBCloseDevice
	.long	0x02280014
	.long	_P_SPBPauseRecording
	.long	0x022C0014
	.long	_P_SPBResumeRecording
	.long	0x02300014
	.long	_P_SPBStopRecording
	.long	0x03080008
	.long	_P_SndStopFilePlay
	.long	0x030C0014
	.long	_P_SPBSignInDevice
	.long	0x03200014
	.long	_P_SPBRecord
	.long	0x04240014
	.long	_P_SPBRecordToFile
	.long	0x04400014
	.long	_P_SPBMillisecondsToBytes
	.long	0x04440014
	.long	_P_SPBBytesToMilliseconds
	.long	0x05140014
	.long	_P_SPBGetIndexedDevice
	.long	0x05180014
	.long	_P_SPBOpenDevice
	.long	0x06380014
	.long	_P_SPBGetDeviceInfo
	.long	0x063C0014
	.long	_P_SPBSetDeviceInfo
	.long	0x07080014
	.long	_P_SndRecordToFile
	.long	0x08040014
	.long	_P_SndRecord
	.long	0x0B4C0014
	.long	_P_SetupAIFFHeader
	.long	0x0D000008
	.long	_P_SndStartFilePlay
	.long	0x0D480014
	.long	_P_SetupSndHeader
	.long	0x0E340014
	.long	_P_SPBGetRecordingStatus
	.long	0xFFFFFFFF
	.long	myabort


.globl	_ScriptUtil
_ScriptUtil:
	movew	d0,	sp@-	/* save d0 */
	clrw	d0
	moveb	sp@(9),	d0	/* get selector */
	movel	sp@(2),	sp@(6)	/* slide up the ret-pc */
	cmpw	#0x36,	d0
	bhi	SUDL2
SUDL1:
	movel	pc@(SUDL5-SUDL1-2:w,d0:w:2),	sp@(2)
	beq	SUDL4
	movew	sp@+,	d0
	rts
SUDL5:
	.long	_P_FontScript	/* 0 FontScript */
	.long	_P_IntlScript	/* 2 IntlScript */
	.long	_P_KeyScript	/* 4 KybdScript */
	.long	_P_Font2Script	/* 6 Font2Script */
	.long	_P_GetEnvirons	/* 8 */
	.long	_P_SetEnvirons	/* 10 */
	.long	_P_GetScript	/* 12 */
	.long	_P_SetScript	/* 14 */
	.long	_P_CharByte	/* 16 CharByte */
	.long	_P_CharType	/* 18 CharType */
	.long	_P_Pixel2Char	/* 20 Pixel2Char */
	.long	_P_Char2Pixel	/* 22 Char2Pixel */
	.long	_P_Transliterate	/* 24 Translit*/
	.long	_P_FindWord	/* 26 FindWord */
	.long	_P_HiliteText	/* 28 HiliteText */
	.long	_P_DrawJust	/* 30 DrawJust */
	.long	_P_MeasureJust	/* 32 MeasureJust */
	.long	0	/* 0x22 ParseTable */
	.long	0	/* 0x24 PortionText */
	.long	0	/* 0x26 FindScriptRun */
	.long	0	/* 0x28 VisibleLength */
	.long	0	/* 0x2A IsSpecialFont */
	.long	0	/* 0x2C RawPrinterValues */
	.long	0	/* 0x2E NPixel2Char */
	.long	0	/* 0x30 NChar2Pixel */
	.long	0	/* 0x32 NDrawJust */
	.long	0	/* 0x34 NMeasureJust */
	.long	0	/* 0x36 NPortionText */

SUDL2:	
	cmpw	#0xDC,	d0
	blt	SUDL4
SUDL3:
	movel	pc@(SUDL6-SUDL3 - 2 - 2*0xDC:w,d0:w:2),	sp@(2)
	beq	SUDL4
	movew	sp@+,	d0
	rts
SUDL4:	.word 0x4AFC	/* don't know what it is */

SUDL6:
	.long	0	/* 0xDC ReplaceText */
	.long	0	/* 0xDE TruncText */
	.long	0	/* 0xE0 TruncString */
	.long	0	/* 0xE2 NFindWord */
	.long	0	/* 0xE4 ValidDate */
	.long	0	/* 0xE6 FormatStr2X */
	.long	0	/* 0xE8 FormatX2Str */
	.long	0	/* 0xEA Format2Str */
	.long	0	/* 0xEC Str2Format */
	.long	0	/* 0xEE ToggleDate */
	.long	0	/* 0xF0 LongSecs2Date */
	.long	0	/* 0xF2 LongDate2Secs */
	.long	_P_String2Time	/* 0xF4 String2Time */
	.long	0	/* 0xF6 String2Date */
	.long	_P_InitDateCache	/* 0xF8 InitDateCache */
	.long	0	/* 0xFA IntlTokenize */
	.long	0	/* 0xFC GetFormatOrder */
	.long	0	/* 0xFE StyledLineBreak */


.globl	_TEDispatch
_TEDispatch:
	subql	#2,	sp	/* shim for return pc */
	movew	d0,	sp@-	/* save d0 */
	movew	sp@(8),	d0	/* get selector */
	movel	sp@(4),	sp@(6)	/* slide up the ret-pc */
	cmpw	#13,	d0
	bhi	TEDL2
TEDL1:
	movel	pc@(TEDL3-TEDL1-2:w,d0:w:4),	sp@(2)
	movew	sp@+,	d0
	rts
TEDL2:	.word 0x4AFC	/* don't know what it is */
TEDL3:
	.long	_P_TEStylPaste	/* 0 */
	.long	_P_TESetStyle	/* 1 */
	.long	_P_TEReplaceStyle	/* 2 */
	.long	_P_TEGetStyle	/* 3 */
	.long	_P_GetStylHandle	/* 4 */
	.long	_P_SetStylHandle	/* 5 */
	.long	_P_GetStylScrap	/* 6 */
	.long	_P_TEStylInsert	/* 7 */
	.long	_P_TEGetPoint	/* 8 */
	.long	_P_TEGetHeight	/* 9 */
	.long	_P_TEContinuousStyle	/* 10 */
	.long	_P_SetStylScrap	/* 11 */
	.long	_P_TECustomHook	/* 12 */
	.long	_P_TENumStyles	/* 13 */

/* beginning of Toolbox Utilities that have hidden arguments */

.globl __Fix2X
__Fix2X:
	jsr	_R_Fix2X
	rtd	#4

.globl __Frac2X
__Frac2X:
	jsr	_R_Frac2X
	rtd	#4

/* end of Toolbox Utilities that use Extended format */

/* beginning of routines that toolbox routines that are register based */

/*
 * NOTE: IMI-94 claims that register based traps have registers a1, d1 and d2
 *	 preserved.  Previously I took that to mean os traps, but DayMaker
 *	 calls Secs2Date and expects a1 to be preserved.  Does this mean
 *	 that the trap mechanism should be doing the preservation, or does
 *	 it mean that the stub should do the preservation.  Right now, I'm
 *	 going to do the preservation in the stub, but if I'm wrong, it
 *	 could show up if someone were to patch one of the toolbox traps
 *	 that is register based.
 */

.globl	__HandToHand
__HandToHand:
	link	a6,	#-4
	moveml	d1/a1,	sp@-
	movel	a0,	a6@(-4)	/* so we can take address */
	pea	a6@(-4)
	jbsr	_HandToHand
	movel	a6@(-4),	a0
	moveml	sp@(4),	d1/a1
	unlk	a6
	rts

.globl	__PtrToHand
__PtrToHand:
	link	a6,	#-4
	moveml	d1/a1,	sp@-
	movel	d0,	sp@-	/* size */
	pea	a6@(-4)	/* dsthndl */
	movel	a0,	sp@-	/* srcptr */
	jbsr	_PtrToHand
	movel	a6@(-4),	a0
	moveml	sp@(12),	d1/a1
	unlk	a6
	rts

.globl	__PtrToXHand
__PtrToXHand:
	link	a6,	#-4
	moveml	d1/a1,	sp@-
	movel	a1,	a6@(-4)
	movel	d0,	sp@-	/* size */
	movel	a1,	sp@-	/* dsthndl */
	movel	a0,	sp@-
	jbsr	_PtrToXHand
	movel	a6@(-4),	a0
	moveml	sp@(12),	d1/a1
	unlk	a6
	rts

.globl	__HandAndHand
__HandAndHand:
	link	a6,	#-4
	moveml	d1/a1,	sp@-
	movel	a1,	a6@(-4)
	movel	a1,	sp@-
	movel	a0,	sp@-
	jbsr	_HandAndHand
	movel	a6@(-4),	a0
	moveml	sp@(8),	d1/a1
	unlk	a6
	rts

.globl	__PtrAndHand
__PtrAndHand:
	link	a6,	#-4
	moveml	d1/a1,	sp@-
	movel	a1,	a6@(-4)
	movel	d0,	sp@-
	movel	a1,	sp@-
	movel	a0,	sp@-
	jbsr	_PtrAndHand
	movel	a6@(-4),	a0
	moveml	sp@(12),	d1/a1
	unlk	a6
	rts

.globl	__Date2Secs
__Date2Secs:
	link	a6,	#-4
	moveml	d1/a1,	sp@-
	pea	a6@(-4)
	movel	a0,	sp@-
	jbsr	_Date2Secs
	movel	a6@(-4),	d0
	moveml	sp@(8),	d1/a1
	unlk	a6
	rts

.globl	__Secs2Date
__Secs2Date:
	link	a6,	#-4
	moveml	d1/a1,	sp@-
	movel	a0,	a6@(-4)
	movel	a0,	sp@-
	movel	d0,	sp@-
	jbsr	_Secs2Date
	movel	a6@(-4),	a0
	moveml	sp@(8),	d1/a1
	unlk	a6
	rts

.globl	__Enqueue
__Enqueue:
	link	a6,	#-4
	moveml	d1/a1,	sp@-
	movel	a1,	a6@(-4)
	movel	a1,	sp@-
	movel	a0,	sp@-
	jbsr	_Enqueue
	movel	a6@(-4),	a1
	moveml	sp@(8),	d1/a1
	unlk	a6
	rts

.globl	__Dequeue
__Dequeue:
	link	a6,	#-4
	moveml	d1/a1,	sp@-
	movel	a1,	a6@(-4)
	movel	a1,	sp@-
	movel	a0,	sp@-
	jbsr	_Dequeue
	movel	a6@(-4),	a1
	moveml	sp@(8),	d1/a1
	unlk	a6
	rts

.globl	__Key1Trans
__Key1Trans:
.globl	__Key2Trans
__Key2Trans:
/*
 * I don't really know what these do and neither does Cliff as we have
 * little documentation on it.  The following is based upon d2 containing
 * the virtual key code and d1 containing the high byte of the modifier
 * flag which is what it appears the mac+ does.
 */
	subql	#4,	sp
	clrl	sp@-
	lslw	#8,	d1
	orw	d2,	d1
	movew	d1,	sp@-
	clrl	sp@-
	jbsr	_P_KeyTrans
	movel	sp@+,	d0
	swap	d0
	andl	#0xff,	d0
	rts

.globl	__NMInstall
__NMInstall:
	link	a6,	#0
	movel	a0,	sp@-
	jbsr	_NMInstall
	unlk	a6
	rts

.globl	__NMRemove
__NMRemove:
	link	a6,	#0
	movel	a0,	sp@-
	jbsr	_NMRemove
	unlk	a6
	rts


/* end of routines that toolbox routines that are register based */

/* beginning of pack routines */

/*
 * NOTE: Pack4 keeps the sel on the stack, while most other dispatch routines
 * (including more floating point: Pack5 and Pack7) don't.
 */

.globl	_Pack4
_Pack4:
	subql	#4,	sp	/* shim for "phoney return address" */
	movew	d0,	sp@-
	clrw	d0
	moveb	sp@(11),d0	/* get the selector opcode into d0 */
	cmpib	#0x1c,		d0
	bls	pack4here
2:
	.word	0x4AFC
pack4here:
	movel	pc@(p4table-pack4here-2:w,d0:w:4),	sp@(2)
	beq	2b
	movew	sp@+,	d0
	rts	/* really a branch to where we want to go */
p4table:
	.long	_P_ROMlib_Faddx	/* 00 */
	.long	_P_ROMlib_Fsetenv	/* 01 */
	.long	_P_ROMlib_Fsubx	/* 02 */
	.long	_P_ROMlib_Fgetenv	/* 03 */
	.long	_P_ROMlib_Fmulx	/* 04 */
	.long	_P_ROMlib_Fsethv		/* 05 */
	.long	_P_ROMlib_Fdivx	/* 06 */
	.long	_P_ROMlib_Fgethv		/* 07 */
	.long	_P_ROMlib_Fcmpx	/* 08 */
	.long	_P_ROMlib_Fdec2x	/* 09 */
	.long	_P_ROMlib_FcpXx	/* 0A */
	.long	_P_ROMlib_Fx2dec	/* 0B */
	.long	_P_ROMlib_Fremx	/* 0C */
	.long	_P_ROMlib_FnegX	/* 0D */
	.long	_P_ROMlib_Fx2X	/* 0E */
	.long	_P_ROMlib_FabsX	/* 0F */
	.long	_P_ROMlib_FX2x	/* 10 */
	.long	_P_ROMlib_Fcpysgnx/* 11 */
	.long	_P_ROMlib_FsqrtX	/* 12 */
	.long	0		/* 13 */
	.long	_P_ROMlib_FrintX	/* 14 */
	.long	0		/* 15 */
	.long	_P_ROMlib_FtintX	/* 16 */
	.long	_P_ROMlib_Fprocentry	/* 17 */
	.long	_P_ROMlib_FscalbX	/* 18 */
	.long	_P_ROMlib_Fprocexit	/* 19 */
	.long	_P_ROMlib_FlogbX	/* 1A */
	.long	_P_ROMlib_Ftestxcp	/* 1B */
	.long	_P_ROMlib_Fclassx		/* 1C */

.globl	_Pack5
_Pack5:
	subql	#2,	sp	/* shim for "phoney return address" */
	movew	d0,	sp@-
	clrw	d0
	moveb	sp@(9),	d0	/* get the selector opcode into d0 */
	movel	sp@(4), sp@(6)
	cmpib	#0x20,		d0
	bls	pack5here
2:
	.word	0x4AFC
pack5here:
	movel	pc@(p5table-pack5here-2:w,d0:w:2),	sp@(2)
	movew	sp@+,	d0
	rts	/* really a branch to where we want to go */
p5table:
	.long	_P_ROMlib_FlnX		/* 00 */
	.long	_P_ROMlib_Flog2X		/* 02 */
	.long	_P_ROMlib_Fln1X		/* 04 */
	.long	_P_ROMlib_Flog21X		/* 06 */
	.long	_P_ROMlib_FexpX		/* 08 */
	.long	_P_ROMlib_Fexp2X		/* 0A */
	.long	_P_ROMlib_Fexp1X		/* 0C */
	.long	_P_ROMlib_Fexp21X		/* 0E */
	.long	_P_ROMlib_Fxpwri		/* 10 */
	.long	_P_ROMlib_Fxpwry		/* 12 */
	.long	_P_ROMlib_Fcompound	/* 14 */
	.long	_P_ROMlib_Fannuity	/* 16 */
	.long	_P_ROMlib_FsinX		/* 18 */
	.long	_P_ROMlib_FcosX		/* 1A */
	.long	_P_ROMlib_FtanX		/* 1C */
	.long	_P_ROMlib_FatanX		/* 1E */
	.long	_P_ROMlib_FrandX		/* 20 */

.globl	_Pack0
_Pack0:
	subql	#2,	sp	/* shim for retpc */
	movel	d0,	sp@-
	movew	sp@(10),d0	/* get selector */
	movel	sp@(6),	sp@(8)	/* slide up the ret-pc */
	cmpw	#100,	d0
	bhi	LP02
p0here:
	movel	pc@(LP029-p0here-2:w,d0:w:1),	sp@(4)
	movel	sp@+,	a0
	rts
LP02:	.word	0x4AFC	/* don't know what it is */
LP029:
	.long	_P_LActivate	/* 0 */
	.long	_P_LAddColumn	/* 1 */
	.long	_P_LAddRow	/* 2 */
	.long	_P_LAddToCell	/* 3 */
	.long	_P_LAutoScroll	/* 4 */
	.long	_P_LCellSize	/* 5 */
	.long	_P_LClick		/* 6 */
	.long	_P_LClrCell	/* 7 */
	.long	_P_LDelColumn	/* 8 */
	.long	_P_LDelRow	/* 9 */
	.long	_P_LDispose	/* 10 */
	.long	_P_LDoDraw	/* 11 */
	.long	_P_LDraw		/* 12 */
	.long	_P_LFind		/* 13 */
	.long	_P_LGetCell	/* 14 */
	.long	_P_LGetSelect	/* 15 */
	.long	_P_LLastClick	/* 16 */
	.long	_P_LNew		/* 17 */
	.long	_P_LNextCell	/* 18 */
	.long	_P_LRect		/* 19 */
	.long	_P_LScroll	/* 20 */
	.long	_P_LSearch	/* 21 */
	.long	_P_LSetCell	/* 22 */
	.long	_P_LSetSelect	/* 23 */
	.long	_P_LSize		/* 24 */
	.long	_P_LUpdate	/* 25 */

.globl	_Pack3
_Pack3:
	subql	#2,	sp
	movew	d0,	sp@-
	movew	sp@(8),	d0	/* get selector */
	movel	sp@(4),	sp@(6)	/* slide up the ret-pc */
	cmpw	#4,	d0
	bhi	P3L2
P3L1:
	movel	pc@(P3L3-P3L1-6:w,d0:w:4),	sp@(2)
	movew	sp@+,	d0
	rts
P3L2:	.word	0x4AFC	/* don't know what it is */
P3L3:		/* sfputfile */
	.long	_P_SFPutFile
	.long	_P_SFGetFile
	.long	_P_SFPPutFile
	.long	_P_SFPGetFile

.globl	_Pack2
_Pack2:
	subql	#2,	sp
	movew	d0,	sp@-
	movew	sp@(8),	d0	/* get selector */
	movel	sp@(4),	sp@(6)	/* slide up the ret-pc */
	cmpw	#10,	d0
	bhi	DI2
DI12:
	movel pc@(D12-DI12-2:w,d0:w:2),	sp@(2)
	movew	sp@+,	d0
	rts
DI2:	.word	0x4AFC	/* don't know what it is */
D12:
	.long	_DIBadMount	/* 0 */
	.long	_DILoad		/* 2 */
	.long	_DIUnload	/* 4 */
	.long	_DIFormat	/* 6 */
	.long	_DIVerify	/* 8 */
	.long	_DIZero		/* 10 */


_DIBadMount:
_DIFormat:
_DIVerify:
_DIZero:
	.word	0x4AFC

_DILoad:
_DIUnload:
	rts

.globl	_Pack6
_Pack6:
	subql	#2,	sp
	movew	d0,	sp@-
	movew	sp@(8),	d0	/* get selector */
	movel	sp@(4),	sp@(6)	/* slide up the ret-pc */
	cmpw	#0x24,	d0
	bhi	L2
LI12:
	movel pc@(L12-LI12-2:w,d0:w:2),	sp@(2)
	movew	sp@+,	d0
	rts
L2:	.word	0x4AFC	/* don't know what it is */
L12:
	.long	_P_IUDateString
	.long	_P_IUTimeString
	.long	_P_IUMetric
	.long	_P_IUGetIntl
	.long	_P_IUSetIntl
	.long	_P_IUMagString
	.long	_P_IUMagIDString
	.long	_P_IUDatePString
	.long	_P_IUTimePString
	.long	_P_IUMystery
	.long	_P_IULDateString
	.long	_P_IULTimeString
	.long	_P_IUClearCache
	.long	_P_IUMagPString
	.long	_P_IUMagIDPString
	.long	_P_IUScriptOrder
	.long	_P_IULangOrder
	.long	_P_IUTextOrder
	.long	_P_IUGetItlTable



.globl	_Pack7
_Pack7:
	subql	#2,	sp	/* shim for "phoney return address" */
	movew	d0,	sp@-
	movew	sp@(8),	d0	/* get the selector opcode into d0 */
	movel	sp@(4), sp@(6)
	cmpiw	#0x04,		d0
	bls	pack7here
2:
	.word	0x4AFC
pack7here:
	movel	pc@(p7table-pack7here-2:w,d0:w:4),	sp@(2)
	movew	sp@+,	d0
	rts	/* really a branch to where we want to go */
p7table:
	.long	p7iszero		/* 00 */
	.long	p7isone			/* 01 */
	.long	_P_ROMlib_Fpstr2dec	/* 02 */
	.long	_P_ROMlib_Fdec2str	/* 03 */
	.long	_P_ROMlib_Fcstr2dec	/* 04 */

p7iszero:
	moveml	d0/d1/a0/a1,	sp@-
	movel	a0,	sp@-	/* pointer to the string */
	movel	d0,	sp@-	/* the number */
	jbsr	_NumToString
	addql	#8,	sp	/* pop our args */
	moveml	sp@+,	d0/d1/a0/a1
	rts

p7isone:
	moveml	d1/a0/a1,	sp@-
	subql	#4,	sp	/* room for argument */
	pea	sp@
	movel	a0,	sp@-
	jbsr	_StringToNum
	addql	#8,	sp
	movel	sp@+,	d0
	moveml	sp@+,	d1/a0/a1
	rts

/* Pack12, template copied from Pack3 */
	
.globl	_Pack12
_Pack12:
	subql	#2,	sp
	movew	d0,	sp@-
	movew	sp@(8),	d0	/* get selector */
	movel	sp@(4),	sp@(6)	/* slide up the ret-pc */
	cmpw	#4,	d0
	bhi	P12L2
P12L1:
	/* FIXME: figure out what the spew should be in pc@(spew) */
	movel	pc@(P12L3-P12L1-2:w,d0:w:4),	sp@(2)
	movew	sp@+,	d0
	rts
P12L2:	.word	0x4AFC	/* don't know what it is */
P12L3:		
	.long	_P_Fix2SmallFract
	.long	_P_SmallFract2Fix
	.long	_P_CMY2RGB
	.long	_P_RGB2CMY
	.long	_P_HSL2RGB
	.long	_P_RGB2HSL
	.long	_P_HSV2RGB
	.long	_P_RGB2HSV
	.long   _P_GetColor

.globl	_Pack14
_Pack14:
/* NOTE: this isn't really supported, it is just here */
/* 	 to make EqnEditor work */
	cmpw	#0x200,	d0	/* HMGetHelpMenuHandle */
	bne	_Pack14Death
	movew	#-855, 	sp@(8)
	rtd	#4
.globl	_Pack14Death
_Pack14Death:
	illegal

/* end of pack stuff */

/* Beginning of hand crafted OStrap stubs */

.globl	__SysEnvirons
__SysEnvirons:
	link	a6,	#-4
	movel	a0,	a6@(-4)
	movel	a0,	sp@-
	movel	d0,	sp@-	/* MSHORT: movew */
	jbsr	_SysEnvirons
	movel	a6@(-4),	a0
	unlk	a6
	extl	d0
	rts

/* begin Event Manager */

.globl	__PostEvent	/* handles both Post and PPost */
__PostEvent:
	subql	#4,	sp	/* reserve room for qelemp */
	pea	sp@		/* pointer to qelemp */
	movel	d0,	sp@-
	movel	a0,	sp@-	/* MSHORT: movew */
	jbsr	_PPostEvent
	addl	#12,	sp	/* MSHORT: #10 */
	movel	sp@+,	a0
	rts

.globl	__FlushEvents
__FlushEvents:
	link	a6,	#0
	swap	d0
	movel	d0,	sp@-	/* MSHORT: movew */
	swap	d0
	movel	d0,	sp@-	/* MSHORT: movew */
	jbsr	_FlushEvents
			/* NOTE: there should be stuff in d0 */
	clrl	d0	/* lie */
	unlk	a6
	rts

.globl	__GetOSEvent
__GetOSEvent:
	link	a6,	#0
	movel	a0,	sp@-
	movel	d0,	sp@-	/* MSHORT: movew */
	jbsr	_GetOSEvent
	tstw	d0
	seq	d0
	extbl	d0
	unlk	a6
	rts

.globl	__OSEventAvail
__OSEventAvail:
	link	a6,	#0
	movel	a0,	sp@-
	movel	d0,	sp@-	/* MSHORT: movew */
	jbsr	_OSEventAvail
	tstw	d0
	seq	d0
	extbl	d0
	unlk	a6
	rts

/* end of Event Manager */

.globl	__VInstall
__VInstall:
	link	a6,	#0
	movel	a0,	sp@-
	jbsr	_VInstall
	unlk	a6
	rts

.globl	__VRemove
__VRemove:
	link	a6,	#0
	movel	a0,	sp@-
	jbsr	_VRemove
	unlk	a6
	rts

.globl __InsTime
__InsTime:
	link	a6,	#0
	movel	a0,	sp@-
	jbsr	_InsTime
	unlk	a6
	clrl	d0
	rts

.globl __RmvTime
__RmvTime:
	link	a6,	#0
	movel	a0,	sp@-
	jbsr	_RmvTime
	unlk	a6
	clrl	d0
	rts

.globl __PrimeTime
__PrimeTime:
	link	a6,	#0
	movel	d0,	sp@-
	movel	a0,	sp@-
	jbsr	_PrimeTime
	unlk	a6
	clrl	d0
	rts

.globl	__ReadDateTime
__ReadDateTime:
	link	a6,	#0
	movel	a0, sp@-
	jbsr	_ReadDateTime
	unlk	a6
	rts

.globl	__SetDateTime
__SetDateTime:
	link	a6,	#0
	movel	d0,	sp@-
	jbsr	_SetDateTime
	unlk	a6
	rts

.globl	__Delay
__Delay:
	link	a6,	#-4
	pea	a6@(-4)
	movel	a0,	sp@-
	jbsr	_Delay
	movel	a6@(-4),	d0
	unlk	a6
	rts

.globl	__EqualString
__EqualString:
	link	a6,	#0
	movel	d0,	sp@-
	clrl	sp@-	/* diacsense = false, MSHORT: clrw */
	btst	#9,	d1
	bne	nodiac
diac:
	notl	sp@	/* diacsense = true, MSHORT: notw */
nodiac:
	clrl	sp@-	/* casesense = false, MSHORT: clrw */
	btst	#10,	d1
	beq	nocase
case:
	notl	sp@	/* casesense = false, MSHORT: notw */
nocase:
	movel	a1,	sp@-
	movel	a0,	sp@-
	jbsr	_ROMlib_RelString
	tstl	d0
	bge	1f
	negl	d0
1:
	unlk	a6
	rts

.globl	__RelString
__RelString:
	link	a6,	#0
	movel	d0,	sp@-
	clrl	sp@-	/* diacsense = false, MSHORT: clrw */
	btst	#9,	d1
	bne	rnodiac
rdiac:
	notl	sp@	/* diacsense = true, MSHORT: notw */
rnodiac:
	clrl	sp@-	/* casesense = false, MSHORT: clrw */
	btst	#10,	d1
	beq	rnocase
rcase:
	notl	sp@	/* casesense = false, MSHORT: notl */
rnocase:
	movel	a1,	sp@-
	movel	a0,	sp@-
	jbsr	_ROMlib_RelString
	unlk	a6
	rts

.globl	__UprString
__UprString:
	link	a6,	#-4
	movel	a0,	a6@(-4)
	movel	d0,	sp@-	/* MSHORT: movew */
	clrl	sp@-	/* diacsense = false, MSHORT: clrw */
	btst	#9,	d1
	bne	unodiac
udiac:
	notl	sp@	/* diacsense = true, MSHORT: notl */
unodiac:
	movel	a0,	sp@-
	jbsr	_ROMlib_UprString
	movel	a6@(-4), a0
	unlk	a6
	rts

.globl	__StripAddress
__StripAddress:
	cmpl	#0xFFFFFFFF,	d0
	bne	1f
/* 4 comes from the #define for ROMLIB_STRIPADDRESSHACK_BIT in options.h */
	btst	#4,		_ROMlib_options + 3
	beq	1f
	andil	#0xFFFFFF,	d0
1:
	rts

myabort:	.word	0x4AFC

prtab:
	.long _P_PrOpenDoc	/* 0 */
	.long _P_PrCloseDoc	/* 1 */
	.long _P_PrOpenPage	/* 2 */
	.long _P_PrClosePage	/* 3 */
	.long _P_PrintDefault	/* 4 */
	.long _P_PrStlDialog	/* 5 */
	.long _P_PrJobDialog	/* 6 */
	.long _P_PrStlInit	/* 7 */
	.long _P_PrJobInit	/* 8 */
	.long _P_PrDlgMain	/* 9 */
	.long _P_PrValidate	/* 10 */
	.long _P_PrJobMerge	/* 11 */
	.long _P_PrPicFile	/* 12 */
	.long myabort		/* 13 */
	.long _P_PrGeneral	/* 14 */
	.long myabort		/* 15 */
	.long _P_PrDrvrOpen	/* 16 */
	.long _P_PrDrvrClose	/* 17 */
	.long _P_PrDrvrDCE	/* 18 */
	.long _P_PrDrvrVers	/* 19 */
	.long _P_PrCtlCall	/* 20 */
	.long _P_PrPurge		/* 21 */
	.long _P_PrNoPurge	/* 22 */
	.long _P_PrError		/* 23 */
	.long _P_PrSetError	/* 24 */
	.long _P_PrOpen		/* 25 */
	.long _P_PrClose		/* 26 */

.globl	__PrGlue
__PrGlue:
	movel	d0,	sp@-
	clrl	d0
	moveb	sp@(8),	d0	/* pick up selector */
	movel	sp@(4),	sp@(8)	/* move return address */
	lsrl	#3,	d0
	cmpl	#26,	d0
	bls	1f
	.word	0x4AFC
1:
	movel	a0,	sp@-
	lea prtab,	a0
	movel a0@(d0:l:4),	sp@(8)
	movel	sp@+,	a0
	movel	sp@+,	d0
	rts	/* goes where we want it */

/* beginning of File Manager */

hfstab:
	.long myabort		/* 0 */
	.long _PBOpenWD		/* 1 */
	.long _PBCloseWD	/* 2 */
	.long myabort		/* 3 */
	.long myabort		/* 4 */
	.long _PBCatMove	/* 5 */
	.long _PBDirCreate	/* 6 */
	.long _PBGetWDInfo	/* 7 */
	.long _PBGetFCBInfo	/* 8 */
	.long _PBGetCatInfo	/* 9 */
	.long _PBSetCatInfo	/* 10 */
	.long _PBSetVInfo	/* 11 */
	.long myabort		/* 12 */
	.long myabort		/* 13 */
	.long myabort		/* 14 */
	.long myabort		/* 15 */
	.long _PBLockRange	/* 0x10 */
	.long _PBUnlockRange	/* 0x11 */
	.long myabort		/* 0x12 */
	.long myabort		/* 0x13 */
	.long myabort		/* 0x14 */
	.long myabort		/* 0x15 */
	.long myabort		/* 0x16 */
	.long myabort		/* 0x17 */
	.long myabort		/* 0x18 */
	.long myabort		/* 0x19 */
	.long _PBOpen		/* 0x1A */
	.long myabort		/* 0x1B */
	.long myabort		/* 0x1C */
	.long myabort		/* 0x1D */
	.long myabort		/* 0x1E */
	.long myabort		/* 0x1F */
	.long myabort		/* 0x20 */
	.long myabort		/* 0x21 */
	.long myabort		/* 0x22 */
	.long myabort		/* 0x23 */
	.long myabort		/* 0x24 */
	.long myabort		/* 0x25 */
	.long myabort		/* 0x26 */
	.long myabort		/* 0x27 */
	.long myabort		/* 0x28 */
	.long myabort		/* 0x29 */
	.long myabort		/* 0x2A */
	.long myabort		/* 0x2B */
	.long myabort		/* 0x2C */
	.long myabort		/* 0x2D */
	.long myabort		/* 0x2E */
	.long myabort		/* 0x2F */
	.long _PBHGetVolParms	/* 0x30 */
	.long _GetLogInInfo	/* 0x31 */
	.long _GetDirAccess	/* 0x32 */
	.long _SetDirAccess	/* 0x33 */
	.long _MapID		/* 0x34 */
	.long _MapName		/* 0x35 */
	.long _CopyFile		/* 0x36 */
	.long _MoveRename	/* 0x37 */
	.long _OpenDeny		/* 0x38 */
	.long _OpenRFDeny	/* 0x39 */

.globl	__HFSDispatch
__HFSDispatch:
	link	a6,	#0
	cmpw	#0x39,	d0
	bls	1f
	.word	0x4AFC
1:
	clrl	sp@-	/* MSHORT: clrw */
	btst	#10,	d1
	beq	na
	notl	sp@	/* MSHORT: notw */
na:
	movel	a0,	sp@-
	lea hfstab,	a0
	movel a0@(d0:w:4),	a0
	jbsr 	a0@
/*
 * NOTE: I have no idea why I had the next instruction in here.  It
 *	 certainly looks like I thought that we had some sort of pascal
 *	 calling convention going on here. --ctm 3/23/93
 */

/*	movel	sp@,	a0	 no need for sp@+ with unlk next insn */
	unlk	a6
	rts

.globl	__FInitQueue
__FInitQueue:	/* TODO */
	/* .word	0x4AFC 		NOP */
	rts

.globl	__PBOpen
__PBOpen:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	openna
	notl	sp@		/* MSHORT: notw */
openna:
	movel	a0,	sp@-
	btst	#9,	d1
	bne	openhfs
	jbsr	_PBOpen
	unlk	a6
	rts
openhfs:
	jbsr	_PBHOpen
	unlk	a6
	rts

.globl	__PBClose
__PBClose:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	closena
	notl	sp@		/* MSHORT: notw */
closena:
	movel	a0,	sp@-
	jbsr	_PBClose
	unlk	a6
	rts

.globl	__PBRead
__PBRead:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	readna
	notl	sp@		/* MSHORT: notw */
readna:
	movel	a0,	sp@-
	jbsr	_PBRead
	unlk	a6
	rts

.globl	__PBWrite
__PBWrite:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	writena
	notl	sp@		/* MSHORT: notw */
writena:
	movel	a0,	sp@-
	jbsr	_PBWrite
	unlk	a6
	rts

.globl	__PBControl
__PBControl:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	controlna
	notl	sp@		/* MSHORT: notw */
controlna:
	movel	a0,	sp@-
	jbsr	_PBControl
	unlk	a6
	rts

.globl	__PBStatus
__PBStatus:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	statusna
	notl	sp@		/* MSHORT: notw */
statusna:
	movel	a0,	sp@-
	jbsr	_PBStatus
	unlk	a6
	rts

.globl	__PBKillIO
__PBKillIO:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	killiona
	notl	sp@		/* MSHORT: notw */
killiona:
	movel	a0,	sp@-
	jbsr	_PBKillIO
	unlk	a6
	rts

.globl	__PBGetVInfo
__PBGetVInfo:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	getvinfona
	notl	sp@		/* MSHORT: notw */
getvinfona:
	movel	a0,	sp@-
	btst	#9,	d1
	bne	getvinfohfs
	jbsr	_PBGetVInfo
	unlk	a6
	rts
getvinfohfs:
	jbsr	_PBHGetVInfo
	unlk	a6
	rts

.globl	__PBCreate
__PBCreate:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	createna
	notl	sp@		/* MSHORT: notw */
createna:
	movel	a0,	sp@-
	btst	#9,	d1
	bne	createhfs
	jbsr	_PBCreate
	unlk	a6
	rts
createhfs:
	jbsr	_PBHCreate
	unlk	a6
	rts

.globl	__PBDelete
__PBDelete:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	deletena
	notl	sp@		/* MSHORT: notw */
deletena:
	movel	a0,	sp@-
	btst	#9,	d1
	bne	deletehfs
	jbsr	_PBDelete
	unlk	a6
	rts
deletehfs:
	jbsr	_PBHDelete
	unlk	a6
	rts

.globl	__PBOpenRF
__PBOpenRF:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	openrfna
	notl	sp@		/* MSHORT: notw */
openrfna:
	movel	a0,	sp@-
	btst	#9,	d1
	bne	openrfhfs
	jbsr	_PBOpenRF
	unlk	a6
	rts
openrfhfs:
	jbsr	_PBHOpenRF
	unlk	a6
	rts

.globl	__PBRename
__PBRename:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	renamena
	notl	sp@		/* MSHORT: notw */
renamena:
	movel	a0,	sp@-
	btst	#9,	d1
	bne	renamehfs
	jbsr	_PBRename
	unlk	a6
	rts
renamehfs:
	jbsr	_PBHRename
	unlk	a6
	rts

.globl	__PBGetFInfo
__PBGetFInfo:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	getfinfona
	notl	sp@		/* MSHORT: notw */
getfinfona:
	movel	a0,	sp@-
	btst	#9,	d1
	bne	getfinfohfs
	jbsr	_PBGetFInfo
	unlk	a6
	rts
getfinfohfs:
	jbsr	_PBHGetFInfo
	unlk	a6
	rts

.globl	__PBSetFInfo
__PBSetFInfo:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	setfinfona
	notl	sp@		/* MSHORT: notw */
setfinfona:
	movel	a0,	sp@-
	btst	#9,	d1
	bne	setfinfohfs
	jbsr	_PBSetFInfo
	unlk	a6
	rts
setfinfohfs:
	jbsr	_PBHSetFInfo
	unlk	a6
	rts

.globl	__PBUnmountVol
__PBUnmountVol:
	link	a6,	#0
	movel	a0,	sp@-
	jbsr	_PBUnmountVol
	unlk	a6
	rts

.globl	__PBMountVol
__PBMountVol:
	link	a6,	#0
	movel	a0,	sp@-
	jbsr	_PBMountVol
	unlk	a6
	rts

.globl	__PBAllocate
__PBAllocate:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	allocna
	notl	sp@		/* MSHORT: notw */
allocna:
	movel	a0,	sp@-
	btst	#9,	d1
	bne	allochfs
	jbsr	_PBAllocate
	unlk	a6
	rts
allochfs:
	jbsr	_PBAllocContig	/* just a guess */
	unlk	a6
	rts

.globl	__PBGetEOF
__PBGetEOF:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	geteofna
	notl	sp@		/* MSHORT: notw */
geteofna:
	movel	a0,	sp@-
	jbsr	_PBGetEOF
	unlk	a6
	rts

.globl	__PBSetEOF
__PBSetEOF:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	seteofna
	notl	sp@		/* MSHORT: notw */
seteofna:
	movel	a0,	sp@-
	jbsr	_PBSetEOF
	unlk	a6
	rts

.globl	__PBFlushVol
__PBFlushVol:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	flushvna
	notl	sp@		/* MSHORT: notw */
flushvna:
	movel	a0,	sp@-
	jbsr	_PBFlushVol
	unlk	a6
	rts

.globl	__PBGetVol
__PBGetVol:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	getvona
	notl	sp@		/* MSHORT: notw */
getvona:
	movel	a0,	sp@-
	btst	#9,	d1
	bne	getvohfs
	jbsr	_PBGetVol
	unlk	a6
	rts
getvohfs:
	jbsr	_PBHGetVol
	unlk	a6
	rts

.globl	__PBSetVol
__PBSetVol:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	setvona
	notl	sp@		/* MSHORT: notw */
setvona:
	movel	a0,	sp@-
	btst	#9,	d1
	bne	setvohfs
	jbsr	_PBSetVol
	unlk	a6
	rts
setvohfs:
	jbsr	_PBHSetVol
	unlk	a6
	rts

.globl	__PBEject
__PBEject:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	ejecna
	notl	sp@		/* MSHORT: notw */
ejecna:
	movel	a0,	sp@-
	jbsr	_PBEject
	unlk	a6
	rts

.globl	__PBGetFPos
__PBGetFPos:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	getfposna
	notl	sp@		/* MSHORT: notw */
getfposna:
	movel	a0,	sp@-
	jbsr	_PBGetFPos
	unlk	a6
	rts

.globl	__PBOffLine
__PBOffLine:
	link	a6,	#0
	movel	a0,	sp@-
	jbsr	_PBOffLine
	unlk	a6
	rts

.globl	__PBSetFLock
__PBSetFLock:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	setflna
	notl	sp@		/* MSHORT: notw */
setflna:
	movel	a0,	sp@-
	btst	#9,	d1
	bne	setflhfs
	jbsr	_PBSetFLock
	unlk	a6
	rts
setflhfs:
	jbsr	_PBHSetFLock
	unlk	a6
	rts

.globl	__PBRstFLock
__PBRstFLock:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	rstflna
	notl	sp@		/* MSHORT: notw */
rstflna:
	movel	a0,	sp@-
	btst	#9,	d1
	bne	rstflhfs
	jbsr	_PBRstFLock
	unlk	a6
	rts
rstflhfs:
	jbsr	_PBHRstFLock
	unlk	a6
	rts

.globl	__PBSetFVers
__PBSetFVers:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	setfvna
	notl	sp@		/* MSHORT: notw */
setfvna:
	movel	a0,	sp@-
	jbsr	_PBSetFVers
	unlk	a6
	rts

.globl	__PBSetFPos
__PBSetFPos:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	setfpna
	notl	sp@		/* MSHORT: notw */
setfpna:
	movel	a0,	sp@-
	jbsr	_PBSetFPos
	unlk	a6
	rts

.globl	__PBFlushFile
__PBFlushFile:
	link	a6,	#0
	clrl	sp@-		/* MSHORT: clrw */
	btst	#10,	d1
	beq	flushfna
	notl	sp@		/* MSHORT: notw */
flushfna:
	movel	a0,	sp@-
	jbsr	_PBFlushFile
	unlk	a6
	rts

/* end of File Manager */

.globl	__GetTrapAddress
__GetTrapAddress:

/*
 * NOTE: this ugly hack is for A-train, cause it's brain damaged
 *	 and it loads the contents of 0xA198 and 0xA89F!
 */
	cmpl	@(0xA198),	d0
	bne	1f
	movel	#0xA198,	d0
1:
	cmpl	@(0xA89F),	d0
	bne	1f
	movel	#0xA89F,	d0
1:
/* End of Icky Hack */

	btst	#9,	d1
	beq	old
	btst	#10,	d1
	bne	tool
os:
	andil	#0xff,	d0

	cmpl	#0x77,	d0	/* CountADBs: we don't support it */
	beq	unimp
	cmpl	#0x78,	d0	/* GetIndADB: we don't support it */
	beq	unimp
	cmpl	#0x79,	d0	/* GetADBInfo: we don't support it */
	beq	unimp
	cmpl	#0x7A,	d0	/* SetADBInfo: we don't support it */
	beq	unimp
	cmpl	#0x7B,	d0	/* ADBReInit: we don't support it */
	beq	unimp
	cmpl	#0x7C,	d0	/* ADBOp: we don't support it */
	beq	unimp
	cmpl	#0x3D,	d0	/* DrvrInstall: we don't support it */
	beq	unimp
	cmpl	#0x3E,	d0	/* DrvrRemove: we don't support it */
	beq	unimp
	cmpl	#0x4F,	d0	/* RDrvrInstall: we don't support it */
	beq	unimp

	asll	#2,	d0
	lea	_ostraptable,	a0
	movel	a0@(d0),	a0
	cmpw	#0x4AFC,	a0@
	beq	unimp
	clrl	d0
	rts
tool:
	andil	#0x3ff,	d0
	cmpl	#0x30,	d0	/* Pack14: we don't support it, but we */
	beq	unimp		/* don't have it start with illegal because */
				/* EqnEditor (bundled w/Word 5.0) blindly */
				/* uses it! */
	cmpl	#0xB5,	d0	/* ScriptUtil: we don't support it */
	beq	unimp
	asll	#2,	d0
	lea	_tooltraptable,	a0
	movel	a0@(d0),	a0
	cmpw	#0x4AFC,	a0@
	beq	unimp
	clrl	d0
	rts
unimp:
	lea	_P_Unimplemented, a0
	clrl	d0
	rts
old:
	andil	#0x1ff,	d0
	cmpw	#0x4f,	d0	/* 0x00 - 0x4F were OS */
	ble	os
	cmpw	#0x54,	d0	/* 0x54: UprString, OS */
	beq	os
	cmpw	#0x57,	d0	/* 0x57: SetApplBase, OS */
	beq	os
	bra	tool

.globl	__SetTrapAddress
__SetTrapAddress:
	btst	#9,	d1
	beq	olds
	btst	#10,	d1
	bne	tools
oss:
	andil	#0xff,	d0
	asll	#2,	d0
	movel	a1,	sp@-
	lea	_ostraptable,	a1
	bra	outofhere

tools:
	andil	#0x3ff,	d0
	asll	#2,	d0
	movel	a1,	sp@-
	lea	_tooltraptable,	a1

outofhere:
	movel	a0,	a1@(d0)

	cmpw	#0xED,	d0
	beq	1f
	moveml	d1/d2/a0,	sp@-
	jsr	_flushcache
	moveml	sp@+,		d1/d2/a0
1:

	movel	sp@+,	a1
	clrl	d0
	rts
olds:
	andil	#0x1ff,	d0
	cmpw	#0x4f,	d0	/* 0x00 - 0x4F were OS */
	ble	oss
	cmpw	#0x54,	d0	/* 0x54: UprString, OS */
	beq	oss
	cmpw	#0x57,	d0	/* 0x57: SetApplBase, OS */
	beq	oss
	bra	tools

.globl	__Gestalt
__Gestalt:
	link	a6,	#-4
	pea	a6@(-4)
	movel	d0,	sp@-
	jbsr	_Gestalt
	movel	a6@(-4),	a0
	unlk	a6
	rts

/* End of hand crafted OStrap stubs */

/* Beginning of os traps we don't support */

.globl	__AddDrive	/* Not Supported */
__AddDrive:
	.word 0x4AFC

.globl	__SlotManager	/* Not Supported */
__SlotManager:
	.word 0x4AFC

.globl	__AttachVBL	/* Not Supported */
__AttachVBL:
	.word 0x4AFC

.globl	__DoVBLTask	/* Not Supported */
__DoVBLTask:
	.word 0x4AFC

.globl	__DTInstall	/* Not Supported */
__DTInstall:
	.word 0x4AFC

.globl	__SIntRemove	/* Not Supported */
__SIntRemove:
	.word 0x4AFC

.globl	__InternalWait	/* Not Supported */
__InternalWait:
	.word 0x4AFC

.globl	__SIntInstall	/* Not Supported */
__SIntInstall:
	.word 0x4AFC

.globl _GetLogInInfo
_GetLogInInfo:
	.word 0x4AFC

.globl _GetDirAccess
_GetDirAccess:
	.word 0x4AFC

.globl _SetDirAccess
_SetDirAccess:
	.word 0x4AFC

.globl _MapID
_MapID:
	.word 0x4AFC

.globl _MapName
_MapName:
	.word 0x4AFC

.globl _CopyFile
_CopyFile:
	.word 0x4AFC

.globl _MoveRename
_MoveRename:
	.word 0x4AFC

.globl _OpenRFDeny
_OpenRFDeny:
	.word 0x4AFC

.globl _P_PrPurge
_P_PrPurge:
	.word	0x4AFC

.globl _P_PrNoPurge
_P_PrNoPurge:
	.word	0x4AFC

.globl _IMVI_ReadXPRam
_IMVI_ReadXPRam:
	.word	0x4AFC

.globl _IMVI_WriteXPRam
_IMVI_WriteXPRam:
	.word	0x4AFC

.globl _IMVI_MemoryDispatch
_IMVI_MemoryDispatch:
	.word	0x4AFC

.globl _IMVI_IdleUpdate
_IMVI_IdleUpdate:
	.word	0x4AFC

.globl _IMVI_SlpQInstall
_IMVI_SlpQInstall:
	.word	0x4AFC

.globl _IMVI_CommToolboxDispatch
_IMVI_CommToolboxDispatch:
	.word	0x4AFC

.globl _IMVI_DebugUtil
_IMVI_DebugUtil:
	.word	0x4AFC

.globl _IMVI_DeferUserFn
_IMVI_DeferUserFn:
	.word	0x4AFC

.globl _IMVI_Translate24To32
_IMVI_Translate24To32:
	.word	0x4AFC

.globl _IMVI_PPC
_IMVI_PPC:
	.word	0x4AFC

/* End of os traps we don't support */
