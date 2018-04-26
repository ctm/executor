/*
 * Check the size of all the structures that we use that correspond to Mac
 * memory (i.e. layouts imposed by the Mac ABI).  It would be nice if we
 * checked the size and offset of each member element too, but this is a good
 * start and something Executor should have done from the beginning, since
 * if a structure doesn't get packed properly it can be a pain to track the
 * issue down in a debugger.
 *
 * WARNING: The sizes that it is checking against are not known to be correct.
 *          In most cases they are correct, however, the sizes have been
 *          gleaned from a port that hasn't been tested much, so there are two
 *          potential sources of bad sizes.  One is we may have structs that
 *          we've never used.  The other is we may have structs that have been
 *          used on our more thoroughly tested ports that have managed to
 *          become incorrect on the port we generated the numbers on.
 */

#include <rsys/common.h>
#include <rsys/check_structs.h>

#include "ADB.h"
#include "AliasMgr.h"
#include "AppleEvents.h"
#include "AppleTalk.h"
#include "BinaryDecimal.h"
#include "CommTool.h"
#include "Components.h"
#include "ControlMgr.h"
#include "CQuickDraw.h"
#include "DeskMgr.h"
#include "DeviceMgr.h"
#include "DialogMgr.h"
#include "Disk.h"
#include "DiskInit.h"
#include "EditionMgr.h"
#include "EventMgr.h"
#include "FileMgr.h"
#include "Finder.h"
#include "FontMgr.h"
#include "Gestalt.h"
#include "HelpMgr.h"
#include "Iconutil.h"
#include "IntlUtil.h"
#include "ListMgr.h"
#include "ExMacTypes.h"
#include "MemoryMgr.h"
#include "MenuMgr.h"
#include "NotifyMgr.h"
#include "OSEvent.h"
#include "OSUtil.h"
#include "Package.h"
#include "PPC.h"
#include "PrintMgr.h"
#include "ProcessMgr.h"
#include "QuickDraw.h"
#include "QuickTime.h"
#include "ResourceMgr.h"
#include "SANE.h"
#include "ScrapMgr.h"
#include "ScriptMgr.h"
#include "SegmentLdr.h"
#include "Serial.h"
#include "ShutDown.h"
#include "SoundDvr.h"
#include "SoundMgr.h"
#include "StartMgr.h"
#include "StdFilePkg.h"
#include "SysErr.h"
#include "TextEdit.h"
#include "TimeMgr.h"
#include "ToolboxEvent.h"
#include "ToolboxUtil.h"
#include "VDriver.h"
#include "VRetraceMgr.h"
#include "WindowMgr.h"

#include <rsys/alias.h>
#include <rsys/apple_events.h>
#include <rsys/cfm.h>
#include <rsys/cquick.h>
#include <rsys/ctl.h>
#include <rsys/dial.h>
#include <rsys/emustubs.h>
#include <rsys/filedouble.h>
#include <rsys/file.h>
#include <rsys/float.h>
#include <rsys/font.h>
#include <rsys/hfs.h>
#include <rsys/hfs_plus.h>
#include <rsys/icon.h>
#include <rsys/itm.h>
#include <rsys/keyboard.h>
#include <rsys/launch.h>
#include <rsys/menu.h>
#include <rsys/mixed_mode.h>
#include <rsys/mman_private.h>
#include <rsys/nextprint.h>
#include <rsys/partition.h>
#include <rsys/pef.h>
#include <rsys/picture.h>
#include <rsys/print.h>
#include <rsys/process.h>
#include <rsys/quick.h>
#include <rsys/resource.h>
#include <rsys/screen-dump.h>
#include <rsys/segment.h>
#include <rsys/serial.h>
#include <rsys/soundopts.h>
#include <rsys/syserr.h>
#include <rsys/tesave.h>
#include <rsys/toolevent.h>
#include <rsys/wind.h>

#define check(type, expected_size)                                             \
    do                                                                         \
    {                                                                          \
        if(sizeof(type) != expected_size)                                      \
            fprintf(stderr, "Expected sizeof(" #type ") to be %u, got %u\n", \
                    (unsigned)expected_size, (unsigned) sizeof(type));                      \
    } while(false)

using namespace Executor;

