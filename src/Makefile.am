.DELETE_ON_ERROR:

bin_PROGRAMS = executor

noinst_PROGRAMS = map_to_c mkseedtables mkultable mksspairtable

ctl_bitmap_c_files = arrow_up_active.c arrow_up_inactive.c       \
                     arrow_down_active.c arrow_down_inactive.c   \
                     arrow_right_active.c arrow_right_inactive.c \
                     arrow_left_active.c arrow_left_inactive.c   \
                     thumb_horiz.c thumb_vert.c

wind_bitmap_c_files = zoom.c go_away.c active.c ractive.c grow.c

CLEANFILES = $(ctl_bitmap_c_files) $(wind_bitmap_c_files)  apple.c \
             seedtables.c ultable.c sspairtable.c rawpatstubs.c rawsrcstubs.c

$(ctl_bitmap_c_files) apple.c $(wind_bitmap_c_files):	map_to_c

.map.c:	map_to_c
	./map_to_c < $< > $@

seedtables.c:	mkseedtables
	./mkseedtables > $@

ultable.c:	mkultable
	./mkultable > $@

sspairtable.c:	mksspairtable
	./mksspairtable > $@

rawpatstubs.c:	$(srcdir)/makerawblt.pl $(srcdir)/pat-blitters.tmpl
	$(PERL) $(srcdir)/makerawblt.pl < $(srcdir)/pat-blitters.tmpl > rawpatstubs.c

rawsrcstubs.c: $(srcdir)/makerawblt.pl $(srcdir)/src-blitters.tmpl
	$(PERL) $(srcdir)/makerawblt.pl < $(srcdir)/src-blitters.tmpl > rawsrcstubs.c

ctlArrows.$(OBJEXT):	$(ctl_bitmap_c_files)
stdmbdf.$(OBJEXT):	apple.c
qIMIV.$(OBJEXT):	seedtables.c
qStdText.$(OBJEXT):	ultable.c
qIMVI.$(OBJEXT):	sspairtable.c
rawpatblt.$(OBJEXT):	rawpatstubs.c
rawsrcblt.$(OBJEXT):	rawsrcstubs.c
windDocdef.$(OBJEXT):	$(wind_bitmap_c_files)

# These helper variables break the source into logical groups, but there's
# no actual significance to them.

ctl_sources = ctlArrows.c ctlDisplay.c ctlIMIV.c ctlInit.c ctlMisc.c \
              ctlMouse.c ctlSet.c ctlSize.c ctlStddef.c ctlPopup.c

dial_sources = dialAlert.c dialCreate.c dialHandle.c dialInit.c dialManip.c \
               dialDispatch.c dialItem.c

file_sources = fileAccess.c fileCreate.c fileDirs.c fileDouble.c fileInfo.c \
               fileMisc.c fileVolumes.c fileHighlevel.c dcache.c

hfs_sources = hfsBtree.c hfsChanging.c hfsCreate.c hfsFile.c hfsHelper.c \
              hfsHier.c hfsMisc.c hfsVolume.c hfsWorkingdir.c hfsXbar.c

list_sources = listAccess.c listAddDel.c listCreate.c listDisplay.c \
               listMouse.c listOps.c listStdLDEF.c

menu_sources = menu.c menuColor.c menuV.c stdmdef.c stdmbdf.c

print_sources = PSprint.c PSstrings.c prError.c prInit.c prLowLevel.c \
                prPrinting.c prRecords.c

qd_sources = qBit.c qCConv.c qCGrafPort.c qCRegular.c qColor.c qColorMgr.c    \
             qColorutil.c qCursor.c qGrafport.c qIMIV.c qIMV.c qIMVxfer.c     \
             qMisc.c qPaletteMgr.c qPen.c qPicstuff.c qPicture.c              \
             qPixMapConv.c qPoint.c qPoly.c qRect.c qRegion.c qRegular.c      \
             qScale.c qStandard.c qStdArc.c qStdBits.c qStdLine.c qStdOval.c  \
             qStdPic.c qStdPoly.c qStdRRect.c qStdRect.c qStdRgn.c            \
             qStdText.c qText.c qGWorld.c qGDevice.c qIMVI.c qHooks.c         \
             xdata.c xdblt.c rawpatblt.c rawsrcblt.c dirtyrect.c srcblt.c     \
             qColorPicker.c qPict2.c image.c image_inits.c                    \
             default_ctab_values.c dcconvert.c rgbutil.c dcmaketables.c

