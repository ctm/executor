.DELETE_ON_ERROR:

bin_PROGRAMS = executor

noinst_PROGRAMS = map_to_c mkseedtables mkultable mksspairtable opfind

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

opfind_SOURCES = config/arch/i386/opfind.c

opfind_CPPFLAGS = -DCOMPILE_FOR_BUILD $(AM_CPPFLAGS)

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

front_end_sdl_sources = config/front-ends/sdl/SDL_bmp.c   \
                        config/front-ends/sdl/sdlevents.c \
                        config/front-ends/sdl/sdl_mem.c   \
                        config/front-ends/sdl/sdlquit.c   \
                        config/front-ends/sdl/sdlscrap.c  \
                        config/front-ends/sdl/sdlwin.c    \
                        config/front-ends/sdl/sdlwm.c     \
                        config/front-ends/sdl/sdlX.c      \
                        config/front-ends/sdl/syswm_map.c \
                        config/front-ends/sdl/winmain.c

sound_sdl_sources = config/sound/sdl/sdl-sound.c

os_linux_sources = config/os/linux/linux.c          \
                   config/os/linux/linux_except.c   \
                   config/os/linux/lowglobals-mem.c

arch_alpha_sources = config/arch/alpha/alpha.c

arch_i386_sources = config/arch/i386/x86patblt.S    \
                    config/arch/i386/x86srcblt.S    \
                    config/arch/i386/xdstubtables.c \
	            config/arch/i386/sbstubtables.c \
                    config/arch/i386/i386.c

# TODO: If we ever support the m68k target again we'll need to make it so that
#       we build m68k-callback-stubs.s

arch_m68k_sources = config/arch/m68k/m68k.c                  \
                    config/arch/m68k/m68k-callback.c         \
                    config/arch/m68k/m68k-callback-handler.s \
                    config/arch/m68k/m68k-call-emulator.s    \
                    config/arch/m68k/m68k-destroy.c          \
                    config/arch/m68k/m68k-stack.c            \
                    config/arch/m68k/m68k-trap-handler.s     \
                    config/arch/m68k/trap.S                  \
                    config/arch/m68k/m68k-callback-stubs.s

x86patblt.$(OBJEXT):	pat-blitters.s

x86srcblt.$(OBJEXT):	src-blitters.s

pat-blitters-stamp pat-blitters.h pat-blitters.s:	\
	config/arch/i386/opfind.c config/arch/i386/opfind.h \
        config/arch/i386/metaasm.pl config/arch/i386/pat-blitters.meta
	$(PERL) $(srcdir)/config/arch/i386/metaasm.pl $(METAASM_ARGS)\
		$(srcdir)/config/arch/i386/pat-blitters.meta\
		pat-blitters.s pat-blitters.h\
		config/arch/i386/opfind.c
	$(RM) opfind asmsamples.h
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
	$(RM) opfind
	touch src-blitters-stamp

executor_SOURCES = $(ctl_sources) $(dial_sources) $(file_sources)  \
                   $(hfs_sources) $(list_sources) $(menu_sources)  \
	           $(print_sources) $(qd_sources) $(res_sources)   \
		   $(te_sources) $(wind_sources) $(ae_sources)     \
                   $(sound_sources) $(num_sources) $(misc_sources) \
                   $(front_end_sdl_sources) $(sound_sdl_sources)   \
                   $(os_linux_sources) $(arch_i386_sources)

AM_CPPFLAGS=-I$(srcdir)/include -I$(srcdir)/config/front-ends/$(front_end) -I$(srcdir)/config/os/$(host_os) -I$(srcdir)/config/arch/$(host_cpu)
