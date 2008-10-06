# The use of dlltool below means that you need to have a dlltool that
# will work with your currently existing path.  The best solution is
# probably to make a /usr/local/bin/dlltool that is a shell script that
# adjusts the path and then calls dlltool proper.

DLLTOOL = dlltool

LIBS = libkernel32.a libuser32.a libgdi32.a libcomdlg32.a libwinmm.a \
	libhfv32.a # libadvapi32.a not needed

LIBDIR = /usr/local/i486-pc-cygwin32/lib

all: $(LIBS)

%.def: %.rawdef header.rawdef
	sort $< | cat header.rawdef - > $@

lib%.a: %.def
	$(DLLTOOL) --dllname `basename $< .def`.dll --def $< --output-lib $@ -k

install:	$(LIBS)
	cp -a $(LIBS) $(LIBDIR)

clean:
	rm -f $(LIBS) *.def
