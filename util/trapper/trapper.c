BEGIN(PBMountVol, ParmBlkPtr)
  IN(ioVRefNum);
  OUT(ioResult);
  OUT(ioVRefNum);
END()

BEGIN(PBGetVInfo, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioVolIndex);
  OUT(ioResult);
  OUT(ioNamePtr);
  OUT(ioVRefNum);
  OUT(ioVCrDate);
  OUT(ioVLsBkUp);
  OUT(ioVAtrb);
  OUT(ioVNmFls);
  OUT(ioVDirSt);
  OUT(ioVBlLn);
  OUT(ioVNmAlBlks);
  OUT(ioVAlBlkSiz);
  OUT(ioVClpSiz);
  OUT(ioAlBlSt);
  OUT(ioVNxtFNum);
  OUT(ioVFRBlk);
END()

BEGIN(PBHGetVInfo, HParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioVolIndex);
  OUT(ioResult);
  OUT(ioNamePtr);
  OUT(ioVRefNum);
  OUT(ioVCrDate);
  OUT(ioVLsBkUp);
  OUT(ioVAtrb);
  OUT(ioVNmFls);
  OUT(ioVBitMap);
  OUT(ioVAllocPtr);
  OUT(ioVNmAlBlks);
  OUT(ioVAlBlkSiz);
  OUT(ioVClpSiz);
  OUT(ioAlBlSt);
  OUT(ioVNxtFNum);
  OUT(ioVFRBlk);
  OUT(ioVSigWord);
  OUT(ioVDrvInfo);
  OUT(ioVDRefNum);
  OUT(ioVFSID);
  OUT(ioVBkUp);
  OUT(ioVSeqNum);
  OUT(ioVWrCnt);
  OUT(ioVFilCnt);
  OUT(ioVDirCnt);
  OUT(ioVFndrInfo);
END()

BEGIN(PBSetVInfo, HParmBlkPtr)
  IN(ioCompletion);
  IN(NamePtr);
  IN(VRefNum);
  IN(VCrDate);
  IN(VLsMod);
  IN(VAtrb);
  IN(VClpSiz);
  IN(VBkUp);
  IN(VSeqNum);
  IN(VFndrInfo);
  OUT(ioResult);
END()

BEGIN(PBGetVol, ParmBlkPtr)
  IN(ioCompletion);
  OUT(ioResult);
  OUT(ioNamePtr);
  OUT(ioVRefNum);
END()

BEGIN(PBHGetVol, WDPBPtr)
  IN(ioCompletion);
  OUT(ioResult);
  OUT(ioNamePtr);
  OUT(ioVRefNum);
  OUT(ioWDProcID);
  OUT(ioWDVRefNum);
  OUT(ioWDDirID);
END()

