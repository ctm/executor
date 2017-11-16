.DELETE_ON_ERROR:

bin_PROGRAMS = executor

noinst_PROGRAMS = map_to_c mkseedtables mkultable mksspairtable

ctl_bitmap_c_files = arrow_up_active.cmap arrow_up_inactive.cmap       \
                     arrow_down_active.cmap arrow_down_inactive.cmap   \
                     arrow_right_active.cmap arrow_right_inactive.cmap \
                     arrow_left_active.cmap arrow_left_inactive.cmap   \
                     thumb_horiz.cmap thumb_vert.cmap

wind_bitmap_c_files = zoom.cmap go_away.cmap active.cmap ractive.cmap grow.cmap

CLEANFILES = $(ctl_bitmap_c_files) $(wind_bitmap_c_files)  apple.cmap \
             seedtables.ctable ultable.ctable sspairtable.ctable rawpatstubs.cpp rawsrcstubs.cpp

$(ctl_bitmap_c_files) apple.cpp $(wind_bitmap_c_files):	map_to_c

map_to_c_SOURCES = map_to_c/map_to_c.cpp
mkseedtables_SOURCES = mkseedtables/mkseedtables.c
mkultable_SOURCES = mkultable/mkultable.c
mksspairtable_SOURCES = mksspairtable/mksspairtable.c

.map.cmap:	map_to_c
	./map_to_c < $< > $@

seedtables.ctable:	mkseedtables
	./mkseedtables > $@

ultable.ctable:	mkultable
	./mkultable > $@

sspairtable.ctable:	mksspairtable
	./mksspairtable > $@

rawpatstubs.ctable:	$(srcdir)/makerawblt.pl $(srcdir)/pat-blitters.tmpl
	$(PERL) $(srcdir)/makerawblt.pl < $(srcdir)/pat-blitters.tmpl > rawpatstubs.ctable

rawsrcstubs.ctable: $(srcdir)/makerawblt.pl $(srcdir)/src-blitters.tmpl
	$(PERL) $(srcdir)/makerawblt.pl < $(srcdir)/src-blitters.tmpl > rawsrcstubs.ctable

parse.o: parse.cpp

ctlArrows.$(OBJEXT):	$(ctl_bitmap_c_files)
stdmbdf.$(OBJEXT):	apple.cmap
qIMIV.$(OBJEXT):	seedtables.ctable
qStdText.$(OBJEXT):	ultable.ctable
qIMVI.$(OBJEXT):	sspairtable.ctable
rawpatblt.$(OBJEXT):	rawpatstubs.ctable
rawsrcblt.$(OBJEXT):	rawsrcstubs.ctable
windDocdef.$(OBJEXT):	$(wind_bitmap_c_files)

# These helper variables break the source into logical groups, but there's
# no actual significance to them.

ctl_sources = ctlArrows.cpp ctlDisplay.cpp ctlIMIV.cpp ctlInit.cpp ctlMisc.cpp \
              ctlMouse.cpp ctlSet.cpp ctlSize.cpp ctlStddef.cpp ctlPopup.cpp

dial_sources = dialAlert.cpp dialCreate.cpp dialHandle.cpp dialInit.cpp dialManip.cpp \
               dialDispatch.cpp dialItem.cpp

file_sources = fileAccess.cpp fileCreate.cpp fileDirs.cpp fileDouble.cpp fileInfo.cpp \
               fileMisc.cpp fileVolumes.cpp fileHighlevel.cpp dcache.cpp

hfs_sources = hfsBtree.cpp hfsChanging.cpp hfsCreate.cpp hfsFile.cpp hfsHelper.cpp \
              hfsHier.cpp hfsMisc.cpp hfsVolume.cpp hfsWorkingdir.cpp hfsXbar.cpp

list_sources = listAccess.cpp listAddDel.cpp listCreate.cpp listDisplay.cpp \
               listMouse.cpp listOps.cpp listStdLDEF.cpp

menu_sources = menu.cpp menuColor.cpp menuV.cpp stdmdef.cpp stdmbdf.cpp

print_sources = PSstrings.cpp prError.cpp prInit.cpp prLowLevel.cpp \
                prPrinting.cpp prRecords.cpp PSprint.cpp 

