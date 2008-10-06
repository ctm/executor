
TARGET_ARCH_OBJ = i386.o x86patblt.o x86srcblt.o xdstubtables.o sbstubtables.o

pat-blitters-stamp pat-blitters.h pat-blitters.s: \
		opfind.c opfind.h metaasm.pl pat-blitters.meta
	$(TARGET_ARCH_DIR)/metaasm.pl $(METAASM_ARGS)\
		$(TARGET_ARCH_DIR)/pat-blitters.meta\
		pat-blitters.s pat-blitters.h\
		$(TARGET_ARCH_DIR)/opfind.c
	$(RM) opfind
	touch pat-blitters-stamp

# We have src-blitters-stamp depend on pat-blitters-stamp so we don't
# try to do two metaasm's at once.  They would fight over `opfind'.
src-blitters-stamp src-blitters.h src-blitters.s: \
		opfind.c opfind.h metaasm.pl src-blitters.meta src-shift.meta\
		src-noshift.meta src-shift-fgbk.meta src-noshift-fgbk.meta\
		pat-blitters-stamp src-blitters-core.meta
	$(TARGET_ARCH_DIR)/metaasm.pl -define DST_SEG= $(METAASM_ARGS)\
		$(TARGET_ARCH_DIR)/src-blitters.meta\
		src-blitters.s src-blitters.h\
		$(TARGET_ARCH_DIR)/opfind.c
	$(RM) opfind
	touch src-blitters-stamp

x86patblt.o: x86patblt.S pat-blitters-stamp
	$(TARGET_AS_CPP) $(TARGET_CFLAGS) -c $(TARGET_ARCH_DIR)/x86patblt.S

x86srcblt.o: x86srcblt.S src-blitters-stamp
	$(TARGET_AS_CPP) $(TARGET_CFLAGS) -c $(TARGET_ARCH_DIR)/x86srcblt.S

opfind:	opfind.c opfind.h asmsamples.h
	$(HOST_GCC) $(HOST_CFLAGS) $(TARGET_ARCH_DIR)/opfind.c -o opfind
	$(RM) asmsamples.h

xdstubtables.o: xdstubtables.c pat-blitters-stamp

sbstubtables.o: sbstubtables.c src-blitters-stamp

clean::
	rm -f $(TARGET_ARCH_OBJ) pat-blitters.s pat-blitters.h asmsamples.h\
		pat-blitters-stamp src-blitters.s src-blitters.h\
		src-blitters-stamp opfind
