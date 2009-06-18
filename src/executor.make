CTL_SRC	=								\
  ctlArrows.c   ctlDisplay.c  ctlIMIV.c     ctlInit.c     ctlMisc.c	\
  ctlMouse.c    ctlSet.c      ctlSize.c     ctlStddef.c   ctlPopup.c

DIAL_SRC =								\
  dialAlert.c   dialCreate.c  dialHandle.c  dialInit.c    dialManip.c	\
  dialDispatch.c dialItem.c

FILE_SRC =								 \
  fileAccess.c   fileCreate.c   fileDirs.c     fileDouble.c   fileInfo.c \
  fileMisc.c     fileVolumes.c	fileHighlevel.c dcache.c

HFS_SRC =								\
  hfsBtree.c       hfsChanging.c    hfsCreate.c      hfsFile.c		\
  hfsHelper.c      hfsHier.c        hfsMisc.c        hfsVolume.c	\
  hfsWorkingdir.c  hfsXbar.c

LIST_SRC =								  \
  listAccess.c   listAddDel.c   listCreate.c   listDisplay.c  listMouse.c \
  listOps.c      listStdLDEF.c

MENU_SRC =							\
  menu.c       menuColor.c  menuV.c stdmdef.c stdmbdf.c

PR_SRC =								\
  PSprint.c     PSstrings.c   prError.c     prInit.c      prLowLevel.c	\
  prPrinting.c  prRecords.c

QD_SRC =								    \
  qBit.c                        qCConv.c       qCGrafPort.c   qCRegular.c   \
  qColor.c       qColorMgr.c    qColorutil.c   qCursor.c      qGrafport.c   \
  qIMIV.c        qIMV.c         qIMVxfer.c     qMisc.c        qPaletteMgr.c \
  qPen.c         qPicstuff.c    qPicture.c     qPixMapConv.c  qPoint.c	    \
  qPoly.c        qRect.c        qRegion.c      qRegular.c     qScale.c	    \
  qStandard.c    qStdArc.c      qStdBits.c     qStdLine.c     qStdOval.c    \
  qStdPic.c      qStdPoly.c     qStdRRect.c    qStdRect.c     qStdRgn.c	    \
  qStdText.c     qText.c	qGWorld.c      qGDevice.c     qIMVI.c	    \
  qHooks.c	 xdata.c	xdblt.c	       rawpatblt.c    rawsrcblt.c   \
  dirtyrect.c    srcblt.c	qColorPicker.c qPict2.c

RES_SRC =								\
  resGet.c      resGetinfo.c  resGettype.c  resIMIV.c     resInit.c	\
  resMisc.c     resMod.c      resOpen.c     resSetcur.c	  resPartial.c

TE_SRC =								    \
  teAccess.c   teDisplay.c  teEdit.c     teIMIV.c     teIMV.c      teInit.c \
  teInsert.c   teMisc.c     teScrap.c

WIND_SRC =								 \
  windColor.c    windDisplay.c  windDocdef.c   windInit.c     windMisc.c \
  windMouse.c    windSize.c     windUpdate.c

AE_SRC =								\
  AE.c AE_desc.c AE_hdlr.c AE_coercion.c

AUX_SRC	=								\
  bindec.c default_ctab_values.c desk.c device.c disk.c diskinit.c	\
  dump.c trapname.c float4.c float5.c float7.c floatnext.c font.c	\
  gestalt.c globals.c							\
  image.c image_inits.c iu.c launch.c main.c mman.c mmansubr.c		\
  notify.c hle.c osevent.c osutil.c pack.c scrap.c script.c	        \
  segment.c serial.c setuid.c slash.c 					\
  sounddriver.c sound.c soundIMVI.c soundfake.c				\
  stdfile.c romlib_stubs.c						\
  snth5.c syserr.c toolevent.c toolmath.c toolutil.c time.c vbl.c	\
  syncint.c virtualint.c refresh.c autorefresh.c			\
  aboutbox.c licensetext.c dcmaketables.c			        \
  dcconvert.c rgbutil.c keycode.c option.c parseopt.c parsenum.c	\
  desperate.c								\
  version.c shutdown.c uniquefile.c sigio_multiplex.c			\
  screen-dump.c mkvol/mkvol.c process.c alias.c string.c tempmem.c	\
  edition.c fontIMVI.c balloon.c error.c adb.c color_wheel_bits.c	\
  finder.c system_error.c ibm_keycodes.c splash.c icon.c		\
  redrawscreen.c ini.c checkpoint.c qt.c cleanup.c paramline.c          \
  fauxdbm.c custom.c commtool.c cfm.c local_charset.c pef_hash.c        \
  mathlib.c interfacelib.c mixed_mode.c suffix_maps.c appearance.c	\
  lockrange.c unix_like.c check_structs.c

