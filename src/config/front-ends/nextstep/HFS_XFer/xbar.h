extern BOOLEAN xWaitNextEvent(INTEGER em, EventRecord *evt, LONGINT sleep,
							  RgnHandle mousergn);
extern BOOLEAN xGetNextEvent(INTEGER em, EventRecord *evt);

extern OSErr xPBHRename(HFileParam *pb, BOOLEAN async);
extern OSErr xPBHCreate(HFileParam *pb, BOOLEAN async);
extern OSErr xPBDirCreate(HFileParam *pb, BOOLEAN async);
extern OSErr xPBHDelete(HFileParam *pb, BOOLEAN async);
extern OSErr xPBRead(ioParam *pb, BOOLEAN async);
extern OSErr xPBWrite(ioParam *pb, BOOLEAN async);
extern OSErr xPBClose(ioParam *pb, BOOLEAN async);
extern OSErr xPBHOpen(HFileParam *pb, BOOLEAN async);
extern OSErr xPBHOpenRF(HFileParam *pb, BOOLEAN async);
extern OSErr xPBGetCatInfo(CInfoPBPtr pb, BOOLEAN async);
extern OSErr xPBSetCatInfo(CInfoPBPtr pb, BOOLEAN async);
extern OSErr xPBCatMove(CMovePBPtr pb, BOOLEAN async);
extern OSErr xPBGetVInfo(volumeParam *pb, BOOLEAN async);
extern OSErr xPBUnmountVol(volumeParam *pb);
extern OSErr xPBEject(volumeParam *pb);
extern OSErr xPBAllocate(ParmBlkPtr pb, BOOLEAN async);
extern OSErr xPBAllocContig(ParmBlkPtr pb, BOOLEAN async);
extern OSErr xPBHGetFInfo(ParmBlkPtr pb, BOOLEAN async);
extern OSErr xPBSetEOF(ParmBlkPtr pb, BOOLEAN async);

extern OSErr myPBHRename(HFileParam *pb, BOOLEAN async);
extern OSErr myPBHCreate(HFileParam *pb, BOOLEAN async);
extern OSErr myPBDirCreate(HFileParam *pb, BOOLEAN async);
extern OSErr myPBHDelete(HFileParam *pb, BOOLEAN async);
extern OSErr myPBRead(ioParam *pb, BOOLEAN async);
extern OSErr myPBWrite(ioParam *pb, BOOLEAN async);
extern OSErr myPBClose(ioParam *pb, BOOLEAN async);
extern OSErr myPBHOpen(HFileParam *pb, BOOLEAN async);
extern OSErr myPBHOpenRF(HFileParam *pb, BOOLEAN async);
extern OSErr myPBGetCatInfo(CInfoPBPtr pb, BOOLEAN async);
extern OSErr myPBSetCatInfo(CInfoPBPtr pb, BOOLEAN async);
extern OSErr myPBCatMove(CMovePBPtr pb, BOOLEAN async);
extern OSErr myPBGetVInfo(volumeParam *pb, BOOLEAN async);
extern OSErr myPBUnmountVol(volumeParam *pb);
extern OSErr myPBEject(volumeParam *pb);
extern OSErr myPBAllocate(ioParam *pb, BOOLEAN async);
extern OSErr myPBAllocContig(ioParam *pb, BOOLEAN async);
extern OSErr myPBHGetFInfo(HFileParam *pb, BOOLEAN async);
extern OSErr myPBSetEOF(ioParam *pb, BOOLEAN async);

extern OSErr myPBOpenWD(WDPBPtr pb, BOOLEAN async);
extern OSErr myPBCloseWD(WDPBPtr pb, BOOLEAN async);
extern OSErr myPBFlushFile(ioParam *pb, BOOLEAN async);