qd_sources = qBit.cpp qCConv.cpp qCGrafPort.cpp qCRegular.cpp qColor.cpp qColorMgr.cpp    \
             qColorutil.cpp qCursor.cpp qGrafport.cpp qIMIV.cpp qIMV.cpp qIMVxfer.cpp     \
             qMisc.cpp qPaletteMgr.cpp qPen.cpp qPicstuff.cpp qPicture.cpp              \
             qPixMapConv.cpp qPoint.cpp qPoly.cpp qRect.cpp qRegion.cpp qRegular.cpp      \
             qScale.cpp qStandard.cpp qStdArc.cpp qStdBits.cpp qStdLine.cpp qStdOval.cpp  \
             qStdPic.cpp qStdPoly.cpp qStdRRect.cpp qStdRect.cpp qStdRgn.cpp            \
             qStdText.cpp qText.cpp qGWorld.cpp qGDevice.cpp qIMVI.cpp qHooks.cpp         \
             xdata.cpp xdblt.cpp rawpatblt.cpp rawsrcblt.cpp dirtyrect.cpp srcblt.cpp     \
             qColorPicker.cpp qPict2.cpp image.cpp image_inits.cpp                    \
             default_ctab_values.cpp dcconvert.cpp rgbutil.cpp dcmaketables.cpp

res_sources = resGet.cpp resGetinfo.cpp resGettype.cpp resIMIV.cpp resInit.cpp \
              resMisc.cpp resMod.cpp resOpen.cpp resSetcur.cpp resPartial.cpp

te_sources = teAccess.cpp teDisplay.cpp teEdit.cpp teIMIV.cpp teIMV.cpp teInit.cpp \
             teInsert.cpp teMisc.cpp teScrap.cpp

wind_sources = windColor.cpp windDisplay.cpp windDocdef.cpp windInit.cpp windMisc.cpp \
               windMouse.cpp windSize.cpp windUpdate.cpp

ae_sources = AE.cpp AE_desc.cpp AE_hdlr.cpp AE_coercion.cpp

sound_sources = sounddriver.cpp sound.cpp soundIMVI.cpp soundfake.cpp snth5.cpp \
                SpeechManager.cpp

num_sources = bindec.cpp float4.cpp float5.cpp float7.cpp floatnext.cpp toolmath.cpp \
              mathlib.cpp

misc_sources = desk.cpp device.cpp disk.cpp diskinit.cpp dump.cpp trapname.cpp font.cpp    \
  gestalt.cpp globals.cpp iu.cpp launch.cpp main.cpp mman.cpp mmansubr.cpp notify.cpp hle.cpp  \
  osevent.cpp osutil.cpp pack.cpp scrap.cpp script.cpp segment.cpp serial.cpp setuid.cpp     \
  slash.cpp stdfile.cpp romlib_stubs.cpp syserr.cpp toolevent.cpp toolutil.cpp time.cpp    \
  vbl.cpp syncint.cpp virtualint.cpp refresh.cpp autorefresh.cpp aboutbox.cpp            \
  keycode.cpp option.cpp parseopt.cpp parsenum.cpp desperate.cpp         \
  version.cpp shutdown.cpp uniquefile.cpp sigio_multiplex.cpp screen-dump.cpp          \
  process.cpp alias.cpp string.cpp tempmem.cpp edition.cpp fontIMVI.cpp balloon.cpp        \
  error.cpp adb.cpp color_wheel_bits.cpp finder.cpp system_error.cpp ibm_keycodes.cpp    \
  splash.cpp icon.cpp redrawscreen.cpp ini.cpp checkpoint.cpp qt.cpp cleanup.cpp           \
  paramline.c fauxdbm.cpp custom.cpp commtool.cpp cfm.cpp local_charset.cpp pef_hash.cpp \
  interfacelib.cpp mixed_mode.cpp suffix_maps.cpp appearance.cpp lockrange.cpp         \
  emutrap.cpp emutraptables.cpp emustubs.cpp unix_like.cpp parse.ypp check_structs.cpp   \
  executor.cpp  crc.cpp
#mkvol/mkvol.cpp