ROMLIB_SRC = $(CTL_SRC) $(DIAL_SRC) $(FILE_SRC) $(HFS_SRC) $(LIST_SRC) \
  $(MENU_SRC) $(PR_SRC) $(QD_SRC) $(RES_SRC) $(TE_SRC) $(WIND_SRC)     \
  $(AE_SRC) $(AUX_SRC)

ROMLIB_OBJ = $(strip $(notdir $(addsuffix .o,$(basename $(ROMLIB_SRC)))))

ROMLIB_DEFINES =

MAP_SRC =						\
  apple.map						\
  arrow_up_active.map arrow_up_inactive.map		\
  arrow_down_active.map arrow_down_inactive.map		\
  arrow_right_active.map arrow_right_inactive.map	\
  arrow_left_active.map arrow_left_inactive.map		\
  thumb_horiz.map thumb_vert.map			\
  active.map go_away.map grow.map ractive.map zoom.map
MAP_C =	$(MAP_SRC:.map=.c)

$(build_obj_dir)/mksspairtable: $(build_obj_dir_stamp) 
$(build_obj_dir)/mksspairtable: mksspairtable.c
	$(BUILD_GCC) $(BUILD_CFLAGS) -o $(build_obj_dir)/mksspairtable $<
sspairtable.c: $(build_obj_dir)/mksspairtable
	rm -f sspairtable.c
	$(build_obj_dir)/mksspairtable > sspairtable.c
	chmod -w sspairtable.c

$(build_obj_dir)/mkultable: $(build_obj_dir_stamp) 
$(build_obj_dir)/mkultable: mkultable.c
	$(BUILD_GCC) $(BUILD_CFLAGS) -o $(build_obj_dir)/mkultable $<
ultable.c: $(build_obj_dir)/mkultable
	rm -f ultable.c
	$(build_obj_dir)/mkultable > ultable.c
	chmod -w ultable.c

$(build_obj_dir)/mkseedtables: $(build_obj_dir_stamp)
$(build_obj_dir)/mkseedtables: mkseedtables.c
	$(BUILD_GCC) $(BUILD_CFLAGS) -o $(build_obj_dir)/mkseedtables $<

seedtables.c: $(build_obj_dir)/mkseedtables
	rm -f seedtables.c
	$(build_obj_dir)/mkseedtables > seedtables.c
	chmod -w seedtables.c

rawpatstubs.c: makerawblt.pl pat-blitters.tmpl
	$(SRC_DIR)/makerawblt.pl < $(SRC_DIR)/pat-blitters.tmpl > rawpatstubs.c
	-indent -T uint8 -T uint32 -T blt_section_t rawpatstubs.c
	-rm rawpatstubs.c~

rawpatblt.o: rawpatblt.c rawpatstubs.c

rawsrcstubs.c: makerawblt.pl src-blitters.tmpl
	$(SRC_DIR)/makerawblt.pl < $(SRC_DIR)/src-blitters.tmpl > rawsrcstubs.c
	-indent -T uint8 -T uint32 -T blt_section_t rawsrcstubs.c
	-rm rawsrcstubs.c~

rawsrcblt.o: rawsrcblt.c rawsrcstubs.c

qIMIV.o: seedtables.c

$(build_obj_dir)/mkexpandtables: $(build_obj_dir_stamp)
$(build_obj_dir)/mkexpandtables: mkexpandtables.c
	$(BUILD_GCC) $(BUILD_CFLAGS) -o $(build_obj_dir)/mkexpandtables $<