res_sources = resGet.c resGetinfo.c resGettype.c resIMIV.c resInit.c \
              resMisc.c resMod.c resOpen.c resSetcur.c resPartial.c

te_sources = teAccess.c teDisplay.c teEdit.c teIMIV.c teIMV.c teInit.c \
             teInsert.c teMisc.c teScrap.c

wind_sources = windColor.c windDisplay.c windDocdef.c windInit.c windMisc.c \
               windMouse.c windSize.c windUpdate.c

ae_sources = AE.c AE_desc.c AE_hdlr.c AE_coercion.c

sound_sources = sounddriver.c sound.c soundIMVI.c soundfake.c snth5.c

num_sources = bindec.c float4.c float5.c float7.c floatnext.c toolmath.c \
              mathlib.c

misc_sources = desk.c device.c disk.c diskinit.c dump.c trapname.c font.c    \
  gestalt.c globals.c iu.c launch.c main.c mman.c mmansubr.c notify.c hle.c  \
  osevent.c osutil.c pack.c scrap.c script.c segment.c serial.c setuid.c     \
  slash.c stdfile.c romlib_stubs.c syserr.c toolevent.c toolutil.c time.c    \
  vbl.c syncint.c virtualint.c refresh.c autorefresh.c aboutbox.c            \
  licensetext.c keycode.c option.c parseopt.c parsenum.c desperate.c         \
  version.c shutdown.c uniquefile.c sigio_multiplex.c screen-dump.c          \
  process.c alias.c string.c tempmem.c edition.c fontIMVI.c balloon.c        \
  error.c adb.c color_wheel_bits.c finder.c system_error.c ibm_keycodes.c    \
  splash.c icon.c redrawscreen.c ini.c checkpoint.c qt.c cleanup.c           \
  paramline.c fauxdbm.c custom.c commtool.c cfm.c local_charset.c pef_hash.c \
  interfacelib.c mixed_mode.c suffix_maps.c appearance.c lockrange.c         \
  emutrap.c emutraptables.c emustubs.c unix_like.c parse.y check_structs.c   \
  executor.c mkvol/mkvol.c crc.c

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
                  include/MacTypes.h \
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
                  include/rsys/jumpvectors.h \
                  include/rsys/keyboard.h \
                  include/rsys/keyboards.h \
                  include/rsys/keycode.h \
                  include/rsys/launch.h \
                  include/rsys/libcproto.h \
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
                  include/rsys/misc.h \
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
                  include/rsys/Olibcproto.h \
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
                  include/rsys/smash.h \
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
                  include/rsys/Xdefs.h \
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
arch_sources = config/arch/alpha/alpha.c config/arch/alpha/alpha.h
endif CONFIG_ARCH_ALPHA

if CONFIG_ARCH_I386

CLEANFILES +=  src-blitters.h src-blitters.s pat-blitters.h pat-blitters.s \
               asmsamples.h src-blitters-stamp pat-blitters-stamp opfind

opfind_SOURCES = config/arch/i386/opfind.c

opfind_CPPFLAGS = -DCOMPILE_FOR_BUILD $(AM_CPPFLAGS)

x86patblt.$(OBJEXT):	pat-blitters.s

x86srcblt.$(OBJEXT):	src-blitters.s

pat-blitters-stamp pat-blitters.h pat-blitters.s:	\
	config/arch/i386/opfind.c config/arch/i386/opfind.h \
        config/arch/i386/metaasm.pl config/arch/i386/pat-blitters.meta
	$(PERL) $(srcdir)/config/arch/i386/metaasm.pl $(METAASM_ARGS)\
		$(srcdir)/config/arch/i386/pat-blitters.meta\
		pat-blitters.s pat-blitters.h\
		config/arch/i386/opfind.c
	$(RM) asmsamples.h
	touch pat-blitters-stamp

# We have src-blitters-stamp depend on pat-blitters-stamp so we don't
# try to do two metaasm's at once.  They would fight over `opfind'.
src-blitters-stamp src-blitters.h src-blitters.s: \
		config/arch/i386/opfind.c config/arch/i386/opfind.h \
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
		config/arch/i386/opfind.c
	touch src-blitters-stamp

arch_sources = config/arch/i386/i386.c \
               config/arch/i386/i386.h \
               config/arch/i386/i386_djgpp_version.h \
               config/arch/i386/opfind.h

if !CONFIG_OS_MACOSX