include_sources = hintemplate.h \
                  include/ADB.h \
                  include/AliasMgr.h \
                  include/AppleEvents.h \
                  include/AppleTalk.h \
                  include/BinaryDecimal.h \
                  include/CommTool.h \
                  include/Components.h \
                  include/ControlMgr.h \
                  include/CQuickDraw.h \
                  include/DeskMgr.h \
                  include/DeviceMgr.h \
                  include/DialogMgr.h \
                  include/Disk.h \
                  include/DiskInit.h \
                  include/EditionMgr.h \
                  include/EventMgr.h \
                  include/FileMgr.h \
                  include/Finder.h \
                  include/FontMgr.h \
                  include/Gestalt.h \
                  include/HelpMgr.h \
                  include/Iconutil.h \
                  include/IntlUtil.h \
                  include/ListMgr.h \
                  include/ExMacTypes.h \
                  include/MemoryMgr.h \
                  include/MenuMgr.h \
                  include/NotifyMgr.h \
                  include/OSEvent.h \
                  include/OSUtil.h \
                  include/Package.h \
                  include/paramline.h \
                  include/PPC.h \
                  include/PrintMgr.h \
                  include/ProcessMgr.h \
                  include/QuickDraw.h \
                  include/QuickTime.h \
                  include/ResourceMgr.h \
                  include/rsys/aboutbox.h \
                  include/rsys/aboutpanel.h \
                  include/rsys/adb.h \
                  include/rsys/alias.h \
                  include/rsys/appearance.h \
                  include/rsys/apple_events.h \
                  include/rsys/arch.h \
                  include/rsys/arrowkeys.h \
                  include/rsys/autorefresh.h \
                  include/rsys/blockdev.h \
                  include/rsys/blockinterrupts.h \
                  include/rsys/byteswap.h \
                  include/rsys/cfm.h \
                  include/rsys/check_structs.h \
                  include/rsys/checkpoint.h \
                  include/rsys/cleanup.h \
                  include/rsys/color_wheel_bits.h \
                  include/rsys/common.h \
                  include/rsys/commonevt.h \
                  include/rsys/cquick.h \
                  include/rsys/crc.h \
                  include/rsys/cruft.h \
                  include/rsys/ctl.h \
                  include/rsys/ctopflags.h \
                  include/rsys/custom.h \
                  include/rsys/dcache.h \
                  include/rsys/depthconv.h \
                  include/rsys/desk.h \
                  include/rsys/desperate.h \
                  include/rsys/device.h \
                  include/rsys/dial.h \
                  include/rsys/dirtyrect.h \
                  include/rsys/drive_flags.h \
                  include/rsys/dump.h \
                  include/rsys/emustubs.h \
                  include/rsys/error.h \
                  include/rsys/everything.h \
                  include/rsys/evil.h \
                  include/rsys/executor.h \
                  include/rsys/fauxdbm.h \
                  include/rsys/file.h \
                  include/rsys/filedouble.h \
                  include/rsys/flags.h \
                  include/rsys/float.h \
                  include/rsys/float_fcw.h \
                  include/rsys/floatconv.h \
                  include/rsys/font.h \
                  include/rsys/gestalt.h \
                  include/rsys/glue.h \
                  include/rsys/gworld.h \
                  include/rsys/hfs.h \
                  include/rsys/hfs_plus.h \
                  include/rsys/hook.h \
                  include/rsys/host.h \
                  include/rsys/icon.h \
                  include/rsys/image.h \
                  include/rsys/ini.h \
                  include/rsys/int386.h \
                  include/rsys/interfacelib.h \
                  include/rsys/itm.h \
                  include/rsys/iv.h \
                  include/rsys/keyboard.h \
                  include/rsys/keyboards.h \
                  include/rsys/keycode.h \
                  include/rsys/launch.h \
                  include/rsys/licensetext.h \
                  include/rsys/list.h \
                  include/rsys/local_charset.h \
                  include/rsys/lockunlock.h \
                  include/rsys/lowglobals.h \
                  include/rsys/m68kint.h \
                  include/rsys/macros.h \
                  include/rsys/mactype.h \
                  include/rsys/mathlib.h \
                  include/rsys/memory_layout.h \
                  include/rsys/memsize.h \
                  include/rsys/menu.h \
                  include/rsys/mixed_mode.h \
                  include/rsys/mman.h \
                  include/rsys/mman_private.h \
                  include/rsys/mmanstubs.h \
                  include/rsys/new_mmanstubs.h \
                  include/rsys/newvga.h \
                  include/rsys/next.h \
                  include/rsys/nextprint.h \
                  include/rsys/noreturn.h \
                  include/rsys/notmac.h \
                  include/rsys/option.h \
                  include/rsys/options.h \
                  include/rsys/os.h \
                  include/rsys/osevent.h \
                  include/rsys/osutil.h \
                  include/rsys/parse.h \
                  include/rsys/parsenum.h \
                  include/rsys/parseopt.h \
                  include/rsys/partition.h \
                  include/rsys/pef.h \
                  include/rsys/picture.h \
                  include/rsys/pragmal.h \
                  include/rsys/pragmar.h \
                  include/rsys/pragmas.h \
                  include/rsys/prefpanel.h \
                  include/rsys/prefs.h \
                  include/rsys/print.h \
                  include/rsys/process.h \
                  include/rsys/PSstrings.h \
                  include/rsys/pstuff.h \
                  include/rsys/ptocflags.h \
                  include/rsys/qcolor.h \
                  include/rsys/quick.h \
                  include/rsys/rawblt.h \
                  include/rsys/redrawscreen.h \
                  include/rsys/refresh.h \
                  include/rsys/region.h \
                  include/rsys/release.h \
                  include/rsys/resource.h \
                  include/rsys/rgbutil.h \
                  include/rsys/safe_alloca.h \
                  include/rsys/scrap.h \
                  include/rsys/screen-dump.h \
                  include/rsys/segment.h \
                  include/rsys/serial.h \
                  include/rsys/setuid.h \
                  include/rsys/sigio_multiplex.h \
                  include/rsys/slash.h \
                  include/rsys/sounddriver.h \
                  include/rsys/soundfake.h \
                  include/rsys/soundopts.h \
                  include/rsys/splash.h \
                  include/rsys/srcblt.h \
                  include/rsys/stdbits.h \
                  include/rsys/stdfile.h \
                  include/rsys/stdio_smashage.h \
                  include/rsys/string.h \
                  include/rsys/stubify.h \
                  include/rsys/suffix_maps.h \
                  include/rsys/syncint.h \
                  include/rsys/syserr.h \
                  include/rsys/system_error.h \
                  include/rsys/tempalloc.h \
                  include/rsys/tesave.h \
                  include/rsys/text.h \
                  include/rsys/time.h \
                  include/rsys/toolevent.h \
                  include/rsys/toolutil.h \
                  include/rsys/trapdecl.h \
                  include/rsys/trapdefines.h \
                  include/rsys/trapglue.h \
                  include/rsys/trapname.h \
                  include/rsys/types.h \
                  include/rsys/uniquefile.h \
                  include/rsys/vbl.h \
                  include/rsys/vdriver.h \
                  include/rsys/version.h \
                  include/rsys/vgavdriver.h \
                  include/rsys/wind.h \
                  include/rsys/wrappers.h \
                  include/rsys/x.h \
                  include/rsys/xdata.h \
                  include/rsys/xdblt.h \
                  include/SANE.h \
                  include/ScrapMgr.h \
                  include/ScriptMgr.h \
                  include/SegmentLdr.h \
                  include/Serial.h \
                  include/ShutDown.h \
                  include/SoundDvr.h \
                  include/SoundMgr.h \
                  include/StartMgr.h \
                  include/StdFilePkg.h \
                  include/SysErr.h \
                  include/TextEdit.h \
                  include/ThinkC.h \
                  include/TimeMgr.h \
                  include/ToolboxEvent.h \
                  include/ToolboxUtil.h \
                  include/VDriver.h \
                  include/VRetraceMgr.h \
                  include/WindowMgr.h \
                  mkvol/mkvol.h \
                  mkvol/mkvol_internal.h