void Executor::check_structs(void)
{
    /* ADB.h */
    check(ADBDataBlock, 10);
    check(ADBSetInfoBlock, 8);

    /* AliasMgr.h has no structs or unions */

    /* AppleEvents.h */
    check(AEArrayData, 12);
    check(AEDesc, 8);
    check(AEKeyDesc, 12);
    check(AE_hdlr_t, 8);
    check(AE_hdlr_selector_t, 8);
    check(AE_hdlr_table_elt_t, 24);
    check(AE_hdlr_table_t, 52);
    check(AE_zone_tables_t, 56);
    check(AE_info_t, 596);
    check(AEArrayData, 12);

    /* AppleTalk.h has no structs or unions */

    /* BinaryDecimal.h has no structs or unions */

    /* CommTool.h */
    check(CRMRec, 34);
    check(CRMSerialRecord, 30);

    /* Components.h */
    check(ComponentRecord, 4);
    check(ComponentInstanceRecord, 4);

    /* ControlMgr.h */
    check(ControlRecord, 296);
    check(CtlCTab, 16);
    check(AuxCtlRec, 22);

    /* CQuickDraw.h */
    check(ITab, 8);
    check(SProcRec, 8);
    check(CProcRec, 8);
    check(GDevice, 62);
    check(ColorInfo, 16);
    check(Palette, 32);
    check(ReqListRec, 4);
    check(OpenCPicParams, 24);
    check(CommentSpec, 4);
    check(FontSpec, 26);
    check(PictInfo, 104);

    /* DeskMgr.h has no structs or unions */

    /* DeviceMgr.h */
    check(ramdriver, 20);
    check(DCtlEntry, 40);

    /* DialogMgr.h */
    check(DialogRecord, 170);
    check(DialogTemplate, 276);
    check(StageList, 2);
    check(AlertTemplate, 12);

    /* Disk.h */
    check(DrvSts, 22);

    /* DiskInit.h has no structs or unions */

    /* EditionMgr.h */
    check(struct SectionRecord, 36);
    check(EditionContainerSpec, 110);
    check(EditionInfoRecord, 126);
    check(NewPublisherReply, 122);
    check(NewSubscriberReply, 112);
    check(SectionOptionsReply, 10);
    check(EditionOpenerParamBlock, 148);
    check(FormatIOParamBlock, 24);

    /* EventMgr.h */
    check(EventRecord, 16);

    /* FileMgr.h */
    check(FInfo, 16);
    check(FXInfo, 16);
    check(DInfo, 16);
    check(DXInfo, 16);
    check(IOParam, 50);
    check(FileParam, 80);
    check(VolumeParam, 64);
    check(CntrlParam, 50);
    check(ParamBlockRec, 80);
    check(HIoParam, 50);
    check(HFileParam, 80);
    check(HVolumeParam, 122);
    check(HParamBlockRec, 122);
    check(HFileInfo, 108);
    check(DirInfo, 104);
    check(CInfoPBRec, 108);
    check(CMovePBRec, 52);
    check(WDPBRec, 52);
    check(FCBPBRec, 62);
    check(VCB, 178);
    check(DrvQEl, 16);
    check(FSSpec, 70);

    /* Finder.h */
    check(DTPBRec, 104);

    /* FontMgr.h */
    check(FMetricRec, 20);
    check(FamRec, 52);
    check(WidthTable, 1072);
    check(FMInput, 16);
    check(FMOutput, 26);
    check(FontRec, 26);

    /* Gestalt.h has no structs or unions */

    /* HelpMgr.h */
    check(HMStringResType, 4);
    check(HMMessageRecord, 258);

    /* Iconutil.h */
    check(CIcon, 84);

    /* IntlUtil.h */
    check(Intl0Rec, 32);
    check(Intl1Rec, 332);

    /* ListMgr.h */
    check(ListRec, 88);

    /* ExMacTypes.h */
    check(QHdr, 10);
    check(Point, 4);
    check(Rect, 8);

    /* MemoryMgr.h */
    check(Zone, 54);

    /* MenuMgr.h */
    check(MenuInfo, 270);
    check(MCEntry, 30);

    /* NotifyMgr.h */
    check(NMRec, 36);

    /* OSEvent.h */
    check(EvQEl, 22);
    check(TargetID, 252);
    check(HighLevelEventMsg, 36);

    /* OSUtil.h */
    check(SysParmType, 20);
    check(DateTimeRec, 14);
    check(SysEnvRec, 16);
    check(QElem, 178);

    /* Package.h has no structs or unions */

    /* PPC.h */
    check(LocationNameRec, 104);
    check(PPCPortRec, 72);

    /* PrintMgr.h */
    check(TPrPort, 178);
    check(TPrInfo, 14);
    check(TPrStl, 8);
    check(TPrXInfo, 16);
    check(TPrJob, 20);
    check(TPrint, 120);
    check(TPrStatus, 26);
    check(TPrDlg, 204);

    /* ProcessMgr.h */
    check(ProcessSerialNumber, 8);
    check(LaunchParamBlockRec, 44);
    check(ProcessInfoRec, 60);

    /* QuickDraw.h */
    check(Region, 10);
    check(BitMap, 14);
    check(Cursor, 68);
    check(Polygon, 14);
    check(FontInfo, 8);
    check(QDProcs, 52);
    check(GrafPort, 108);
    check(Picture, 10);
    check(PenState, 18);
    check(RGBColor, 6);
    check(HSVColor, 6);
    check(HSLColor, 6);
    check(CMYColor, 6);
    check(ColorSpec, 8);
    check(ColorTable, 16);
    check(CQDProcs, 80);
    check(PixMap, 50);
    check(PixPat, 28);
    check(CGrafPort, 108);
    check(CCrsr, 96);
    check(MatchRec, 10);

    /* QuickTime.h */
    check(MovieRecord, 4);

    /* ResourceMgr.h has no structs or unions */

    /* SANE.h */
    check(comp_t, 8);
    check(native_comp_t, 8);
    check(x80_t, 10);
#if defined(mc68000)
    check(extended96, 12);
#endif /* defined (mc68000) */
    check(Decimal, 24);
    check(DecForm, 4);

    /* ScrapMgr.h */
    check(ScrapStuff, 16);

    /* ScriptMgr.h */
    check(DateCacheRec, 512);
    check(LongDateRec, 28);
    check(NumFormatStringRec, 256);
    check(WideChar, 2);
    check(WideCharArr, 22);
    check(NumberParts, 172);
    check(Extended80, 10);
    check(ToggleResults, 2);
    check(LongDateField, 1);
    check(DateDelta, 1);
    check(TogglePB, 28);

    /* SegmentLdr.h */
    check(AppFile, 264);

    /* Serial.h */
    check(SerShk, 8);
    check(SerStaRec, 8);

    /* ShutDown.h has no structs or unions */

    /* SoundDvr.h */
    check(FFSynthRec, 30008);
    check(Tone, 6);
    check(SWSynthRec, 30008);
    check(FTSoundRec, 50);
    check(FTSynthRec, 6);

    /* SoundMgr.h */
    check(SndCommand, 8);
    check(SndChannel, 1060);
    check(soundbuffer_t, 24);
    check(SoundHeader, 24);
    check(ExtSoundHeader, 66);
    check(SndDoubleBuffer, 18);
    check(SndDoubleBufferHeader, 24);
    check(SCStatus, 24);

    /* StartMgr.h */
    check(DefStartRec, 4);
    check(DefVideoRec, 2);
    check(DefOSRec, 2);

    /* StdFilePkg.h */
    check(SFReply, 74);
    check(StandardFileReply, 88);

    /* SysErr.h has no structs or unions */

    /* TextEdit.h */
    check(TERec, 98);
    check(StyleRun, 4);
    check(STElement, 18);
    check(LHElement, 4);
    check(TextStyle, 12);
    check(ScrpSTElement, 20);
    check(StScrpRec, 22);
    check(NullSTRec, 8);
    check(TEStyleRec, 24);

    /* TimeMgr.h */
    check(TMTask, 14);

    /* ToolboxEvent.h has no structs or unions */

    /* ToolboxUtil.h */
    check(Int64Bit, 8);

#if defined(USE_VDRIVER_H)
    /* VDriver.h */
    check(VDParamBlock, 0); /* TODO*/
    check(VDEntryRecord, 0); /* TODO */
    check(VDGammaRecord, 0); /* TODO */
    check(VDPgInfo, 0); /* TODO */
    check(VDFlagRec, 0); /* TODO */
    check(VDDefModeRec, 0); /* TODO */
#endif /* defined(USE_VDRIVER_H) */

    /* VRetraceMgr.h */
    check(VBLTask, 14);

    /* WindowMgr.h */
    check(WindowRecord, 156);
    check(WStateData, 16);
    check(AuxWinRec, 28);

    /* rsys/alias.h */
    check(Str27, 28);
    check(alias_head_t, 150);
    check(alias_parent_t, 4);
    check(alias_unknown_000100_t, 10);
    check(alias_fullpath_t, 4);
    check(alias_tail_t, 170);
    check(alias_parsed_t, 20);

    /* rsys/apple_events.h */
    check(inline_desc_t, 8);
    check(inline_key_desc_t, 12);
    check(list_header_t, 24);
    check(ae_header_t, 66);

    /* rsys/cfm.h */
    check(cfrg_resource_t, 32);
    check(cfir_t, 44);
    check(MemFragment, 12);
    check(DiskFragment, 12);
    check(SegmentedFragment, 12);
    check(FragmentLocator, 16);
    check(InitBlock, 36);
    check(section_info_t, 16);
    check(CFragConnection_t, 28);
    check(lib_t, 12);
    check(CFragClosure_t, 4);
    //check(map_entry_t, 8);     // not a GUEST struct, contains native pointers

    /* rsys/cquick.h */
    check(GrafVars, 26);

    /* rsys/ctl.h */
    check(struct popup_data, 12);
    check(thumbstr, 18);
    check(contrlrestype, 24);
    check(struct lsastr, 18);

    /* rsys/emustubs.h */
    check(adbop_t, 12);
    check(comm_toolbox_dispatch_args_t, 12);
    check(initzonehiddenargs_t, 14);

    ///* rsys/dial.h */
    //check (icon_item_template_t, 18);

    /* rsys/filedouble.h */
    check(Single_descriptor, 12);
    check(Single_header, 26);
    check(Single_dates, 16);
    check(Single_finfo, 32);
    check(Single_attribs, 4);
    check(defaulthead_t, 146);
    check(defaultentries_t, 52);

    /* rsys/file.h */
    check(hfs_access_t, 16);
    //check(DrvQExtra, 46); // not a GUEST struct, contains native pointers
    check(fcbrec, 94);
    check(fcbhidden, 32714);
    //check(VCBExtra, 202);   // not a GUEST struct, contains native pointers
    check(getvolparams_info_t, 20);

/* rsys/float.h */
#if defined(mc68000)
    check(m68k_x96_t, 12);
#endif /* defined (mc68000) */

#if defined(i386)
    check(i386_x80_t, 10);
#endif /* defined (i386) */

#if defined(__alpha)
    check(alpha_x64_t, 8);
#endif /* defined(__alpha) */

    check(f64_t, 8);
    check(f32_t, 4);
    check(native_f32_t, 4);

    /* rsys/font.h */
    check(fatabentry, 6);
    check(widentry_t, 4);

    /* rsys/hfs.h */
    check(xtntdesc, 4);
    check(volumeinfo, 162);
    check(btnode, 14);
    check(catkey, 38);
    check(xtntkey, 8);
    check(anykey, 38);
    check(filerec, 102);
    check(directoryrec, 70);
    check(threadrec, 46);
    check(filecontrolblock, 94);
    check(btblock0, 512);
    check(cacheentry, 540);
    check(cachehead, 12);
    check(wdentry, 16);

    /* rsys/hfs_plus.h */
    check(HFSUniStr255, 512);
    check(HFSPlusPermissions, 16);
    check(HFSPlusExtentDescriptor, 8);
    check(HFSPlusExtentRecord, 64);
    check(HFSPlusForkData, 80);
    check(HFSPlusVolumeHeader, 512);
    check(BTNodeDescriptor, 14);
    check(BTHeaderRec, 106);
    check(HFSPlusCatalogKey, 518);
    check(HFSPlusCatalogFolder, 88);
    check(HFSPlusCatalogFile, 248);
    check(HFSPlusCatalogThread, 520);
    check(HFSPlusExtentKey, 12);
    check(HFSPlusAttrForkData, 88);
    check(HFSPlusAttrExtents, 72);

    /* rsys/icon.h */
    //check(cotton_suite_layout_t, 26);  // not a GUEST struct, contains native pointers

    /* rsys/itm.h */
    check(itmstr, 14);
    check(altstr, 12);
    check(dlogstr, 22);
    check(item_style_info_t, 20);
    check(item_color_info_t, 4);

    /* rsys/keyboard.h */
    check(completer_pair_t, 2);
    check(completer_t, 2);
    check(dead_key_rec_t, 6);
    check(kchr_str, 262);

    /* rsys/launch.h */
    check(vers_t, 8);

    /* rsys/menu.h */
    check(mext, 5);
    check(muelem, 6);
    //check(menu_elt, 8);  // not a GUEST struct, contains native pointers
    check(menu_list, 8);
    check(menulist, 102);
    check(mbdfheader, 20);
    check(mbdfentry, 28);
    check(mct_res_t, 32);
    check(mbartype, 4);
    //check(startendpairs, 16);   // not a GUEST struct, contains native pointers
    //check(table, 16);    // not a GUEST struct, contains native pointers

    /* rsys/mixed_mode.h */
    check(RoutineRecord, 20);
    check(RoutineDescriptor, 32);

    /* rsys/mman_private.h */
    check(block_header_t, 12);
    check(pblock_t, 14);

    /* rsys/partition.h */
    check(partmapentry_t, 256);
    check(oldmapentry_t, 12);
    check(oldblock1_t, 506);

    /* rsys/pef.h */
    check(PEFContainerHeader_t, 40);
    check(PEFSectionHeader_t, 28);
    check(PEFLoaderInfoHeader_t, 56);
    check(PEFImportedLibrary_t, 24);
    check(PEFLoaderRelocationHeader_t, 12);
    check(PEFExportedSymbol, 10);

    /* rsys/picture.h */
    check(piccache, 96);

    /* rsys/print.h */
    check(TGnlData, 8);
    check(TRslRg, 4);
    check(TRslRec, 4);
    check(TGetRslBlk, 128);
    check(TSetRslBlk, 16);
    check(TTxtPicRec, 10);
    check(TCenterRec, 8);

    /* rsys/process.h */
    check(size_resource_t, 10);

    /* rsys/quick.h */
    check(ccrsr_res, 148);

    /* rsys/resource.h */
    check(reshead, 16);
    check(rsrvrec, 240);
    check(resmap, 28);
    check(typref, 8);
    check(resref, 12);
    check(empty_resource_template_t, 286);
    check(dcomp_info_t, 16);
    //check(res_sorttype_t, 8);   // not a GUEST struct, contains native pointers

    /* rsys/screen-dump.h */
    check(struct header, 8);
    check(struct directory_entry, 12);
    check(struct ifd, 14);

    /* rsys/segment.h */
    check(finderinfo, 268);

    /* rsys/serial.h */
    check(sersetbuf_t, 6);

    /* rsys/soundopts.h */
    //check(ModifierStub, 47); // not a GUEST struct, contains native pointers

    /* rsys/syserr.h */
    check(myalerttab_t, 326);
    check(struct adef, 14);
    check(struct tdef, 10);
    check(struct idef, 140);
    check(struct pdef, 8);
    check(struct bdef, 18);
    check(struct sdef, 6);

    /* rsys/tesave.h */
    check(tesave, 56);
    check(generic_elt_t, 16);
    check(tehidden, 20);

    /* rsys/toolevent.h */
    check(keymap, 644);

    /* rsys/wind.h */
    check(windrestype, 20);

    /* config/arch/powerpc/ppc_stubs.h */

    /*
   * I don't have a machine I can test the aixtosysv4 structs against.
   * The PPC work used a gcc command line flag that is no longer
   * present in the gccs that I have access to.
   */
}

/*

mkvol_internal.h is designed to work without the normal includes, as such
it can't be easily tested from here.  Instead we should do the testing in
mkvol itself, since mkvol could be compiled with different compilation flags
anyway


./mkvol/mkvol_internal.h:80:typedef struct {
./mkvol/mkvol_internal.h:89:typedef struct {
./mkvol/mkvol_internal.h:98:typedef struct {
./mkvol/mkvol_internal.h:103:typedef struct {
./mkvol/mkvol_internal.h:136:typedef struct {
./mkvol/mkvol_internal.h:147:typedef struct {
./mkvol/mkvol_internal.h:154:typedef struct {
./mkvol/mkvol_internal.h:169:typedef struct {
./mkvol/mkvol_internal.h:194:typedef struct {
./mkvol/mkvol_internal.h:210:typedef struct {
./mkvol/mkvol_internal.h:224:typedef struct {
./mkvol/mkvol.c:487:  typedef struct
./mkvol/mkvol.c:505:  typedef struct
./mkvol/mkvol.c:519:  static struct

*/