BEGIN(PBSetVol, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  OUT(ioResult);
END()

BEGIN(PBHSetVol, WDPBPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioWDDirID);
  OUT(ioResult);
END()

BEGIN(PBFlushVol, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  OUT(ioResult);
END()

BEGIN(PBUnmountVol, ParmBlkPtr)
  IN(ioNamePtr);
  IN(ioVRefNum);
  OUT(ioResult);
END()

BEGIN(PBOffLine, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  OUT(ioResult);
END()

BEGIN(PBEject, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  OUT(ioResult);
END()

BEGIN(PBOpen, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioVersNum);
  IN(ioPermssn);
  IN(ioMisc);
  OUT(ioResult);
  OUT(ioRefNum);
END()

BEGIN(PBHOpen, HParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioPermssn);
  IN(ioMisc);
  IN(ioDirID);
  OUT(ioResult);
  OUT(ioRefNum);
END()

BEGIN(PBOpenRF, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioVersNum);
  IN(ioPermssn);
  IN(ioMisc);
  OUT(ioResult);
  OUT(ioRefNum);
END()

BEGIN(PBHOpenRF, HParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioPermssn);
  IN(ioMisc);
  IN(ioDirID);
  OUT(ioResult);
  OUT(ioRefNum);
END()

BEGIN(PBLockRange, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioRefNum);
  IN(ioReqCount);
  IN(ioPosMode);
  IN(ioPosOffset);
  OUT(ioResult);
END()

BEGIN(PBUnlockRange, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioRefNum);
  IN(ioReqCount);
  IN(ioPosMode);
  IN(ioPosOffset);
  OUT(ioResult);
END()

BEGIN(PBRead, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioRefNum);
  IN(ioBuffer);
  IN(ioReqCount);
  IN(ioPosMode);
  IN(ioPosOffset);
  OUT(ioResult);
  OUT(ioActCount);
  OUT(ioPosOffset);
END()

BEGIN(PBWrite, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioRefNum);
  IN(ioBuffer);
  IN(ioReqCount);
  IN(ioPosMode);
  IN(ioPosOffset);
  OUT(ioResult);
  OUT(ioActCount);
  OUT(ioPosOffset);
END()

BEGIN(PBGetFPos, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioRefNum);
  OUT(ioResult);
  OUT(ioReqCount);
  OUT(ioActCount);
  OUT(ioPosMode);
  OUT(ioPosOffset);
END()

BEGIN(PBSefFPos, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioRefNum);
  IN(ioPosMode);
  IN(ioPosOffset);
  OUT(ioResult);
  OUT(ioPosOffset);
END()

BEGIN(PBGetEOF, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioRefNum);
  OUT(ioResult);
  OUT(ioMisc);
END()

BEGIN(PBSetEOF, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioRefNum);
  IN(ioMisc);
  OUT(ioResult);
END()

BEGIN(PBAllocate, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioRefNum);
  IN(ioReqCount);
  OUT(ioResult);
  OUT(ioActCount);
END()

BEGIN(PBAllocContig, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioRefNum);
  IN(ioReqCount);
  OUT(ioResult);
  OUT(ioActCount);
END()

BEGIN(PBFlushFile, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioRefNum);
  OUT(ioResult);
END()

BEGIN(PBClose, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioRefNum);
  OUT(ioResult);
END()

BEGIN(PBCreate, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioFVersNum);
  OUT(ioResult);
END()

BEGIN(PBHCreate, HParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioDirID);
  OUT(ioResult);
END()

BEGIN(PBDirCreate, HParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioDirID);
  OUT(ioResult);
  OUT(ioDirID);
END()

BEGIN(PBDelete, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioFVersNum);
  OUT(ioResult);
END()

BEGIN(PBHDelete, HParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioDirID);
  OUT(ioResult);
END()

BEGIN(PBGetFInfo, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioFVersNum);
  IN(ioFDirIndex);
  OUT(ioResult);
  OUT(NamePtr);
  OUT(FRefNum);
  OUT(FlAttrib);
  OUT(FlVersNum);
  OUT(FlFndrInfo);
  OUT(FlNum);
  OUT(FlStBlk);
  OUT(FlLgLen);
  OUT(FlPyLen);
  OUT(FlRStBlk);
  OUT(FlRLgLen);
  OUT(FlRPyLen);
  OUT(FlCrDat);
  OUT(FlMdDat);
END()

BEGIN(PBHGetFInfo, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioFDirIndex);
  IN(ioDirID);
  OUT(ioResult);
  OUT(NamePtr);
  OUT(FRefNum);
  OUT(FlAttrib);
  OUT(FlFndrInfo);
  OUT(ioDirID);
  OUT(FlStBlk);
  OUT(FlLgLen);
  OUT(FlPyLen);
  OUT(FlRStBlk);
  OUT(FlRLgLen);
  OUT(FlRPyLen);
  OUT(FlCrDat);
  OUT(FlMdDat);
END()

BEGIN(PBSetFInfo, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioFVersNum);
  IN(ioFlFndrInfo);
  IN(ioFlCrDat);
  IN(ioFlMdDat);
  OUT(ioResult);
END()

BEGIN(PBHSetFInfo, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioFlFndrInfo);
  IN(ioDirID);
  IN(ioFlCrDat);
  IN(ioFlMdDat);
  OUT(ioResult);
END()

BEGIN(PBSetFLock, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioFVersNum);
  OUT(ioResult);
END()

BEGIN(PBHSetFLock, HParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioDirID);
  OUT(ioResult);
END()

BEGIN(PBRstFLock, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioFVersNum);
  OUT(ioResult);
END()

BEGIN(PBHRstFLock, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioDirID);
  OUT(ioResult);
END()

BEGIN(PBSetFVers, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioVersNum);
  IN(ioMisc);
  OUT(ioResult);
END()

BEGIN(PBRename, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioVersNum);
  IN(ioMisc);
  OUT(ioResult);
END()

BEGIN(PBHRename, ParmBlkPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioMisc);
  IN(ioDirID);
  OUT(ioResult);
END()

BEGIN(PBGetCatInfo, CInfoPBPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioFDirIndex);
  IN(ioDirID);
  OUT(ioResult);
  OUT(ioNamePtr);
  OUT(ioFRefNum);
  OUT(ioFlAttrib);
  if (???->ioFlAttrib & ???)
    {
      OUT(ioFlFndrInfo);
      OUT(ioDirID
      OUT(FlStBlk);
      OUT(FlLgLen);
      OUT(FlPyLen);
      OUT(FlRStBlk);
      OUT(FlRLgLen);
      OUT(FlRPyLen);
      OUT(FlCrDat);
      OUT(FlMdDat);
      OUT(FlBkDat);
      OUT(FlXFndrInfo);
      OUT(FlParID);
      OUT(FlClpSiz);
    }
  else
    {
      OUT(ioDrUsrWds);
      OUT(ioDrDirID);
      OUT(ioDrNmFls);
      OUT(ioDrCrDat);
      OUT(ioDrMdDat);
      OUT(ioDrBkDat);
      OUT(ioDrFndrInfo);
      OUT(ioDrParID);
    }
END()

BEGIN(PBSetCatInfo, CInfoPBPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioFlAttrib);
  IN(ioFlFndrInfo);
  IN(ioDirID);
  IN(ioFlCrDat);
  IN(ioFlMdDat);
  IN(ioFlBkDat);
  IN(ioFlXFndrInfo);
  IN(ioFlClpSiz);
  OUT(ioResult);
  OUT(ioNamePtr);
END()

BEGIN(PBCatMove, CMovePBPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioNewName);
  IN(ioNewDirID);
  IN(ioDirID);
  OUT(ioResult);
END()

BEGIN(PBOpenWD, WDPBPtr)
  IN(ioCompletion);
  IN(ioNamePtr);
  IN(ioVRefNum);
  IN(ioWDProcID);
  IN(ioWDDirID);
  OUT(ioResult);
  OUT(ioVRefNum);
END()

BEGIN(PBCloseWD, WDPBPtr)
  IN(ioCompletion);
  IN(ioVRefNum);
  OUT(ioResult);
END()

BEGIN(PBGetWDInfo, WDPBPtr)
  IN(ioCompletion);
  IN(ioVRefNum);
  IN(ioWDIndex);
  IN(ioWDProcID);
  IN(ioWDVRefNum);
  OUT(ioResult);
  OUT(ioNamePtr);
  OUT(ioVRefNum);
  OUT(ioWDProcID);
  OUT(ioWDVRefNum);
END()

BEGIN(PBGetFCBInfo, FCBPBPtr)
  IN(ioCompletion);
  IN(ioVRefNum);
  IN(ioRefNum);
  IN(ioFCBIndx);
  OUT(ioResult);
  OUT(ioNamePtr);
  OUT(ioVRefNum);
  OUT(ioRefNum);
  OUT(ioFCBFlNm);
  OUT(ioFCBFlags);
  OUT(ioFCBStBlk);
  OUT(ioFCBEOF);
  OUT(ioFCBPLen);
  OUT(ioFCBCrPs);
  OUT(ioFCBVRefNum);
  OUT(ioFCBClpSiz);
  OUT(ioFCBParID);
END()

trap_handler (void)
{
  /* save all registers */
  /* call the appropriate input logger */
  /* restore all registers */
  /* do the trap */
  /* save all registers */
  /* call the appropriate output logger */
}
