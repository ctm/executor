#include <rsys/common.h>
#include <rsys/check_structs.h>

#include <VRetraceMgr.h>

#if defined(USE_VDRIVER_H)
#  include <VDriver.h>
#endif

#include <HelpMgr.h>
#include <ProcessMgr.h>
#include <SoundMgr.h>

#include <rsys/file.h>

#define check(type, expected_size)                                       \
  do {                                                                   \
    if (sizeof(type) != expected_size)                                   \
      fprintf(stderr, "Expected sizeof(" #type ") to be %zu, got %zu\n", \
              (size_t) expected_size, sizeof(type));                     \
  } while (false)

void check_structs(void)
{
  check(VBLTask, 14); /* VRetraceMgr.h */

#if defined(USE_VDRIVER_H)
  check (VDParamBlock, TODO); /* VDriver.h */
  check (VDEntryRecord, TODO);
  check (VDGammaRecord, TODO);
  check (VDPgInfo, TODO);
  check (VDFlagRec, TODO);
  check (VDDefModeRec, TODO);
#endif /* defined(USE_VDRIVER_H) */

  check (HMStringResType, 4); /* HelpMgr.h */
  check (HMMessageRecord, 258);

  check (ProcessSerialNumber, 8); /* ProcessMgr.h */
  check (LaunchParamBlockRec, 44);
  check (ProcessInfoRec, 60);

  check (SndCommand, 8); /* SoundMgr.h */
  check (SndChannel, 1060);
  check (SoundHeader, 22);
  check (ExtSoundHeader, 62);
  check (SndDoubleBuffer, 16);
  check (SndDoubleBufferHeader, 24);
  check (SCStatus, 24);

  check(fcbrec, 94); /* rsys/file.h */
}

#if false

TODO: These .c files have structs that refer to Mac memory and should
      be size checked