expandtables.c: $(build_obj_dir)/mkexpandtables
	rm -f expandtables.c
	$(build_obj_dir)/mkexpandtables > expandtables.c
	chmod -w expandtables.c

globals.o: globals.c
	rm -f tmp-globals.c
	awk -F, '$$5 !~ /false/ && $$5 !~ /true-b/' $< > tmp-globals.c
	$(HOST_GCC) $(HOST_CFLAGS) -c -o globals.o tmp-globals.c
	rm -f tmp-globals.c

$(build_obj_dir)/map_to_c: $(build_obj_dir_stamp)
$(build_obj_dir)/map_to_c: map_to_c.c
	$(BUILD_GCC) -O1 $(GEN_CFLAGS) -o $(build_obj_dir)/map_to_c $<
$(MAP_C): $(build_obj_dir)/map_to_c
.map.c: $<
	rm -f $*.c
	$(build_obj_dir)/map_to_c > $*.c < $<
	chmod -w $*.c

$(build_obj_dir)/gensplash: $(build_obj_dir_stamp)
$(build_obj_dir)/gensplash: gensplash.c
	$(BUILD_GCC) -g -O1 $(GEN_CFLAGS) -o $(build_obj_dir)/gensplash  $<

# automatically generated c files; used make.depend depends on them
# since they are inlcluded by other executor source files
EXECUTOR_GEN_C = seedtables.c ultable.c sspairtable.c

ctlArrows.o:						\
  arrow_up_active.c arrow_up_inactive.c			\
  arrow_down_active.c arrow_down_inactive.c		\
  arrow_right_active.c arrow_right_inactive.c		\
  arrow_left_active.c arrow_left_inactive.c		\
  thumb_horiz.c thumb_vert.c
stdmbdf.o: apple.c
qRegion.o: hintemplate.h
qStdBits.o: expandtables.c
qStdText.o: ultable.c
qIMVI.o: sspairtable.c
windDocdef.o: 					\
  active.c go_away.c grow.c ractive.c zoom.c

awk_src_dir = $(SRC_DIR)

TRAP_SRC = emustubs.c emutrap.c

EXECUTOR_SRC = crc.c emutraptables.c executor.c priv.c parse.y $(TRAP_SRC)
EXECUTOR_OBJ = $(strip $(addsuffix .o,$(basename $(EXECUTOR_SRC))))

$(build_obj_dir)/genfndecls: genfndecls.c
	$(BUILD_CC) $(HOST_CFLAGS) $< -o $(build_obj_dir)/genfndecls

fndecls: $(build_obj_dir)/genfndecls
fndecls: $(ROMLIB_SRC) $(TRAP_SRC)
	$(build_obj_dir)/genfndecls $(addprefix $(SRC_DIR)/, $(ROMLIB_SRC))	  \
                                   $(addprefix $(SRC_DIR)/, $(TRAP_SRC))          \
                                   $(addprefix $(HOST_ARCH_DIR)/, $(HOST_ARCH_SRC))   \
          > fndecls.tmp
	$(util_dir)/move-if-changed.sh fndecls.tmp fndecls

genptocflags_h.c: genptocflags_h.tmpl fndecls
	$(util_dir)/subst.pl @fndecls@:fndecls < $< > genptocflags_h.c
$(build_obj_dir)/genptocflags_h: genptocflags_h.c
	$(BUILD_GCC) $(GEN_CFLAGS) $< -o $(build_obj_dir)/genptocflags_h
ptocflags.h: $(build_obj_dir)/genptocflags_h
	$(build_obj_dir)/genptocflags_h > ptocflags.h

geninterfacelib.c: geninterfacelib.tmpl fndecls
	$(util_dir)/subst.pl @fndecls@:fndecls < $< > geninterfacelib.c
$(build_obj_dir)/geninterfacelib: geninterfacelib.c
	$(BUILD_GCC) $(GEN_CFLAGS) $< -o $(build_obj_dir)/geninterfacelib

newinterfacelib.c:	$(build_obj_dir)/geninterfacelib
	$(build_obj_dir)/geninterfacelib > newinterfacelib.c

genctopflags_h.c: genctopflags_h.tmpl fndecls
	$(util_dir)/subst.pl @fndecls@:fndecls < $< > genctopflags_h.c
