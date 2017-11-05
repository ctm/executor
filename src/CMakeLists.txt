project(Executor)
cmake_minimum_required(VERSION 3.9)


find_program(PERL_COMMAND perl)
find_library(SYN68K_LIBRARY syn68k)
find_library(DB_LIBRARY db)
find_program(BISON_COMMAND bison)

include_directories(include/)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(FRONT_END sdl)
set(HOST_ARCH x86_64)

add_subdirectory(config/front-ends/x)
add_subdirectory(config/front-ends/sdl)
add_subdirectory(config/arch/x86_64)

set(host_os "linux")

function(linkheader from to variant)
    if(DEFINED ARGV3)
        set(input ${ARGV3})
    else()
        set(input ${variant}.h)
    endif()

    configure_file(${from}/${variant}/${input} ${to} COPYONLY)
endfunction()

linkheader("config/os" "host-os-config.h" "linux")
linkheader("config/os" "build-os-config.h" "linux")
linkheader("config/arch" "host-arch-config.h" "x86_64")
linkheader("config/arch" "build-arch-config.h" "x86_64")
linkheader("config/front-ends" "front-end-config.h" ${FRONT_END})
linkheader("config/sound" "sound-config.h" "dummy" "dummy-sound.h")

configure_file(config-cmake.h.in config.h)

add_executable(map_to_c map_to_c/map_to_c.cpp build-os-config.h build-arch-config.h)
add_executable(mkseedtables mkseedtables/mkseedtables.c)
add_executable(mkultable mkultable/mkultable.c)
add_executable(mksspairtable mksspairtable/mksspairtable.c)

include_directories(config/os/linux)

set(MAP_FILES
        active             arrow_down_inactive  arrow_right_active    arrow_up_inactive  ractive      zoom
        apple              arrow_left_active    arrow_right_inactive  go_away            thumb_horiz
        arrow_down_active  arrow_left_inactive  arrow_up_active       grow               thumb_vert
    )
set(GENERATED_FILES)
foreach(mapfile ${MAP_FILES})
    add_custom_command(
        OUTPUT ${mapfile}.cmap
        DEPENDS ${mapfile}.map
        COMMAND map_to_c < ${CMAKE_CURRENT_SOURCE_DIR}/${mapfile}.map > ${mapfile}.cmap
        )
    list(APPEND GENERATED_FILES ${mapfile}.cmap)
endforeach()

add_custom_command(OUTPUT seedtables.ctable COMMAND mkseedtables > seedtables.ctable)
add_custom_command(OUTPUT ultable.ctable COMMAND mkultable > ultable.ctable)
add_custom_command(OUTPUT sspairtable.ctable COMMAND mksspairtable > sspairtable.ctable)
list(APPEND GENERATED_FILES seedtables.ctable ultable.ctable sspairtable.ctable)