noinst_PROGRAMS += opfind

arch_sources += config/arch/i386/x86patblt.S    \
                config/arch/i386/x86srcblt.S    \
                config/arch/i386/xdstubtables.c \
	        config/arch/i386/sbstubtables.c \
                config/arch/i386/metaasm.pl

endif !CONFIG_OS_MACOSX

endif CONFIG_ARCH_I386

if CONFIG_ARCH_M68K

nodist_executor_SOURCES += m68k-callback-stubs.s

arch_sources = config/arch/m68k/m68k.c                  \
               config/arch/m68k/m68k-callback.c         \
               config/arch/m68k/m68k-callback-handler.s \
               config/arch/m68k/m68k-call-emulator.s    \
               config/arch/m68k/m68k-destroy.c          \
               config/arch/m68k/m68k-stack.c            \
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
# m68k-callback.o:	m68k-callback.c
# 	$(BUILD_GCC) $(BUILD_CFLAGS) -DNUM_CALLBACK_SLOTS=$(max_callbacks)\
#		$(HOST_ARCH_DIR)/m68k-callback.c -c -o m68k-callback.o

endif CONFIG_ARCH_M68K

if CONFIG_ARCH_POWERPC

# NOTE: we used to be able to use -mcall-aix when compiling ppc_stubs and
#       get the compiler to help us run PPC binaries.  That ability hasn't
#       worked since we started doing PPC builds on Mac OS X.

arch_sources = config/arch/powerpc/powerpc.c   \
               config/arch/powerpc/ppc_call.c  \
               config/arch/powerpc/ppc_stubs.c \
               config/arch/powerpc/powerpc.h
               config/arch/powerpc/ppc_stubs.h

AM_CPPFLAGS += -D_GNU_SOURCE
endif CONFIG_ARCH_POWERPC

if CONFIG_ARCH_X86_64
arch_sources = config/arch/x86_64/x86_64.c config/arch/x86_64/x86_64.h
endif CONFIG_ARCH_X86_64


if CONFIG_FRONT_END_DOS
# The DOS port of Executor hasn't ever been built using automake.  Most likely
# we'll rip all the DOS support out of the code before ever making it go, but
# if you're adventurous and want to play, feel free.

front_end_sources = config/front-ends/dos/dosclip.c        \
	            config/front-ends/dos/dosdisk.c        \
	            config/front-ends/dos/dosevents.c      \
	            config/front-ends/dos/vga.c            \
	            config/front-ends/dos/aspi.c           \
	            config/front-ends/dos/dosevq.c         \
	            config/front-ends/dos/dpmilock.c       \
	            config/front-ends/dos/deintr.S         \
	            config/front-ends/dos/dosmem.c         \
	            config/front-ends/dos/dosserial.c      \
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
	            vgavdriver.c

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

front_end_sources = config/front-ends/sdl/SDL_bmp.c        \
                    config/front-ends/sdl/sdlevents.c      \
                    config/front-ends/sdl/sdl_mem.c        \
                    config/front-ends/sdl/sdlquit.c        \
                    config/front-ends/sdl/sdlscrap.c       \
                    config/front-ends/sdl/sdlwin.c         \
                    config/front-ends/sdl/sdlwm.c          \
                    config/front-ends/sdl/syswm_map.c      \
                    config/front-ends/sdl/winmain.c        \
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
front_end_sources += config/front-ends/sdl/sdlX.c
endif CONFIG_OS_LINUX

if CONFIG_OS_MACOSX
front_end_sources += config/front-ends/sdl/macosx_main.m
endif CONFIG_OS_MACOSX

endif CONFIG_FRONT_END_SDL

if CONFIG_FRONT_END_SVGALIB
# This is unlikely to work.  Nobody has built the svgalib version of Executor
# in a long time.

front_end_sources = config/front-ends/svgalib/svgalib.c        \
                    config/front-ends/svgalib/svgalibevent.c   \
                    config/front-ends/svgalib/host_bltmacros.h \
                    config/front-ends/svgalib/host_vdriver.h   \
                    config/front-ends/svgalib/svgalib.h        \
                    vgavdriver.c

endif CONFIG_FRONT_END_SVGALIB

if CONFIG_FRONT_END_WIN32
# This too is unlikely to work.  I believe we stopped working on the win32
# port when Sam created SDL.