# Can't put the .map files into a _SOURCES variable, or else automake will
# try to create .o files out of them, but we just need the .c files that
# map_to_c will create.

EXTRA_DIST = active.map               \
             apple.map                \
             arrow_down_active.map    \
             arrow_down_inactive.map  \
             arrow_left_active.map    \
             arrow_left_inactive.map  \
             arrow_right_active.map   \
             arrow_right_inactive.map \
             arrow_up_active.map      \
             arrow_up_inactive.map    \
             go_away.map              \
             grow.map                 \
             ractive.map              \
             thumb_horiz.map          \
             thumb_vert.map           \
             zoom.map                 \
             skel

other_sources = globals.pl           \
                makerawblt.pl        \
                genctopflags_h.tmpl  \
                geninterfacelib.tmpl \
                genptocflags_h.tmpl  \
                genstubify_h.tmpl    \
                genstubify_s.tmpl    \
                pat-blitters.tmpl    \
                src-blitters.tmpl

AM_CPPFLAGS=-I$(srcdir)/include -I$(srcdir)/config/front-ends/$(front_end) -I$(srcdir)/config/os/$(host_os) -I$(srcdir)/config/arch/$(host_cpu)

nodist_executor_SOURCES =

if CONFIG_ARCH_ALPHA
arch_sources = config/arch/alpha/alpha.cpp config/arch/alpha/alpha.h
endif CONFIG_ARCH_ALPHA

