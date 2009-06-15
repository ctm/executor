## Process this file with automake to produce Makefile.in

## $Id: Makefile.am 63 2004-12-24 18:19:43Z ctm $

bin_PROGRAMS = executor

executor_SOURCE = \
  ctlArrows.c   ctlDisplay.c  ctlIMIV.c     ctlInit.c     ctlMisc.c	\
  ctlMouse.c    ctlSet.c      ctlSize.c     ctlStddef.c   ctlPopup.c \
  dialAlert.c   dialCreate.c  dialHandle.c  dialInit.c    dialManip.c	\
  dialDispatch.c dialItem.c \
  fileAccess.c   fileCreate.c   fileDirs.c     fileDouble.c   fileInfo.c \
  fileMisc.c     fileVolumes.c	fileHighlevel.c dcache.c \
  hfsBtree.c       hfsChanging.c    hfsCreate.c      hfsFile.c		\
  hfsHelper.c      hfsHier.c        hfsMisc.c        hfsVolume.c	\
  hfsWorkingdir.c  hfsXbar.c \
  listAccess.c   listAddDel.c   listCreate.c   listDisplay.c  listMouse.c \
  listOps.c      listStdLDEF.c \
  menu.c       menuColor.c  menuV.c stdmdef.c stdmbdf.c \
  PSprint.c     PSstrings.c   prError.c     prInit.c      prLowLevel.c	\
  prPrinting.c  prRecords.c \
  qBit.c                        qCConv.c       qCGrafPort.c   qCRegular.c   \
  qColor.c       qColorMgr.c    qColorutil.c   qCursor.c      qGrafport.c   \
  qIMIV.c        qIMV.c         qIMVxfer.c     qMisc.c        qPaletteMgr.c \
  qPen.c         qPicstuff.c    qPicture.c     qPixMapConv.c  qPoint.c	    \
  qPoly.c        qRect.c        qRegion.c      qRegular.c     qScale.c	    \
  qStandard.c    qStdArc.c      qStdBits.c     qStdLine.c     qStdOval.c    \
  qStdPic.c      qStdPoly.c     qStdRRect.c    qStdRect.c     qStdRgn.c	    \
  qStdText.c     qText.c	qGWorld.c      qGDevice.c     qIMVI.c	    \
  qHooks.c	 xdata.c	xdblt.c	       rawpatblt.c    rawsrcblt.c   \
  dirtyrect.c    srcblt.c	qColorPicker.c qPict2.c \
  resGet.c      resGetinfo.c  resGettype.c  resIMIV.c     resInit.c	\
  resMisc.c     resMod.c      resOpen.c     resSetcur.c	  resPartial.c \
  teAccess.c   teDisplay.c  teEdit.c     teIMIV.c     teIMV.c      teInit.c \
  teInsert.c   teMisc.c     teScrap.c \
  windColor.c    windDisplay.c  windDocdef.c   windInit.c     windMisc.c \
  windMouse.c    windSize.c     windUpdate.c \
  AE.c AE_desc.c AE_hdlr.c AE_coercion.c \
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
  lockrange.c

AM_CPPFLAGS=-I$(srcdir)/include