$(build_obj_dir)/genctopflags_h: genctopflags_h.c
	$(BUILD_GCC) $(GEN_CFLAGS) $< -o $(build_obj_dir)/genctopflags_h
ctopflags.h: $(build_obj_dir)/genctopflags_h
	$(build_obj_dir)/genctopflags_h > ctopflags.h

$(build_obj_dir)/genrand_h: genrand_h.c
	$(BUILD_GCC) $(BUILD_CFLAGS) -o $(build_obj_dir)/genrand_h $<

rand.h:	$(build_obj_dir)/genrand_h
	$(build_obj_dir)/genrand_h > rand.h

genstubify_body: $(awk_src_dir)/stubify.awk fndecls ptocflags.h
	echo atrap > genstubify_tmp
	awk 'NF > 1' $(SRC_DIR)/trapinfo				\
	  | sed -e 's%[ 	][ 	]*\([^ 	]\)%,\1%g'		\
	        -e 's%__GetResource%GetResource%' | tr -d ' '		\
	  >> genstubify_tmp
	echo ptoc >> genstubify_tmp
	awk '{ print substr ($$2, 6, length ($$2) - 5), $$3 }' ptocflags.h \
	  | tr ' ' ',' >> genstubify_tmp
	echo body >> genstubify_tmp
	sed -e 's/ *, */,/g' < fndecls >> genstubify_tmp
	awk -f $(awk_src_dir)/stubify.awk genstubify_tmp > genstubify_body
	rm -f genstubify_tmp

genstubify_h.c: genstubify_h.tmpl genstubify_body
	$(util_dir)/subst.pl \
	  @genstubify_body@:genstubify_body < $< > genstubify_h.c

p_q_defines.h:	$(util_dir)/mkstubdefns.pl
	$(util_dir)/mkstubdefns.pl 0 11 > p_q_defines.h

$(build_obj_dir)/genstubify_h: genstubify_h.c p_q_defines.h
	$(BUILD_GCC) $(GEN_CFLAGS) genstubify_h.c -o $(build_obj_dir)/genstubify_h
stubify.h:  $(build_obj_dir)/genstubify_h
	$(build_obj_dir)/genstubify_h > stubify.h

lowglobals.s: $(SRC_DIR)/globals.c $(SRC_DIR)/globals.pl
	rm -f lowglobals.s
	tr -d ' \11' < $(SRC_DIR)/globals.c			\
	  | $(SRC_DIR)/globals.pl @symbol_prefix@ > lowglobals.s
	chmod -w lowglobals.s

licensetext.i: $(SRC_DIR)/licensetext.c
	$(HOST_GCC) $(HOST_CFLAGS) -E -o licensetext.i $<

licensetext.o: licensetext.i
	$(HOST_GCC) $(HOST_CFLAGS) -c -o licensetext.o $<

extr.c:	$(root)/packages/extr_license/extr.c
	ln -s $< extr.c

parse.c: parse.y
	yacc $<
	mv y.tab.c parse.c

executor.o: rand.h executor.c
	$(HOST_GCC) $(HOST_CFLAGS) -DROOT_DIR='"$(root)"' -c -o $*.o $(SRC_DIR)/executor.c

clean::
	rm -f $(EXECUTOR_OBJ) $(ROMLIB_OBJ) $(MAP_C) lowglobals.s	\
		lowglobals.o parse.c rand.h ultable.c seedtables.c	\
		expandtables.c sspairtable.c rawsrcstubs.c rawpatstubs.c \
		licensetext.i extr.c extr licensetext.txt

syserr.o:	include/rsys/version.h
toolevent.o:	include/rsys/version.h

mkvol.o: $(SRC_DIR)/mkvol/mkvol.c $(SRC_DIR)/mkvol/mkvol.h
	$(HOST_GCC) $(HOST_CFLAGS) $(CFLAGS) -c $(SRC_DIR)/mkvol/mkvol.c

dcconvert.o:	dcconvert.c
	$(HOST_GCC) $(HOST_CFLAGS) @egcs_dcconvert_workaround@ -c -o $*.o $(SRC_DIR)/dcconvert.c