if CONFIG_ARCH_I386

CLEANFILES +=  src-blitters.h src-blitters.s pat-blitters.h pat-blitters.s \
               asmsamples.h src-blitters-stamp pat-blitters-stamp opfind

opfind_SOURCES = config/arch/i386/opfind.cpp

opfind_CPPFLAGS = -DCOMPILE_FOR_BUILD $(AM_CPPFLAGS)

x86patblt.$(OBJEXT):	pat-blitters.s

x86srcblt.$(OBJEXT):	src-blitters.s

pat-blitters-stamp pat-blitters.h pat-blitters.s:	\
	config/arch/i386/opfind.cpp config/arch/i386/opfind.h \
        config/arch/i386/metaasm.pl config/arch/i386/pat-blitters.meta
	$(PERL) $(srcdir)/config/arch/i386/metaasm.pl $(METAASM_ARGS)\
		$(srcdir)/config/arch/i386/pat-blitters.meta\
		pat-blitters.s pat-blitters.h\
		config/arch/i386/opfind.cpp
	$(RM) asmsamples.h
	touch pat-blitters-stamp

# We have src-blitters-stamp depend on pat-blitters-stamp so we don't
# try to do two metaasm's at once.  They would fight over `opfind'.
src-blitters-stamp src-blitters.h src-blitters.s: \
		config/arch/i386/opfind.cpp config/arch/i386/opfind.h \
                config/arch/i386/metaasm.pl \
                config/arch/i386/src-blitters.meta \
                config/arch/i386/src-shift.meta \
		config/arch/i386/src-noshift.meta \
                config/arch/i386/src-shift-fgbk.meta \
                config/arch/i386/src-noshift-fgbk.meta \
		pat-blitters-stamp config/arch/i386/src-blitters-core.meta
	$(PERL) $(srcdir)/config/arch/i386/metaasm.pl -define DST_SEG= $(METAASM_ARGS)\
		$(srcdir)/config/arch/i386/src-blitters.meta\
		src-blitters.s src-blitters.h\
		config/arch/i386/opfind.cpp
	touch src-blitters-stamp

arch_sources = config/arch/i386/i386.cpp \
               config/arch/i386/i386.h \
               config/arch/i386/i386_djgpp_version.h \
               config/arch/i386/opfind.h \
               config/arch/i386/metaasm.pl \
               config/arch/i386/opfind.h \
               config/arch/i386/pat-blitters.meta \
               config/arch/i386/src-blitters-core.meta \
               config/arch/i386/src-blitters.meta \
               config/arch/i386/src-noshift-fgbk.meta \
               config/arch/i386/src-noshift.meta \
               config/arch/i386/src-shift-fgbk.meta \
               config/arch/i386/src-shift.meta

if !CONFIG_OS_MACOSX

noinst_PROGRAMS += opfind

arch_sources += config/arch/i386/x86patblt.S    \
                config/arch/i386/x86srcblt.S    \
                config/arch/i386/xdstubtables.cpp \
	        config/arch/i386/sbstubtables.cpp \
                config/arch/i386/metaasm.pl

endif !CONFIG_OS_MACOSX

endif CONFIG_ARCH_I386

if CONFIG_ARCH_M68K

nodist_executor_SOURCES += m68k-callback-stubs.s

arch_sources = config/arch/m68k/m68k.cpp                  \
               config/arch/m68k/m68k-callback.cpp         \
               config/arch/m68k/m68k-callback-handler.s \
               config/arch/m68k/m68k-call-emulator.s    \
               config/arch/m68k/m68k-destroy.cpp          \
               config/arch/m68k/m68k-stack.cpp            \
               config/arch/m68k/m68k-trap-handler.s     \
               config/arch/m68k/trap.S                  \
               config/arch/m68k/m68k-stack.h            \
               config/arch/m68k/m68k.h                  \
               config/arch/m68k/syn68k_api.h            \
               config/arch/m68k/make_callback_stubs.pl

max_callbacks=4352	# 4096 plus extra slop

m68k-callback-stubs.s:	config/arch/m68k/make_callback_stubs.pl \
                        config/arch/m68k/m68k-callback-handler.s
	$(srcdir)/config/arch/m68k/make_callback_stubs.pl $(max_callbacks)\
		$(srcdir)/config/arch/m68k/m68k-callback-handler.s\
		./m68k-callback-stubs.s