add_custom_command(OUTPUT rawpatstubs.ctable
    DEPENDS makerawblt.pl pat-blitters.tmpl
    COMMAND ${PERL_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/makerawblt.pl
        < ${CMAKE_CURRENT_SOURCE_DIR}/pat-blitters.tmpl
        > rawpatstubs.ctable)

add_custom_command(OUTPUT rawsrcstubs.ctable
    DEPENDS makerawblt.pl src-blitters.tmpl
    COMMAND ${PERL_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/makerawblt.pl
        < ${CMAKE_CURRENT_SOURCE_DIR}/src-blitters.tmpl
        > rawsrcstubs.ctable)

add_custom_command(OUTPUT parse.tab.cpp
        DEPENDS parse.ypp
        COMMAND ${BISON_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/parse.ypp)
list(APPEND GENERATED_FILES rawpatstubs.ctable rawsrcstubs.ctable parse.tab.cpp)


set(ctl_sources ctlArrows.cpp ctlDisplay.cpp ctlIMIV.cpp ctlInit.cpp ctlMisc.cpp
    ctlMouse.cpp ctlSet.cpp ctlSize.cpp ctlStddef.cpp ctlPopup.cpp)
source_group(Controls FILES ${ctl_sources})

set(dial_sources dialAlert.cpp dialCreate.cpp dialHandle.cpp dialInit.cpp dialManip.cpp
    dialDispatch.cpp dialItem.cpp)
source_group(Dialogs FILES ${dial_sources})

set(file_sources fileAccess.cpp fileCreate.cpp fileDirs.cpp fileDouble.cpp fileInfo.cpp
    fileMisc.cpp fileVolumes.cpp fileHighlevel.cpp dcache.cpp)
source_group(Files FILES ${file_sources})

set(hfs_sources hfsBtree.cpp hfsChanging.cpp hfsCreate.cpp hfsFile.cpp hfsHelper.cpp
    hfsHier.cpp hfsMisc.cpp hfsVolume.cpp hfsWorkingdir.cpp hfsXbar.cpp)
source_group(HFS FILES ${hfs_sources})

set(list_sources listAccess.cpp listAddDel.cpp listCreate.cpp listDisplay.cpp
    listMouse.cpp listOps.cpp listStdLDEF.cpp)
source_group(Lists FILES ${list_sources})

set(menu_sources menu.cpp menuColor.cpp menuV.cpp stdmdef.cpp stdmbdf.cpp)
source_group(Menus FILES ${menu_sources})

set(print_sources PSstrings.cpp prError.cpp prInit.cpp prLowLevel.cpp
    prPrinting.cpp prRecords.cpp PSprint.cpp )
source_group(Printing FILES ${print_sources})

set(qd_sources qBit.cpp qCConv.cpp qCGrafPort.cpp qCRegular.cpp qColor.cpp qColorMgr.cpp
             qColorutil.cpp qCursor.cpp qGrafport.cpp qIMIV.cpp qIMV.cpp qIMVxfer.cpp
             qMisc.cpp qPaletteMgr.cpp qPen.cpp qPicstuff.cpp qPicture.cpp
             qPixMapConv.cpp qPoint.cpp qPoly.cpp qRect.cpp qRegion.cpp qRegular.cpp
             qScale.cpp qStandard.cpp qStdArc.cpp qStdBits.cpp qStdLine.cpp qStdOval.cpp
             qStdPic.cpp qStdPoly.cpp qStdRRect.cpp qStdRect.cpp qStdRgn.cpp
             qStdText.cpp qText.cpp qGWorld.cpp qGDevice.cpp qIMVI.cpp qHooks.cpp
             xdata.cpp xdblt.cpp rawpatblt.cpp rawsrcblt.cpp dirtyrect.cpp srcblt.cpp
             qColorPicker.cpp qPict2.cpp image.cpp image_inits.cpp
             default_ctab_values.cpp dcconvert.cpp rgbutil.cpp dcmaketables.cpp)
source_group(QuickDraw FILES ${qd_sources})

set(res_sources resGet.cpp resGetinfo.cpp resGettype.cpp resIMIV.cpp resInit.cpp
    resMisc.cpp resMod.cpp resOpen.cpp resSetcur.cpp resPartial.cpp)
source_group(Resources FILES ${res_sources})

set(te_sources teAccess.cpp teDisplay.cpp teEdit.cpp teIMIV.cpp teIMV.cpp teInit.cpp
    teInsert.cpp teMisc.cpp teScrap.cpp)
source_group(TextEdit FILES ${te_sources})

set(wind_sources windColor.cpp windDisplay.cpp windDocdef.cpp windInit.cpp windMisc.cpp
    windMouse.cpp windSize.cpp windUpdate.cpp)
source_group(Windows FILES ${wind_sources})

set(ae_sources AE.cpp AE_desc.cpp AE_hdlr.cpp AE_coercion.cpp)
source_group(AppleEvents FILES ${ae_sources})

set(sound_sources sounddriver.cpp sound.cpp soundIMVI.cpp soundfake.cpp snth5.cpp
    SpeechManager.cpp)
source_group(Sound FILES ${sound_sources})

set(num_sources bindec.cpp float4.cpp float5.cpp float7.cpp floatnext.cpp toolmath.cpp
    mathlib.cpp)
source_group(Numerics FILES ${num_sources})

set(misc_sources desk.cpp device.cpp disk.cpp diskinit.cpp dump.cpp trapname.cpp font.cpp
  gestalt.cpp globals.cpp iu.cpp launch.cpp main.cpp mman.cpp mmansubr.cpp notify.cpp hle.cpp
  osevent.cpp osutil.cpp pack.cpp scrap.cpp script.cpp segment.cpp serial.cpp setuid.cpp
  slash.cpp stdfile.cpp romlib_stubs.cpp syserr.cpp toolevent.cpp toolutil.cpp time.cpp
  vbl.cpp syncint.cpp virtualint.cpp refresh.cpp autorefresh.cpp aboutbox.cpp
  keycode.cpp option.cpp parseopt.cpp parsenum.cpp desperate.cpp
  version.cpp shutdown.cpp uniquefile.cpp sigio_multiplex.cpp screen-dump.cpp
  process.cpp alias.cpp string.cpp tempmem.cpp edition.cpp fontIMVI.cpp balloon.cpp
  error.cpp adb.cpp color_wheel_bits.cpp finder.cpp system_error.cpp ibm_keycodes.cpp
  splash.cpp icon.cpp redrawscreen.cpp ini.cpp checkpoint.cpp qt.cpp cleanup.cpp
  paramline.c fauxdbm.cpp custom.cpp commtool.cpp cfm.cpp local_charset.cpp pef_hash.cpp
  interfacelib.cpp mixed_mode.cpp suffix_maps.cpp appearance.cpp lockrange.cpp
  emutrap.cpp emutraptables.cpp emustubs.cpp unix_like.cpp parse.ypp check_structs.cpp
  executor.cpp  crc.cpp)

set(include_sources hintemplate.h
                  include/ADB.h
                  include/AliasMgr.h
                  include/AppleEvents.h
                  include/AppleTalk.h
                  include/BinaryDecimal.h
                  include/CommTool.h
                  include/Components.h
                  include/ControlMgr.h
                  include/CQuickDraw.h
                  include/DeskMgr.h
                  include/DeviceMgr.h
                  include/DialogMgr.h
                  include/Disk.h
                  include/DiskInit.h
                  include/EditionMgr.h
                  include/EventMgr.h
                  include/FileMgr.h
                  include/Finder.h
                  include/FontMgr.h
                  include/Gestalt.h
                  include/HelpMgr.h
                  include/Iconutil.h
                  include/IntlUtil.h
                  include/ListMgr.h
                  include/MacTypes.h
                  include/MemoryMgr.h
                  include/MenuMgr.h
                  include/NotifyMgr.h
                  include/OSEvent.h
                  include/OSUtil.h
                  include/Package.h
                  include/paramline.h
                  include/PPC.h
                  include/PrintMgr.h
                  include/ProcessMgr.h
                  include/QuickDraw.h
                  include/QuickTime.h
                  include/ResourceMgr.h
                  include/rsys/aboutbox.h
                  include/rsys/aboutpanel.h
                  include/rsys/adb.h
                  include/rsys/alias.h
                  include/rsys/appearance.h
                  include/rsys/apple_events.h
                  include/rsys/arch.h
                  include/rsys/arrowkeys.h
                  include/rsys/autorefresh.h
                  include/rsys/blockdev.h
                  include/rsys/blockinterrupts.h
                  include/rsys/byteswap.h
                  include/rsys/cfm.h
                  include/rsys/check_structs.h
                  include/rsys/checkpoint.h
                  include/rsys/cleanup.h
                  include/rsys/color_wheel_bits.h
                  include/rsys/common.h
                  include/rsys/commonevt.h
                  include/rsys/cquick.h
                  include/rsys/crc.h
                  include/rsys/cruft.h
                  include/rsys/ctl.h
                  include/rsys/ctopflags.h
                  include/rsys/custom.h
                  include/rsys/dcache.h
                  include/rsys/depthconv.h
                  include/rsys/desk.h
                  include/rsys/desperate.h
                  include/rsys/device.h
                  include/rsys/dial.h
                  include/rsys/dirtyrect.h
                  include/rsys/drive_flags.h
                  include/rsys/dump.h
                  include/rsys/emustubs.h
                  include/rsys/error.h
                  include/rsys/everything.h
                  include/rsys/evil.h
                  include/rsys/executor.h
                  include/rsys/fauxdbm.h
                  include/rsys/file.h
                  include/rsys/filedouble.h
                  include/rsys/flags.h
                  include/rsys/float.h
                  include/rsys/float_fcw.h
                  include/rsys/floatconv.h
                  include/rsys/font.h
                  include/rsys/gestalt.h
                  include/rsys/glue.h
                  include/rsys/gworld.h
                  include/rsys/hfs.h
                  include/rsys/hfs_plus.h
                  include/rsys/hook.h
                  include/rsys/host.h
                  include/rsys/icon.h
                  include/rsys/image.h
                  include/rsys/ini.h
                  include/rsys/int386.h
                  include/rsys/interfacelib.h
                  include/rsys/itm.h
                  include/rsys/iv.h
                  include/rsys/jumpvectors.h
                  include/rsys/keyboard.h
                  include/rsys/keyboards.h
                  include/rsys/keycode.h
                  include/rsys/launch.h
                  include/rsys/libcproto.h
                  include/rsys/licensetext.h
                  include/rsys/list.h
                  include/rsys/local_charset.h
                  include/rsys/lockunlock.h
                  include/rsys/lowglobals.h
                  include/rsys/m68kint.h
                  include/rsys/macros.h
                  include/rsys/mactype.h
                  include/rsys/mathlib.h
                  include/rsys/memory_layout.h
                  include/rsys/memsize.h
                  include/rsys/menu.h
                  include/rsys/misc.h
                  include/rsys/mixed_mode.h
                  include/rsys/mman.h
                  include/rsys/mman_private.h
                  include/rsys/mmanstubs.h
                  include/rsys/new_mmanstubs.h
                  include/rsys/newvga.h
                  include/rsys/next.h
                  include/rsys/nextprint.h
                  include/rsys/noreturn.h
                  include/rsys/notmac.h
                  include/rsys/Olibcproto.h
                  include/rsys/option.h
                  include/rsys/options.h
                  include/rsys/os.h
                  include/rsys/osevent.h
                  include/rsys/osutil.h
                  include/rsys/parse.h
                  include/rsys/parsenum.h
                  include/rsys/parseopt.h
                  include/rsys/partition.h
                  include/rsys/pef.h
                  include/rsys/picture.h
                  include/rsys/pragmal.h
                  include/rsys/pragmar.h
                  include/rsys/pragmas.h
                  include/rsys/prefpanel.h
                  include/rsys/prefs.h
                  include/rsys/print.h
                  include/rsys/process.h
                  include/rsys/PSstrings.h
                  include/rsys/pstuff.h
                  include/rsys/ptocflags.h
                  include/rsys/qcolor.h
                  include/rsys/quick.h
                  include/rsys/rawblt.h
                  include/rsys/redrawscreen.h
                  include/rsys/refresh.h
                  include/rsys/region.h
                  include/rsys/release.h
                  include/rsys/resource.h
                  include/rsys/rgbutil.h
                  include/rsys/safe_alloca.h
                  include/rsys/scrap.h
                  include/rsys/screen-dump.h
                  include/rsys/segment.h
                  include/rsys/serial.h
                  include/rsys/setuid.h
                  include/rsys/sigio_multiplex.h
                  include/rsys/slash.h
                  include/rsys/smash.h
                  include/rsys/sounddriver.h
                  include/rsys/soundfake.h
                  include/rsys/soundopts.h
                  include/rsys/splash.h
                  include/rsys/srcblt.h
                  include/rsys/stdbits.h
                  include/rsys/stdfile.h
                  include/rsys/stdio_smashage.h
                  include/rsys/string.h
                  include/rsys/stubify.h
                  include/rsys/suffix_maps.h
                  include/rsys/syncint.h
                  include/rsys/syserr.h
                  include/rsys/system_error.h
                  include/rsys/tempalloc.h
                  include/rsys/tesave.h
                  include/rsys/text.h
                  include/rsys/time.h
                  include/rsys/toolevent.h
                  include/rsys/toolutil.h
                  include/rsys/trapdecl.h
                  include/rsys/trapdefines.h
                  include/rsys/trapglue.h
                  include/rsys/trapname.h
                  include/rsys/types.h
                  include/rsys/uniquefile.h
                  include/rsys/vbl.h
                  include/rsys/vdriver.h
                  include/rsys/version.h
                  include/rsys/vgavdriver.h
                  include/rsys/wind.h
                  include/rsys/wrappers.h
                  include/rsys/x.h
                  include/rsys/xdata.h
                  include/rsys/xdblt.h
                  include/rsys/Xdefs.h
                  include/SANE.h
                  include/ScrapMgr.h
                  include/ScriptMgr.h
                  include/SegmentLdr.h
                  include/Serial.h
                  include/ShutDown.h
                  include/SoundDvr.h
                  include/SoundMgr.h
                  include/StartMgr.h
                  include/StdFilePkg.h
                  include/SysErr.h
                  include/TextEdit.h
                  include/ThinkC.h
                  include/TimeMgr.h
                  include/ToolboxEvent.h
                  include/ToolboxUtil.h
                  include/VDriver.h
                  include/VRetraceMgr.h
                  include/WindowMgr.h
                  mkvol/mkvol.h
                  mkvol/mkvol_internal.h)

add_executable(executor ${ctl_sources} ${dial_sources} ${file_sources}
        ${hfs_sources} ${list_sources} ${menu_sources}
        ${print_sources} ${qd_sources} ${res_sources}
        ${te_sources} ${wind_sources} ${ae_sources}
        ${sound_sources} ${num_sources} ${misc_sources}
        ${arch_sources} ${front_end_sources} ${host_sources}
        ${os_sources}  ${include_sources} ${other_sources}
        ${GENERATED_FILES}
    )

target_link_libraries(executor front-end-${FRONT_END} arch-${HOST_ARCH})
target_link_libraries(executor ${SYN68K_LIBRARY} ${DB_LIBRARY})
