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

AM_CPPFLAGS=-I$(srcdir)/include -I$(srcdir)/config/front-ends/$(front_end) -I$(srcdir)/config/os/$(host_os) -I$(srcdir)/config/arch/$(host_cpu)

if CONFIG_ARCH_ALPHA
arch_sources = config/arch/alpha/alpha.c
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

arch_sources = config/arch/i386/i386.c


if !CONFIG_OS_MACOSX

noinst_PROGRAMS += opfind

arch_sources += config/arch/i386/x86patblt.S    \
                config/arch/i386/x86srcblt.S    \
                config/arch/i386/xdstubtables.c \
	        config/arch/i386/sbstubtables.c

endif !CONFIG_OS_MACOSX

endif CONFIG_ARCH_I386

if CONFIG_ARCH_M68K

arch_sources = config/arch/m68k/m68k.c                  \
               config/arch/m68k/m68k-callback.c         \
               config/arch/m68k/m68k-callback-handler.s \
               config/arch/m68k/m68k-call-emulator.s    \
               config/arch/m68k/m68k-destroy.c          \
               config/arch/m68k/m68k-stack.c            \
               config/arch/m68k/m68k-trap-handler.s     \
               config/arch/m68k/trap.S                  \
               config/arch/m68k/m68k-callback-stubs.s

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

arch_sources = config/arch/powerpc/powerpc.c  \
               config/arch/powerpc/ppc_call.c \
               config/arch/powerpc/ppc_stubs.c
AM_CPPFLAGS += -D_GNU_SOURCE
endif CONFIG_ARCH_POWERPC

if CONFIG_FRONT_END_DOS
# The DOS port of Executor hasn't ever been built using automake.  Most likely
# we'll rip all the DOS support out of the code before ever making it go, but
# if you're adventurous and want to play, feel free.

front_end_sources = config/front-ends/dos/dosclip.c    \
	            config/front-ends/dos/dosdisk.c    \
	            config/front-ends/dos/dosevents.c  \
	            config/front-ends/dos/vga.c        \
	            config/front-ends/dos/aspi.c       \
	            config/front-ends/dos/dosevq.c     \
	            config/front-ends/dos/dpmilock.c   \
	            config/front-ends/dos/deintr.S     \
	            config/front-ends/dos/dosmem.c     \
	            config/front-ends/dos/dosserial.c  \
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

front_end_sources = config/front-ends/sdl/SDL_bmp.c   \
                    config/front-ends/sdl/sdlevents.c \
                    config/front-ends/sdl/sdl_mem.c   \
                    config/front-ends/sdl/sdlquit.c   \
                    config/front-ends/sdl/sdlscrap.c  \
                    config/front-ends/sdl/sdlwin.c    \
                    config/front-ends/sdl/sdlwm.c     \
                    config/front-ends/sdl/syswm_map.c \
                    config/front-ends/sdl/winmain.c

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

front_end_sources = config/front-ends/svgalib/svgalib.c      \
                    config/front-ends/svgalib/svgalibevent.c \
                    vgavdriver.c

endif CONFIG_FRONT_END_SVGALIB

if CONFIG_FRONT_END_WIN32
# This too is unlikely to work.  I believe we stopped working on the win32
# port when Sam created SDL.

front_end_sources = config/front-ends/win32/winevents.c \
                    config/front-ends/win32/wincursor.c \
                    config/front-ends/win32/windriver.c

endif CONFIG_FRONT_END_WIN32

if CONFIG_FRONT_END_X

front_end_sources = config/front-ends/x/x.c config/front-ends/x/x_keycodes.c
HAVE_IV = yes

endif CONFIG_FRONT_END_X

if CONFIG_HOST_ALPHA_DEC_OSF
host_sources = config/hosts/alpha-dec-osf/alpha-lowglobals-mem.c
else !CONFIG_HOST_ALPHA_DEC_OSF
host_sources =
endif !CONFIG_HOST_ALPHA_DEC_OSF

if CONFIG_OS_CYGWIN32
os_sources = config/os/cygwin32/cygwin32.c     \
             config/os/cygwin32/winfs.c        \
             config/os/cygwin32/win_disk.c     \
             config/os/cygwin32/win_stat.c     \
             config/os/cygwin32/win_memory.c   \
             config/os/cygwin32/win_serial.c   \
             config/os/cygwin32/win_ntcd.c     \
             config/os/cygwin32/win_print.c    \
             config/os/cygwin32/win_beep.c     \
             config/os/cygwin32/win_cookie.c   \
             config/os/cygwin32/win_clip.c     \
             config/os/cygwin32/win_temp.c     \
             config/os/cygwin32/win_except.c   \
             config/os/cygwin32/win_time.c     \
             config/os/cygwin32/win_dongle.c   \
             config/os/cygwin32/win_queue.c    \
             config/os/cygwin32/win_screen.c   \
             config/os/cygwin32/win_vxdiface.c \
             config/os/cygwin32/win_keyboard.c \
             config/os/cygwin32/win_launch.c   \
             config/os/cygwin32/win_stdfile.c

bin_PROGRAMS += exemove
endif CONFIG_OS_CYGWIN32


if CONFIG_OS_LINUX
os_sources = config/os/linux/linux.c          \
             config/os/linux/linux_except.c   \
             config/os/linux/lowglobals-mem.c
endif CONFIG_OS_LINUX

if CONFIG_OS_MACOSX
executor_LDFLAGS = -framework SDL -framework Cocoa
endif  CONFIG_OS_MACOSX

if CONFIG_OS_MSDOS
os_sources = config/os/msdos/msdos.c    \
             config/os/msdos/dpmimem.c  \
             config/os/msdos/dpmicall.c \
             config/os/msdos/openmany.c \
             config/os/msdos/rmint70.S
endif CONFIG_OS_MSDOS

if CONFIG_OS_NEXT
os_sources = next.c
endif CONFIG_OS_NEXT

if CONFIG_SOUND_DJGPP
sound_sources += config/sound/djgpp/djgpp-sound.c
endif CONFIG_SOUND_DJGPP

if CONFIG_SOUND_LINUX
sound_sources += config/sound/linux/linux-sound.c
endif CONFIG_SOUND_LINUX

if CONFIG_SOUND_SDL
sound_sources += config/sound/sdl/sdl-sound.c
endif CONFIG_SOUND_SDL

executor_SOURCES = $(ctl_sources) $(dial_sources) $(file_sources)       \
                   $(hfs_sources) $(list_sources) $(menu_sources)       \
	           $(print_sources) $(qd_sources) $(res_sources)        \
		   $(te_sources) $(wind_sources) $(ae_sources)          \
                   $(sound_sources) $(num_sources) $(misc_sources)      \
                   $(arch_sources) $(front_end_sources) $(host_sources) \
                   $(os_sources) 