# Right now we need -DNUM_CALLBACK_SLOTS on the command line when compiling
# m68k-callback.o, but I don't know of an elegant way to do this with automake.
# Since the m68k build is unlikely to work anytime soon, I'll just leave this
# code from our old build system here, but commented out.  This will fail, but
# at least it will give anyone playing with the 68k build a hint.
#
# m68k-callback.o:	m68k-callback.cpp
# 	$(BUILD_GCC) $(BUILD_CFLAGS) -DNUM_CALLBACK_SLOTS=$(max_callbacks)\
#		$(HOST_ARCH_DIR)/m68k-callback.cpp -c -o m68k-callback.o

endif CONFIG_ARCH_M68K

if CONFIG_ARCH_POWERPC

# NOTE: we used to be able to use -mcall-aix when compiling ppc_stubs and
#       get the compiler to help us run PPC binaries.  That ability hasn't
#       worked since we started doing PPC builds on Mac OS X.

arch_sources = config/arch/powerpc/powerpc.cpp   \
               config/arch/powerpc/ppc_call.cpp  \
               config/arch/powerpc/ppc_stubs.cpp \
               config/arch/powerpc/powerpc.h
               config/arch/powerpc/ppc_stubs.h

AM_CPPFLAGS += -D_GNU_SOURCE
endif CONFIG_ARCH_POWERPC

if CONFIG_ARCH_X86_64
arch_sources = config/arch/x86_64/x86_64.cpp config/arch/x86_64/x86_64.h
endif CONFIG_ARCH_X86_64


if CONFIG_FRONT_END_DOS
# The DOS port of Executor hasn't ever been built using automake.  Most likely
# we'll rip all the DOS support out of the code before ever making it go, but
# if you're adventurous and want to play, feel free.

front_end_sources = config/front-ends/dos/dosclip.cpp        \
	            config/front-ends/dos/dosdisk.cpp        \
	            config/front-ends/dos/dosevents.cpp      \
	            config/front-ends/dos/vga.cpp            \
	            config/front-ends/dos/aspi.cpp           \
	            config/front-ends/dos/dosevq.cpp         \
	            config/front-ends/dos/dpmilock.cpp       \
	            config/front-ends/dos/deintr.S         \
	            config/front-ends/dos/dosmem.cpp         \
	            config/front-ends/dos/dosserial.cpp      \
                    config/front-ends/dos/aspi.h           \
                    config/front-ends/dos/dos.h            \
                    config/front-ends/dos/dosdisk.h        \
                    config/front-ends/dos/dosevents.h      \
                    config/front-ends/dos/dosevq.h         \
                    config/front-ends/dos/dosevq_defs.h    \
                    config/front-ends/dos/dosmem.h         \
                    config/front-ends/dos/dosserial.h      \
                    config/front-ends/dos/dpmilock.h       \
                    config/front-ends/dos/host_bltmacros.h \
                    config/front-ends/dos/host_vdriver.h   \
                    config/front-ends/dos/itimer.h         \
                    config/front-ends/dos/vga.h            \
                    config/front-ends/dos/vgatables.h      \
	            vgavdriver.cpp

METAASM_ARGS = -define 'DST_SEG=%es:'

endif CONFIG_FRONT_END_DOS

if CONFIG_FRONT_END_NEXTSTEP
# It seems really unlikely that anyone will be building the NEXTSTEP version
# of Executor again, especially with the new GNU Build system.  Should you
# really wish to try, look at the Executor source on Github and look for the
# config/front-ends/nextstep directory for source and nextstep.make to see
# the source we used to use as well as some linker flags.
endif CONFIG_FRONT_END_NEXTSTEP

if CONFIG_FRONT_END_SDL

front_end_sources = config/front-ends/sdl/sdlevents.cpp      \
                    config/front-ends/sdl/sdl_mem.cpp        \
                    config/front-ends/sdl/sdlquit.cpp        \
                    config/front-ends/sdl/sdlscrap.cpp       \
                    config/front-ends/sdl/sdlwin.cpp         \
                    config/front-ends/sdl/sdlwm.cpp          \
                    config/front-ends/sdl/syswm_map.cpp      \
                    config/front-ends/sdl/for_sam.h        \
		    config/front-ends/sdl/host_bltmacros.h \
		    config/front-ends/sdl/host_vdriver.h   \
		    config/front-ends/sdl/map.h            \
		    config/front-ends/sdl/sdl.h            \
		    config/front-ends/sdl/SDL_bmp.h        \
		    config/front-ends/sdl/sdl_mem.h        \
		    config/front-ends/sdl/sdlevents.h      \
		    config/front-ends/sdl/sdlk_to_mkv.h    \
		    config/front-ends/sdl/sdlquit.h        \
		    config/front-ends/sdl/sdlscrap.h       \
		    config/front-ends/sdl/sdlX.h           \
		    config/front-ends/sdl/syswm_map.h      \
		    config/front-ends/sdl/syswm_vars.h