./font.c:267:typedef struct {
./segment.c:66:typedef struct {
./resOpen.c:31:    struct {            /* empty resource template */
./ctlMouse.c:72:typedef struct {
./icon.c:352:typedef struct
./ctlInit.c:110:typedef struct {
./menu.c:224:typedef struct mct_res
./menu.c:738:typedef struct {
./menu.c:806:typedef struct {
./mkvol/mkvol.c:487:  typedef struct
./mkvol/mkvol.c:505:  typedef struct
./mkvol/mkvol.c:519:  static struct
./stdmdef.c:45:typedef struct
./ctlArrows.c:698:struct lsastr
./qCGrafPort.c:420:struct pixpat_res
./fileDouble.c:50:PRIVATE struct defaulthead {
./fileDouble.c:115:PRIVATE struct defaultentries {
./screen-dump.c:60:struct header
./screen-dump.c:67:struct directory_entry
./screen-dump.c:75:struct ifd
./windInit.c:550:typedef struct {
./process.c:28:typedef struct size_resource
./syserr.c:36:PRIVATE struct {
./syserr.c:165:struct adef {
./syserr.c:175:struct tdef {
./syserr.c:182:struct idef {
./syserr.c:189:struct pdef {
./syserr.c:196:struct bdef {
./syserr.c:200:    struct but {
./syserr.c:207:struct sdef {
./launch.c:375:typedef struct {
./qCursor.c:221:typedef struct ccrsr_res
./dialAlert.c:29:static struct
./toolevent.c:160:typedef struct {
./alias.c:384:typedef struct
./alias.c:404:typedef struct /* 0x0000 */
./alias.c:411:typedef struct /* 0x0001 */
./alias.c:418:typedef struct /* 0x0002 */
./alias.c:425:typedef struct /* 0x0009 */
./alias.c:437:typedef struct
./resMod.c:332:typedef struct {
./fileVolumes.c:69:typedef struct {
./fileVolumes.c:76:typedef struct {
./emustubs.c:229:typedef struct
./emustubs.c:1783:typedef struct comm_toolbox_dispatch_args
./emustubs.c:2604:typedef struct
./AE_desc.c:29:typedef struct
./AE_desc.c:36:typedef struct
./AE_desc.c:44:typedef struct list_header
./AE_desc.c:82:typedef struct ae_header
./serial.c:128:    struct {

These .h files have structs that should be size checked

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
./config/arch/powerpc/ppc_stubs.h:11:typedef struct
./config/arch/powerpc/ppc_stubs.h:18:typedef struct
./config/arch/powerpc/ppc_stubs.h:25:typedef struct
./config/arch/powerpc/ppc_stubs.h:34:typedef struct
./config/arch/powerpc/ppc_stubs.h:41:typedef struct
./config/arch/powerpc/ppc_stubs.h:48:typedef struct
./config/arch/powerpc/ppc_stubs.h:55:typedef struct
./config/arch/powerpc/ppc_stubs.h:62:typedef struct
./config/arch/powerpc/ppc_stubs.h:69:typedef struct
./config/arch/powerpc/ppc_stubs.h:76:typedef struct
./config/arch/powerpc/ppc_stubs.h:84:typedef struct

./include/VRetraceMgr.h:14:typedef struct {
./include/VDriver.h:13:typedef struct
./include/VDriver.h:24:typedef struct
./include/VDriver.h:34:typedef struct
./include/VDriver.h:42:typedef struct
./include/VDriver.h:53:typedef struct
./include/VDriver.h:61:typedef struct
./include/HelpMgr.h:15:typedef struct HMStringResType
./include/HelpMgr.h:21:typedef struct HMMessageRecord
./include/ProcessMgr.h:16:typedef struct ProcessSerialNumber
./include/ProcessMgr.h:32:typedef struct
./include/ProcessMgr.h:44:typedef struct
./include/ProcessMgr.h:70:typedef struct ProcessInfoRec
./include/SoundMgr.h:13:typedef struct {
./include/SoundMgr.h:28:typedef struct _SndChannel {
./include/SoundMgr.h:77:typedef struct {
./include/SoundMgr.h:87:typedef struct _SoundHeader {
./include/SoundMgr.h:98:typedef struct _ExtSoundHeader {
./include/SoundMgr.h:145:typedef struct
./include/SoundMgr.h:158:typedef struct
./include/SoundMgr.h:169:typedef struct _SCSTATUS {
./include/FileMgr.h:73:typedef struct {
./include/FileMgr.h:81:typedef struct {
./include/FileMgr.h:88:typedef struct {
./include/FileMgr.h:95:typedef struct {
./include/FileMgr.h:120:typedef struct {
./include/FileMgr.h:133:typedef struct {
./include/FileMgr.h:153:typedef struct {
./include/FileMgr.h:171:typedef struct {
./include/FileMgr.h:186:typedef struct {
./include/FileMgr.h:199:typedef struct {
./include/FileMgr.h:219:typedef struct {
./include/FileMgr.h:265:typedef struct {
./include/FileMgr.h:283:typedef struct {
./include/FileMgr.h:302:typedef struct {
./include/FileMgr.h:313:typedef struct {
./include/FileMgr.h:324:typedef struct {
./include/FileMgr.h:342:typedef struct {
./include/FileMgr.h:393:typedef struct {
./include/FileMgr.h:406:struct FSSpec
./include/MacTypes.h:58:typedef struct {
./include/MacTypes.h:71:typedef struct Point
./include/MacTypes.h:81:typedef struct Rect
./include/WindowMgr.h:81:struct __wr {
./include/WindowMgr.h:101:typedef struct {
./include/WindowMgr.h:117:typedef struct AuxWinRec {
./include/PPC.h:13:typedef struct EntityName
./include/PPC.h:18:typedef struct LocationNameRec
./include/PPC.h:29:typedef struct PPCPortRec
./include/PPC.h:39:      struct
./include/FontMgr.h:54:typedef struct {
./include/FontMgr.h:62:typedef struct {
./include/FontMgr.h:83:typedef struct {
./include/FontMgr.h:107:typedef struct {
./include/FontMgr.h:117:typedef struct {
./include/FontMgr.h:138:typedef struct {
./include/DeviceMgr.h:21:typedef struct {
./include/DeviceMgr.h:30:typedef struct {
./include/DeviceMgr.h:49:typedef struct {
./include/DeviceMgr.h:92:typedef struct {
./include/StartMgr.h:13:    struct {
./include/StartMgr.h:19:    struct {
./include/StartMgr.h:26:typedef struct {
./include/StartMgr.h:31:typedef struct {
./include/Serial.h:70:typedef struct {
./include/Serial.h:81:typedef struct {
./include/ScrapMgr.h:16:typedef struct {
./include/CQuickDraw.h:7:typedef struct
./include/CQuickDraw.h:17:typedef struct
./include/CQuickDraw.h:37:typedef struct SProcRec
./include/CQuickDraw.h:45:typedef struct CProcRec
./include/CQuickDraw.h:53:typedef struct GDevice
./include/CQuickDraw.h:82:typedef struct ColorInfo
./include/CQuickDraw.h:91:typedef struct Palette
./include/CQuickDraw.h:147:typedef struct ReqListRec
./include/CQuickDraw.h:155:typedef struct OpenCPicParams
./include/CQuickDraw.h:165:typedef struct CommonSpec
./include/CQuickDraw.h:175:typedef struct FontSpec
./include/CQuickDraw.h:188:typedef struct PictInfo
./include/OSUtil.h:27:typedef struct {
./include/OSUtil.h:53:typedef struct {
./include/OSUtil.h:63:typedef struct {
./include/Components.h:11:typedef struct ComponentRecord
./include/Components.h:19:typedef struct ComponentInstanceRecord
./include/QuickTime.h:15:typedef struct MovieRecord
./include/AppleEvents.h:19:typedef struct AEDesc
./include/AppleEvents.h:34:typedef struct AEKeyDesc
./include/AppleEvents.h:115:typedef struct AE_hdlr
./include/AppleEvents.h:121:typedef struct AE_hdlr_selector
./include/AppleEvents.h:127:typedef struct AE_hdlr_table_elt
./include/AppleEvents.h:148:typedef struct AE_hdlr_table
./include/AppleEvents.h:164:typedef struct AE_zone_tables
./include/AppleEvents.h:185:typedef struct AE_info
./include/MemoryMgr.h:27:typedef struct Zone
./include/Disk.h:36:typedef struct {
./include/ADB.h:11:typedef struct
./include/ADB.h:19:typedef struct
./include/ToolboxUtil.h:21:typedef struct {
./include/CommTool.h:11:typedef struct
./include/CommTool.h:35:typedef struct
./include/OSEvent.h:18:typedef struct {
./include/OSEvent.h:58:typedef struct size_info
./include/OSEvent.h:98:typedef struct TargetID
./include/OSEvent.h:106:typedef struct HighLevelEventMsg
./include/DialogMgr.h:33:typedef struct
./include/DialogMgr.h:63:typedef struct
./include/DialogMgr.h:80:typedef struct {
./include/DialogMgr.h:95:typedef struct {
./include/NotifyMgr.h:11:typedef struct {
./include/SegmentLdr.h:21:typedef struct {
./include/ScriptMgr.h:99:typedef struct DateCacheRec
./include/ScriptMgr.h:104:typedef struct LongDateRec
./include/ScriptMgr.h:133:typedef struct
./include/ScriptMgr.h:148:typedef struct
./include/ScriptMgr.h:155:typedef struct
./include/ScriptMgr.h:175:typedef struct
./include/SoundDvr.h:17:typedef struct {
./include/SoundDvr.h:24:typedef struct {
./include/SoundDvr.h:31:typedef struct {
./include/SoundDvr.h:46:typedef struct {
./include/SoundDvr.h:64:typedef struct {
./include/Iconutil.h:98:typedef struct CIcon
./include/ControlMgr.h:87:struct __cr {
./include/ControlMgr.h:103:typedef struct {
./include/ControlMgr.h:117:typedef struct AuxCtlRec {
./include/IntlUtil.h:57:typedef struct {
./include/IntlUtil.h:90:typedef struct {
./include/SANE.h:14:  struct {
./include/SANE.h:25:  struct {
./include/SANE.h:37:typedef struct {
./include/SANE.h:41:    struct {                /* Here for added efficiency when BIGENDIAN. */
./include/SANE.h:51:    struct {
./include/SANE.h:68:typedef struct {
./include/SANE.h:77:typedef struct {
./include/SANE.h:93:typedef struct {
./include/StdFilePkg.h:31:typedef struct {
./include/StdFilePkg.h:47:typedef struct
./include/TimeMgr.h:12:typedef struct {
./include/TextEdit.h:53:typedef struct {
./include/TextEdit.h:92:typedef struct {
./include/TextEdit.h:97:typedef struct {
./include/TextEdit.h:113:typedef struct {
./include/TextEdit.h:123:typedef struct {
./include/TextEdit.h:131:typedef struct {
./include/TextEdit.h:144:typedef struct {
./include/TextEdit.h:153:typedef struct {
./include/TextEdit.h:162:typedef struct {
./include/Finder.h:11:typedef struct
./include/EditionMgr.h:16:typedef struct type ## Record type ## Record;				\
./include/EditionMgr.h:22:typedef struct type type;						\
./include/EditionMgr.h:38:struct SectionRecord
./include/EditionMgr.h:56:struct EditionContainerSpec
./include/EditionMgr.h:68:struct EditionInfoRecord
./include/EditionMgr.h:80:struct NewPublisherReply
./include/EditionMgr.h:94:struct NewSubscriberReply
./include/EditionMgr.h:104:struct SectionOptionsReply
./include/EditionMgr.h:123:struct EditionOpenerParamBlock
./include/EditionMgr.h:145:struct FormatIOParamBlock
./include/MenuMgr.h:23:typedef struct {
./include/MenuMgr.h:35:typedef struct MCEntry
./include/OLDSANE.h:11:typedef struct {
./include/OLDSANE.h:16:typedef struct {
./include/OLDSANE.h:22:typedef struct {
./include/OLDSANE.h:29:typedef struct {
./include/OLDSANE.h:44:typedef struct {
./include/PrintMgr.h:40:typedef struct {
./include/PrintMgr.h:49:typedef struct {
./include/PrintMgr.h:58:typedef struct {
./include/PrintMgr.h:67:typedef struct {
./include/PrintMgr.h:81:typedef struct {
./include/PrintMgr.h:94:typedef struct {
./include/PrintMgr.h:110:typedef struct {
./include/PrintMgr.h:124:typedef struct {
./include/ListMgr.h:26:typedef struct {
./include/QuickDraw.h:71:typedef struct {
./include/QuickDraw.h:81:typedef struct {
./include/QuickDraw.h:90:typedef struct {
./include/QuickDraw.h:107:typedef struct {
./include/QuickDraw.h:117:typedef struct {
./include/QuickDraw.h:124:typedef struct {
./include/QuickDraw.h:146:typedef struct {
./include/QuickDraw.h:178:typedef struct {
./include/QuickDraw.h:187:typedef struct {
./include/QuickDraw.h:205:typedef struct {
./include/QuickDraw.h:211:typedef struct {
./include/QuickDraw.h:217:typedef struct {
./include/QuickDraw.h:223:typedef struct { 
./include/QuickDraw.h:229:typedef struct ColorSpec
./include/QuickDraw.h:237:typedef struct {
./include/QuickDraw.h:248:typedef struct {
./include/QuickDraw.h:271:typedef struct {
./include/QuickDraw.h:301:typedef struct {
./include/QuickDraw.h:314:typedef struct {
./include/QuickDraw.h:351:typedef struct {
./include/QuickDraw.h:367:typedef struct {
./include/EventMgr.h:66:typedef struct {

These .h files may have structs that need to be size checked

./include/rsys/common.h:50:typedef struct
./include/rsys/stdbits.h:4:struct cleanup_info
./include/rsys/file.h:27:typedef struct {
./include/rsys/file.h:34:typedef struct {
./include/rsys/file.h:70:typedef struct {
./include/rsys/file.h:92:typedef struct {
./include/rsys/file.h:99:typedef struct {	/* add new elements to the beginning of this struct */
./include/rsys/file.h:197:typedef struct hashlink_str {
./include/rsys/file.h:204:typedef struct {
./include/rsys/file.h:211:	struct {
./include/rsys/file.h:251:typedef struct
./include/rsys/float.h:57:  struct {
./include/rsys/float.h:69:typedef struct {
./include/rsys/float.h:77:typedef struct {
./include/rsys/float.h:98:  struct {
./include/rsys/tempalloc.h:44:typedef struct
./include/rsys/hfs_plus.h:27:typedef struct HFSUniStr255
./include/rsys/hfs_plus.h:40:typedef struct HFSPlusPermissions
./include/rsys/hfs_plus.h:49:typedef struct HFSPlusExtentDescriptor
./include/rsys/hfs_plus.h:58:typedef struct HFSPlusForkData
./include/rsys/hfs_plus.h:67:typedef struct HFSPlusVolumeHeader
./include/rsys/hfs_plus.h:98:typedef struct BTNodeDescriptor
./include/rsys/hfs_plus.h:109:typedef struct BTHeaderRec
./include/rsys/hfs_plus.h:129:typedef struct HFSPlusCatalogKey
./include/rsys/hfs_plus.h:137:typedef struct HFSPlusCatalogFolder
./include/rsys/hfs_plus.h:156:typedef struct HFSPlusCatalogFile
./include/rsys/hfs_plus.h:177:typedef struct HFSPlusCatalogThread
./include/rsys/hfs_plus.h:186:typedef struct HFSPlusExtentKey
./include/rsys/hfs_plus.h:196:typedef struct HFSPlusAttrForkData
./include/rsys/hfs_plus.h:204:typedef struct HFSPlusAttrExtents
./include/rsys/commonevt.h:11:typedef struct {
./include/rsys/mactype.h:21:typedef struct { int32 l PACKED; } HIDDEN_LONGINT;
./include/rsys/mactype.h:22:typedef struct { uint32 u PACKED; } HIDDEN_ULONGINT;
./include/rsys/ini.h:15:typedef struct pair_link_str
./include/rsys/options.h:11:typedef struct {
./include/rsys/int386.h:8:  struct
./include/rsys/int386.h:12:  struct
./include/rsys/int386.h:22:  struct
./include/rsys/itm.h:13:typedef struct {
./include/rsys/itm.h:55:typedef struct {
./include/rsys/itm.h:65:typedef struct
./include/rsys/itm.h:81:typedef struct item_style_info
./include/rsys/itm.h:92:typedef struct item_color_info
./include/rsys/print.h:83:typedef struct
./include/rsys/print.h:91:typedef struct
./include/rsys/print.h:98:typedef struct
./include/rsys/print.h:105:typedef struct
./include/rsys/print.h:118:typedef struct
./include/rsys/print.h:129:typedef struct
./include/rsys/print.h:168:typedef struct
./include/rsys/gworld.h:4:typedef struct gw_info
./include/rsys/mixed_mode.h:33:typedef struct RoutineRecord
./include/rsys/mixed_mode.h:45:typedef struct RoutineDescriptor
./include/rsys/mman_private.h:15:typedef struct block_header
./include/rsys/mman_private.h:186:typedef struct
./include/rsys/vgavdriver.h:6:typedef struct
./include/rsys/keyboard.h:15:typedef struct
./include/rsys/keyboard.h:21:typedef struct
./include/rsys/keyboard.h:32:typedef struct
./include/rsys/keyboard.h:56:typedef struct
./include/rsys/menu.h:32:typedef struct {
./include/rsys/menu.h:75:typedef struct {
./include/rsys/menu.h:80:typedef struct menu_elt
./include/rsys/menu.h:115:typedef struct menu_list
./include/rsys/menu.h:128:typedef struct {
./include/rsys/menu.h:160:typedef struct {	/* from MPW Private.a */
./include/rsys/menu.h:174:typedef struct {
./include/rsys/menu.h:242:typedef struct icon_info
./include/rsys/sounddriver.h:6:struct _sound_driver_t
./include/rsys/font.h:8:typedef struct {
./include/rsys/fauxdbm.h:6:typedef struct
./include/rsys/blockdev.h:4:typedef struct _blockdev_t
./include/rsys/resource.h:18:typedef struct {
./include/rsys/resource.h:25:typedef struct {
./include/rsys/resource.h:30:typedef struct {
./include/rsys/resource.h:55:typedef struct {
./include/rsys/resource.h:61:typedef struct {
./include/rsys/resource.h:193:typedef struct
./include/rsys/cquick.h:14:typedef struct GrafVars
./include/rsys/cquick.h:564:typedef struct draw_state
./include/rsys/cquick.h:585:extern struct qd_color_elt
./include/rsys/cquick.h:591:typedef struct write_back_data
./include/rsys/depthconv.h:73:typedef struct
./include/rsys/depthconv.h:81:typedef struct
./include/rsys/newvga.h:13:typedef struct
./include/rsys/newvga.h:42:typedef struct
./include/rsys/newvga.h:52:typedef struct
./include/rsys/newvga.h:82:typedef struct
./include/rsys/newvga.h:87:typedef struct
./include/rsys/licensetext.h:4:typedef struct
./include/rsys/partition.h:15:typedef struct {
./include/rsys/partition.h:43:typedef struct {
./include/rsys/partition.h:51:typedef struct {
./include/rsys/mmanstubs.h:55:  struct {
./include/rsys/pef.h:13:typedef struct PEFContainerHeader
./include/rsys/pef.h:59:typedef struct PEFSectionHeader
./include/rsys/pef.h:93:typedef struct PEFLoaderInfoHeader
./include/rsys/pef.h:154:typedef struct PEFImportedLibrary
./include/rsys/pef.h:174:typedef struct PEFLoaderRelocationHeader
./include/rsys/pef.h:214:typedef struct PEFExportedSymbol
./include/rsys/pef.h:248:typedef struct pef_hash
./include/rsys/hfs.h:42:typedef struct {
./include/rsys/hfs.h:47:typedef struct {
./include/rsys/hfs.h:87:typedef struct {
./include/rsys/hfs.h:98:typedef struct {
./include/rsys/hfs.h:105:typedef struct {
./include/rsys/hfs.h:120:typedef struct {
./include/rsys/hfs.h:145:typedef struct {
./include/rsys/hfs.h:161:typedef struct {
./include/rsys/hfs.h:179:typedef struct {
./include/rsys/hfs.h:212:typedef struct {
./include/rsys/hfs.h:254:typedef struct _cacheentry {
./include/rsys/hfs.h:271:typedef struct {
./include/rsys/hfs.h:282:typedef struct {
./include/rsys/hfs.h:295:typedef struct {
./include/rsys/hfs.h:306:typedef struct {    /* from MPW equates */
./include/rsys/custom.h:44:typedef struct
./include/rsys/custom.h:51:typedef struct
./include/rsys/custom.h:59:typedef struct
./include/rsys/custom.h:66:typedef struct
./include/rsys/custom.h:73:typedef struct
./include/rsys/xdata.h:9:typedef struct
./include/rsys/xdata.h:20:typedef struct _xdata_t
./include/rsys/rawblt.h:4:typedef struct
./include/rsys/keycode.h:13:typedef struct
./include/rsys/splash.h:4:struct splash_screen_rect
./include/rsys/splash.h:12:struct splash_screen_header
./include/rsys/splash.h:36:  struct splash_screen_rect button_rects[4]
./include/rsys/splash.h:43:struct splash_screen_color
./include/rsys/option.h:20:typedef struct option
./include/rsys/option.h:55:typedef struct opt_val
./include/rsys/option.h:68:typedef struct opt_database
./include/rsys/ctl.h:93:struct popup_data
./include/rsys/cfm.h:18:typedef struct
./include/rsys/cfm.h:37:typedef struct
./include/rsys/cfm.h:111:typedef struct MemFragment
./include/rsys/cfm.h:119:typedef struct DiskFragment
./include/rsys/cfm.h:127:typedef struct SegmentedFragment
./include/rsys/cfm.h:135:typedef struct FragmentLocator
./include/rsys/cfm.h:148:typedef struct InitBlock
./include/rsys/cfm.h:180:typedef struct
./include/rsys/cfm.h:189:typedef struct CFragConnection
./include/rsys/cfm.h:207:typedef struct
./include/rsys/cfm.h:224:typedef struct
./include/rsys/cfm.h:236:typedef struct
./include/rsys/soundopts.h:54:typedef struct _ModifierStub {
./include/rsys/soundopts.h:88:struct hunger_info
./include/rsys/iv.h:13:typedef struct color
./include/rsys/iv.h:18:typedef struct image_header
./include/rsys/checkpoint.h:13:typedef struct
./include/rsys/nextprint.h:24:typedef struct {
./include/rsys/nextprint.h:31:typedef struct {
./include/rsys/nextprint.h:36:typedef struct {
./include/rsys/nextprint.h:42:typedef struct {
./include/rsys/nextprint.h:60:typedef struct {
./include/rsys/nextprint.h:67:typedef struct {
./include/rsys/nextprint.h:73:typedef struct {
./include/rsys/nextprint.h:94:typedef struct {
./include/rsys/vdriver.h:5:struct ColorSpec;
./include/rsys/vdriver.h:21:struct									  \
./include/rsys/vdriver.h:25:  struct { short width, height; } size[num_entries];			  \
./include/rsys/vdriver.h:34:typedef struct
./include/rsys/image.h:4:typedef struct pixel_image
./include/rsys/image.h:21:typedef struct image_bits_desc
./include/rsys/image.h:28:typedef struct pixel_image_desc
./include/rsys/picture.h:56:typedef struct {
./include/rsys/trapglue.h:4:typedef struct {
./include/rsys/trapglue.h:10:typedef struct {
./include/rsys/trapglue.h:15:typedef struct {
./include/rsys/tesave.h:13:typedef struct
./include/rsys/tesave.h:62:typedef struct generic_elt
./include/rsys/tesave.h:128:typedef struct {	/* from MPW: ToolEqu.a */
./include/rsys/rgbutil.h:8:struct rgb_spec;
./include/rsys/rgbutil.h:13:typedef struct
./include/rsys/rgbutil.h:21:typedef void (*rgb_extract_func_t) (const struct rgb_spec *rgb_spec,
./include/rsys/rgbutil.h:26:typedef struct rgb_spec
./include/rsys/rgbutil.h:49:  uint32 (*rgbcolor_to_pixel) (const struct rgb_spec *rgb_spec,
./include/rsys/filedouble.h:30:typedef struct {
./include/rsys/filedouble.h:36:typedef struct {
./include/rsys/filedouble.h:43:typedef struct {
./include/rsys/filedouble.h:50:typedef struct {

#endif
