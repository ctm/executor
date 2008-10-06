#!/bin/sh

cc=$1
cflags=$2

src=$3
dst=$4

# currently only check the empty libdir and the one given
# below; as we try to build on more machines with more xlibdir
# locations, this list should grow

for xlibdir in '' '-L/usr/X11/lib/i486-linuxaout'	\
                  '-L/usr/X11/lib/i486-linux'		\
                  '-L/usr/X11/lib/'			\
		  '-L/usr/X11R6/lib'			\
		    ; do

  cat > ./test.c << __EOF__

#include <X11/Xlib.h>

int
main ()
{ 
  XOpenDisplay ("");
}
__EOF__

  ${cc} ${cflags} -o ./test ./test.c ${xlibdir} -lX11 >/dev/null 2>&1
  if [ $? = "0" ]; then
    good_xlibdir=${xlibdir}
    have_xlibdir=1
    break
  fi
done

if [ x${have_xlibdir} = x"" ]; then
  echo x front-end: fatal error: unable to determine xlibdir
  exit 1
fi

sed -e "s,@xlibdir@,${good_xlibdir}," < ${src} > ${dst}

exit 0