if CONFIG_OS_LINUX
front_end_sources += config/front-ends/sdl/sdlX.cpp
endif CONFIG_OS_LINUX

if CONFIG_OS_MACOSX
front_end_sources += config/front-ends/sdl/macosx_main.m
endif CONFIG_OS_MACOSX

endif CONFIG_FRONT_END_SDL

if CONFIG_FRONT_END_SVGALIB
# This is unlikely to work.  Nobody has built the svgalib version of Executor
# in a long time.

front_end_sources = config/front-ends/svgalib/svgalib.cpp        \
                    config/front-ends/svgalib/svgalibevent.cpp   \
                    config/front-ends/svgalib/host_bltmacros.h \
                    config/front-ends/svgalib/host_vdriver.h   \
                    config/front-ends/svgalib/svgalib.h        \
                    vgavdriver.cpp

endif CONFIG_FRONT_END_SVGALIB

if CONFIG_FRONT_END_WIN32
# This too is unlikely to work.  I believe we stopped working on the win32
# port when Sam created SDL.

front_end_sources = config/front-ends/win32/winevents.cpp      \
                    config/front-ends/win32/wincursor.cpp      \
                    config/front-ends/win32/windriver.cpp      \
		    config/front-ends/win32/host_bltmacros.h \
		    config/front-ends/win32/host_vdriver.h   \
		    config/front-ends/win32/vk_to_mkv.h      \
		    config/front-ends/win32/win32.h          \
		    config/front-ends/win32/windriver.h

endif CONFIG_FRONT_END_WIN32

if CONFIG_FRONT_END_X

front_end_sources = config/front-ends/x/x.cpp              \
                    config/front-ends/x/x_keycodes.cpp     \
		    config/front-ends/x/host_bltmacros.h \
		    config/front-ends/x/host_vdriver.h   \
		    config/front-ends/x/x.h              \
		    config/front-ends/x/x_keycodes.h

HAVE_IV = yes

endif CONFIG_FRONT_END_X

if CONFIG_HOST_ALPHA_DEC_OSF
host_sources = config/hosts/alpha-dec-osf/alpha-lowglobals-mem.cpp
else !CONFIG_HOST_ALPHA_DEC_OSF
host_sources =
endif !CONFIG_HOST_ALPHA_DEC_OSF

if CONFIG_OS_CYGWIN32
os_sources = config/os/cygwin32/cygwin32.cpp          \
             config/os/cygwin32/winfs.cpp             \
             config/os/cygwin32/win_disk.cpp          \
             config/os/cygwin32/win_stat.cpp          \
             config/os/cygwin32/win_memory.cpp        \
             config/os/cygwin32/win_serial.cpp        \
             config/os/cygwin32/win_ntcd.cpp          \
             config/os/cygwin32/win_print.cpp         \
             config/os/cygwin32/win_beep.cpp          \
             config/os/cygwin32/win_clip.cpp          \
             config/os/cygwin32/win_temp.cpp          \
             config/os/cygwin32/win_except.cpp        \
             config/os/cygwin32/win_time.cpp          \
             config/os/cygwin32/win_dongle.cpp        \
             config/os/cygwin32/win_queue.cpp         \
             config/os/cygwin32/win_screen.cpp        \
             config/os/cygwin32/win_vxdiface.cpp      \
             config/os/cygwin32/win_keyboard.cpp      \
             config/os/cygwin32/win_launch.cpp        \
             config/os/cygwin32/win_stdfile.cpp       \
	     config/os/cygwin32/aspi.h              \
             config/os/cygwin32/cdenable.h          \
	     config/os/cygwin32/cygwin32.h          \
	     config/os/cygwin32/dosdisk.h           \
	     config/os/cygwin32/main_windows.h      \
	     config/os/cygwin32/mmsystem.h          \
	     config/os/cygwin32/ntcd.h              \
	     config/os/cygwin32/sysdeps.h           \
	     config/os/cygwin32/timer.h             \
	     config/os/cygwin32/vwin32.h            \
	     config/os/cygwin32/vxdiface.h          \
	     config/os/cygwin32/win_cdenable.h      \
	     config/os/cygwin32/win_clip.h          \
	     config/os/cygwin32/win_dll.h           \
	     config/os/cygwin32/win_dongle.h        \
	     config/os/cygwin32/win_except.h        \
	     config/os/cygwin32/win_hasp.h          \
	     config/os/cygwin32/win_keyboard.h      \
	     config/os/cygwin32/win_memory.h        \
	     config/os/cygwin32/win_ntcd.h          \
	     config/os/cygwin32/win_ntinc.h         \
	     config/os/cygwin32/win_print.h         \
	     config/os/cygwin32/win_print_private.h \
	     config/os/cygwin32/win_queue.h         \
	     config/os/cygwin32/win_screen.h        \
	     config/os/cygwin32/win_sentpro.h       \
	     config/os/cygwin32/win_serial.h        \
	     config/os/cygwin32/win_stat.h          \
	     config/os/cygwin32/win_stat_private.h  \
	     config/os/cygwin32/win_temp.h          \
	     config/os/cygwin32/win_win.h           \
	     config/os/cygwin32/winfs.h