front_end_sources = config/front-ends/win32/winevents.c      \
                    config/front-ends/win32/wincursor.c      \
                    config/front-ends/win32/windriver.c      \
		    config/front-ends/win32/host_bltmacros.h \
		    config/front-ends/win32/host_vdriver.h   \
		    config/front-ends/win32/vk_to_mkv.h      \
		    config/front-ends/win32/win32.h          \
		    config/front-ends/win32/windriver.h

endif CONFIG_FRONT_END_WIN32

if CONFIG_FRONT_END_X

front_end_sources = config/front-ends/x/x.c              \
                    config/front-ends/x/x_keycodes.c     \
		    config/front-ends/x/host_bltmacros.h \
		    config/front-ends/x/host_vdriver.h   \
		    config/front-ends/x/x.h              \
		    config/front-ends/x/x_keycodes.h

HAVE_IV = yes

endif CONFIG_FRONT_END_X

if CONFIG_HOST_ALPHA_DEC_OSF
host_sources = config/hosts/alpha-dec-osf/alpha-lowglobals-mem.c
else !CONFIG_HOST_ALPHA_DEC_OSF
host_sources =
endif !CONFIG_HOST_ALPHA_DEC_OSF

if CONFIG_OS_CYGWIN32
os_sources = config/os/cygwin32/cygwin32.c          \
             config/os/cygwin32/winfs.c             \
             config/os/cygwin32/win_disk.c          \
             config/os/cygwin32/win_stat.c          \
             config/os/cygwin32/win_memory.c        \
             config/os/cygwin32/win_serial.c        \
             config/os/cygwin32/win_ntcd.c          \
             config/os/cygwin32/win_print.c         \
             config/os/cygwin32/win_beep.c          \
             config/os/cygwin32/win_clip.c          \
             config/os/cygwin32/win_temp.c          \
             config/os/cygwin32/win_except.c        \
             config/os/cygwin32/win_time.c          \
             config/os/cygwin32/win_dongle.c        \
             config/os/cygwin32/win_queue.c         \
             config/os/cygwin32/win_screen.c        \
             config/os/cygwin32/win_vxdiface.c      \
             config/os/cygwin32/win_keyboard.c      \
             config/os/cygwin32/win_launch.c        \
             config/os/cygwin32/win_stdfile.c       \
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
exemove_SOURCES = config/os/cygwin32/exemove.c
endif CONFIG_OS_CYGWIN32


if CONFIG_OS_LINUX
os_sources = config/os/linux/linux.c          \
             config/os/linux/linux_except.c   \
             config/os/linux/lowglobals-mem.c \
             config/os/linux/linux.h          \
             config/os/linux/linux_except.h

endif CONFIG_OS_LINUX

if CONFIG_OS_MACOSX
os_sources = config/os/macosx/macosx.h
executor_LDFLAGS = -framework SDL -framework Cocoa
endif  CONFIG_OS_MACOSX

if CONFIG_OS_MSDOS
os_sources = config/os/msdos/msdos.c     \
             config/os/msdos/dpmimem.c   \
             config/os/msdos/dpmicall.c  \
             config/os/msdos/openmany.c  \
             config/os/msdos/rmint70.S   \
             config/os/msdos/dpmicall.h  \
             config/os/msdos/dpmimem.h   \
             config/os/msdos/interrupt.h \
             config/os/msdos/msdos.h     \
             config/os/msdos/openmany.h  \
             config/os/msdos/rmint70.h
endif CONFIG_OS_MSDOS

if CONFIG_OS_NEXT
os_sources = config/os/next/next.c config/os/next/next.h
endif CONFIG_OS_NEXT

if CONFIG_SOUND_DJGPP
sound_sources += config/sound/djgpp/djgpp-sound.c     \
                 config/sound/djgpp/djgpp-sound.h     \
                 config/sound/djgpp/sb_lib/sb_defs.h  \
                 config/sound/djgpp/sb_lib/sb_dma.h   \
                 config/sound/djgpp/sb_lib/sb_lib.h   \
                 config/sound/djgpp/sb_lib/sbdetect.h \
                 config/sound/djgpp/sb_lib/sbdriver.h
endif CONFIG_SOUND_DJGPP

if CONFIG_SOUND_LINUX
sound_sources += config/sound/linux/linux-sound.c \
                 config/sound/linux/linux-sound.h

endif CONFIG_SOUND_LINUX

if CONFIG_SOUND_SDL
sound_sources += config/sound/sdl/sdl-sound.c \
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
	cp -Rp $(srcdir)/skel $(pkgdatadir)/skel
