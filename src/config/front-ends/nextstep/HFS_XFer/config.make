# Begin NeXT
MACHFLAGS=-DSUN -DNEXT -DNEXTSTEP
GCCMFLAGS=-m68881
PCCMFLAGS=-m68881
CPP=/lib/cpp
NEXTLIB=$(ROMLIB)/next

EVENTLIB=     -sectcreate __ICON __header $(NEXTLIB)/MacStuff.iconheader \
	      -segprot __ICON r r					 \
	      -sectcreate __ICON app /usr/lib/nib/default_app_icon.tiff	 \
	      -sectcreate __NIB Info.nib $(NEXTLIB)/Info.nib		 \
	      -sectcreate __NIB MacStuff.nib $(NEXTLIB)/MacStuff.nib	 \
	      -segprot __NIB r r -lNeXT_s -lsys_s

RANLIB=ranlib -c
ALLOCAOFILE=

#.c.o:
#	$(CC) $(CPPFLAGS) -E $< | $(ROMLIB)/intercep $(CPPFLAGS) > $*.tmp.c
#	$(CC) $(CFLAGS) -c $*.tmp.c
#	mv $*.tmp.o $*.o
#	rm $*.tmp.c

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -B$(ROMLIB)/gnu/ -c -o $*.o  $<

# Use this .c.o if your cc won't accept the line control information
# produced by cpp
#.c.o:
#	$(CPP) $(CPPFLAGS) -C -P $< | $(ROMLIB)/unsharp | \
#				      $(ROMLIB)/intercep $(CPPFLAGS) > $*.tmp.c
#	$(CC) $(CFLAGS) -c $*.tmp.c
#	mv $*.tmp.o $*.o

.SUFFIXES: .c .Oo .pgo .ao

# Note in the following rule, $(GCC) should probably include -tradional-strings
# Use one of the sample Makefiles from billdemo, mini, or vp as a template

.c.Oo:
	$(CC) $(CPPFLAGS) $(OFLAGS) -B$(ROMLIB)/gnu/ -c -o $*.Oo  $<

# Use this .c.Oo if you're not using gcc
#.c.Oo:
#	$(CPP) $(CPPFLAGS) -C -P $< | $(ROMLIB)/unsharp | \
#				      $(ROMLIB)/intercep $(CPPFLAGS) > $*.tmp.c
#	$(CC) -O -c $*.tmp.c
#	mv $*.tmp.o $*.Oo

.c.pgo:
	$(CC) $(CPPFLAGS) $(PGFLAGS) -B$(ROMLIB)/gnu/ -c -o $*.pgo  $<

# Use this .c.pgo if you're not using gcc
#.c.pgo:
#	$(CPP) $(CPPFLAGS) -C -P $< | $(ROMLIB)/unsharp | \
#				      $(ROMLIB)/intercep $(CPPFLAGS) > $*.tmp.c
#	$(CC) -pg -c $*.tmp.c
#	mv $*.tmp.o $*.pgo

# End NeXT