bin_PROGRAMS += exemove
exemove_SOURCES = config/os/cygwin32/exemove.cpp
endif CONFIG_OS_CYGWIN32


if CONFIG_OS_LINUX
os_sources = config/os/linux/linux.cpp          \
             config/os/linux/linux_except.cpp   \
             config/os/linux/lowglobals-mem.cpp \
             config/os/linux/linux.h          \
             config/os/linux/linux_except.h

endif CONFIG_OS_LINUX

if CONFIG_OS_MACOSX
os_sources = config/os/macosx/macosx.h
executor_LDFLAGS = -framework Cocoa
endif  CONFIG_OS_MACOSX

if CONFIG_OS_MSDOS
os_sources = config/os/msdos/msdos.cpp     \
             config/os/msdos/dpmimem.cpp   \
             config/os/msdos/dpmicall.cpp  \
             config/os/msdos/openmany.cpp  \
             config/os/msdos/rmint70.S   \
             config/os/msdos/dpmicall.h  \
             config/os/msdos/dpmimem.h   \
             config/os/msdos/interrupt.h \
             config/os/msdos/msdos.h     \
             config/os/msdos/openmany.h  \
             config/os/msdos/rmint70.h
endif CONFIG_OS_MSDOS

if CONFIG_OS_NEXT
os_sources = config/os/next/next.cpp config/os/next/next.h
endif CONFIG_OS_NEXT

if CONFIG_SOUND_DJGPP
sound_sources += config/sound/djgpp/djgpp-sound.cpp     \
                 config/sound/djgpp/djgpp-sound.h     \
                 config/sound/djgpp/sb_lib/sb_defs.h  \
                 config/sound/djgpp/sb_lib/sb_dma.h   \
                 config/sound/djgpp/sb_lib/sb_lib.h   \
                 config/sound/djgpp/sb_lib/sbdetect.h \
                 config/sound/djgpp/sb_lib/sbdriver.h
endif CONFIG_SOUND_DJGPP

if CONFIG_SOUND_LINUX
sound_sources += config/sound/linux/linux-sound.cpp \
                 config/sound/linux/linux-sound.h

endif CONFIG_SOUND_LINUX

if CONFIG_SOUND_SDL
sound_sources += config/sound/sdl/sdl-sound.cpp \
                 config/sound/sdl/sdl-sound.h
endif CONFIG_SOUND_SDL

executor_SOURCES = $(ctl_sources) $(dial_sources) $(file_sources)       \
                   $(hfs_sources) $(list_sources) $(menu_sources)       \
	           $(print_sources) $(qd_sources) $(res_sources)        \
		   $(te_sources) $(wind_sources) $(ae_sources)          \
                   $(sound_sources) $(num_sources) $(misc_sources)      \
                   $(arch_sources) $(front_end_sources) $(host_sources) \
                   $(os_sources)  $(include_sources) $(other_sources)

pkgdata_DATA =

install-data-hook:
	cp -Rp "$(srcdir)/skel" "$(pkgdatadir)/skel"

uninstall-hook:
	find "$(pkgdatadir)/skel" -type d -print0 | xargs -0 chmod u+w
	rm -rf "$(pkgdatadir)/skel"

