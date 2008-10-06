#!/bin/sh

cc=$1
cflags=$2

src=$3
dst=$4

# ctm: I added || true here so we can compile on Fedora Core 1
#      eventually we'll use autoconf to figure out stuff like this, so
#      hardcoding in -ldb is OK for now (20031215)
if nm /usr/lib/libdb.a 2>/dev/null | grep -q ' T dbm_open$' || true; then
  libdb=-ldb
else
  libdb=-ldb1
fi

# clean up on the way in
rm -f test.c test

case ${cflags} in
  *-g*)
	cat > ./test.c << __EOF__
int main () { }
__EOF__

     ${cc} ${cflags} ./test.c -o ./test -lg >/dev/null 2>&1
     if [ $? = "0" ]; then
       sed -e "s,@libg@,-lg," -e "s,@libdb@,$libdb," < ${src} > ${dst}
     else
       sed -e "s,@libg@,," -e "s,@libdb@,$libdb," < ${src} > ${dst}
     fi
     ;;
  *)
     sed -e "s,@libg@,," -e "s,@libdb@,$libdb," < ${src} > ${dst}
     ;;
esac

# clean up on the way out
rm -f test.c test

exit 0
