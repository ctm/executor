#!/bin/sh

# usage: ./configure.sh [options]
# options:
# --cflags='flags'		(default "-g")
# --host='host'		(no default)
# --host-file-format='format	(no default)
# --build='build'			(no default)
# --host-gcc='gcc'		(default "gcc", if build and host are same)
# --host-strip='strip'	(default "strip")
# --host-nm='nm'		(default "nm")
# --root='dir'			(default "..")
# --front-end='front-end'
#    one of `x', `nextstep', `dos', `svgalib', `win32', 'sdl'	(no default)
# --syn68k-host='host'
# --sound='sound'

cflags='-g'
root='..'

# save away arguments to be used when creating the `config.status'
arguments=''

while [ $# != 0 ]; do
  arg="$1"; shift

  # append the next argument onto the argument list for `config.status'
  arguments="$arguments '$arg'"

  case "${arg}" in
    # options
    -*=*)
      opt=`echo ${arg} | sed 's:^-*\([^=]*\)=.*$:\1:'`
      val=`echo ${arg} | sed 's:^-*[^=]*=\(.*\)$:\1:'`
      
      ## Change `-' in the option name to `_'.
      optname="${opt}"
      opt="`echo ${opt} | tr - _`"

      case "${opt}" in
        "cflags")
          cflags="${val}"
        ;;
        "root")
          root="${val}"
        ;;
        "host")
          host="${val}"
        ;;
        "host_gcc")
          host_gcc="${val}"
	;;
        "host_strip")
          host_strip="${val}"
        ;;
        "host_nm")
          host_nm="${val}"
        ;;
        "build")
          build="${val}"
        ;;
        "front_end")
	  front_end="${val}"
	;;
	"host_file_format")
	  host_file_format="${val}"
	;;
	"syn68k_host")
	  syn68k_host="${val}"
	  ;;
	"sound")
	  sound="${val}"
	  ;;
	*)
	  echo "unknown option \`${arg}', ignored"
	;;
      esac
    ;;
    *)
      echo "unknown option \`${arg}', ignored"
    ;;
  esac
done

util_dir=${root}/util

if [ "${host}" = "" ]; then
  echo "Fatal error: you must specify a host.  Exiting."
  exit 1
fi

if [ "${front_end}" = "" ]; then
  echo "Fatal error: you must specify a front end.  Exiting."
  exit 1
fi  

if [ "${build}" = "" ]; then
  echo "You did not specify a build.  Taking a guess."
  build=`${root}/util/config.guess`

  if [ "${build}" = "" ]; then
    echo "config.guess failed to determine the build type.  Exiting."
    exit 1
  else
    echo "This appears to be a \`${build}'."
  fi
fi

case "${front_end}" in
  x | nextstep | dos | svgalib | win32 | sdl)
    ;;
  *)
    echo "Fatal error: unknown front end \`${front_end}'.  Exiting."
    exit 1
  ;;
esac

case "${sound}" in
  dummy | djgpp | linux | sdl)
    ;;
  *)
    echo "Fatal error: unknown sound \`${sound}'.  Exiting."
    exit 1
    ;;
esac

# canonicalize the name of the host; this should
# give us the name used for tool configuration
if canonical_host=`${root}/util/config.sub "${host}"` ; then : ; else
  exit $?
fi

# canonicalize the name of the build; this should
# give us the name used for tool configuration
if canonical_build=`${root}/util/config.sub "${build}"` ; then : ; else
  exit $?
fi

if [ "${host_gcc}" = "" ]; then
  if [ "${canonical_host}" = "${canonical_build}" ]; then
    host_gcc=gcc
  else
    echo "Fatal error: host and build differ, host gcc must by specified.  Exiting."
    exit 1
  fi
fi

if [ "${host_nm}" = "" ]; then
  host_nm=nm
fi

if [ "${host_strip}" = "" ]; then
  host_strip=strip
fi

case ${canonical_host} in
  m68k-next-ns* | m68k-next-bsd* | m68k-next-mach* | m68k-next-nextstep*)
    host_os='next'
    host_syn68k='next'
    host_arch='m68k'
    host_file_format='mach-o'
    objc='yes'
  ;;
  i[3456]86-next-ns* | i[3456]86-next-bsd* | i[3456]86-next-mach* \
	| i[3456]86-next-nextstep*)
    host_os='next'
    host_syn68k='next'
    host_arch='i386'
    host_file_format='mach-o'
    objc='yes'
  ;;
  i[3456]86-msdos-go32 | i[3456]86-go32-bsd | i[3456]86-unknown-msdos)
    host_os='msdos'
    host_syn68k='msdos'
    host_file_format='coff'
    host_arch='i386'
  ;;

# NOTE: Historically we've used mingw32 but called it cygwin.  The reason
# we use mingw32 is because it has a license compatible with Executor.
# However, since internally we've called it cygwin, I'm hesitant to change
# cygwin to mingw32 everywhere right now (20031217).

  i[3456]86-pc-mingw32)
    host_os='cygwin32'
    host_syn68k='mingw32'
    echo host os set to cygwin32 which is a misnomer.  this really is mingw32
    host_file_format='pe'
    host_arch='i386'
  ;;
  i[3456]86-pc-cygwin32)
    host_os='cygwin32'
    host_syn68k='cygwin32'
    host_file_format='pe'
    host_arch='i386'
  ;;
  alpha-unknown-linux)
    host_os='linux'
    host_syn68k='linux'
    host_file_format='elf'
    host_arch='alpha'
  ;;
  powerpc-unknown-linux)
    host_os='linux'
    host_syn68k='linux'
    host_file_format='elf'
    host_arch='powerpc'
  ;;
  i[3456]86-unknown-linux)
    host_os='linux'
    host_syn68k='linux'
    host_arch='i386'
    if [ x"${host_file_format}" = x"" ]; then
      # default linux file format; this may change
      host_file_format='a.out'
    fi
  ;;
  i[456]86-unknown-macosx)
    host_os='macosx'
    host_syn68k='macosx'
    host_arch='i386'
    if [ x"${host_file_format}" = x"" ]; then
      # default linux file format; this may change
      host_file_format='mach-o'
    fi
  ;;

  powerpc-unknown-macosx)
    host_os='macosx'
    host_syn68k='macosx'
    host_arch='powerpc' # how we refer to it
    host_gcc_arch='ppc' # how gcc -arch wants us to call it
    if [ x"${host_file_format}" = x"" ]; then
      # default linux file format; this may change
      host_file_format='mach-o'
    fi
  ;;

  *)
    echo "Fatal error: unknown host \`${canonical_host}'.  Exiting."
    exit 1
  ;;
esac

case ${canonical_build} in
  m68k-next-ns* | m68k-next-bsd* | m68k-next-mach* \
   | m68k-next-nextstep*)
    build_os='next'
    build_arch='m68k'
    objc='yes'
  ;;
  i[3456]86-next-ns* | i[3456]86-next-bsd* | i[3456]86-next-mach* \
   | i[3456]86-next-nextstep*)
    build_os='next'
    build_arch='i386'
    objc='yes'
  ;;
  i[3456]86-msdos-go32 | i[3456]86-go32-bsd | i[3456]86-unknown-msdos)
    build_os='msdos'
    build_arch='i386'
  ;;
  alpha-unknown-linux)
    build_os='linux'
    build_arch='alpha'
  ;;
  powerpc-unknown-linux)
    build_os='linux'
    build_arch='powerpc'
  ;;
  i[3456]86-unknown-linux)
    build_os='linux'
    build_arch='i386'
  ;;
  i[456]86-unknown-macosx)
    build_os='macosx'
    build_arch='i386'
    objc='yes'
  ;;

  powerpc-unknown-macosx)
    build_os='macosx'
    build_arch='powerpc'
    objc='yes'
  ;;

  *)
    echo "Fatal error: unknown build \`${canonical_build}'.  Exiting."
    exit 1
  ;;
esac

# check for the host directories
host_arch_dir=${root}/src/config/arch/${host_arch}
if [ ! -d ${host_arch_dir} ]; then
  echo "Fatal error: host arch directory \`${host_arch_dir}' not found.  Exiting."
  exit 1
fi

host_os_dir=${root}/src/config/os/${host_os}
if [ ! -d ${host_os_dir} ]; then
  echo "Fatal error: host os directory \`${host_os_dir}' not found.  Exiting."
  exit 1
fi

if [ -r ${root}/src/config/os/${host_os}/${host_os}.make ]; then

  if [ -r ${root}/src/config/os/${host_os}/${host_os}.sh ]; then
    ${root}/src/config/os/${host_os}/${host_os}.sh		\
     "${host_gcc}"						\
     "${cflags}"						\
     ${root}/src/config/os/${host_os}/${host_os}.make	\
     __config__.host_os.make
    
    if [ $? != "0" ]; then
      echo "Fatal error: host os configuration failed.  Exiting."
      exit 1
    fi
    host_os_make=__config__.host_os.make
  else
    host_os_make=${root}/src/config/os/${host_os}/${host_os}.make
  fi
else
  host_os_make="/dev/null"
fi

if [ -r ${root}/src/config/arch/${host_arch}/${host_arch}.make ]; then
   host_arch_make=${root}/src/config/arch/${host_arch}/${host_arch}.make
else
  host_arch_make="/dev/null"
fi

if [ -r ${root}/src/config/front-ends/${front_end}/${front_end}.sh ]; then
  # generate the front end makefile front the front end template
  # make file `${front_end}.make'
  
  # there must be a front-end makefile fragment
  ${root}/src/config/front-ends/${front_end}/${front_end}.sh	\
   "${host_gcc}"							\
   "${cflags}"								\
   ${root}/src/config/front-ends/${front_end}/${front_end}.make		\
   __config__.front_end.make
  
  if [ $? != "0" ]; then
    echo "Fatal error: front end configuration failed.  Exiting"
    exit 1
  fi
  
  front_end_make=__config__.front_end.make
else
  # there must be a front-end makefile fragment
  front_end_make=${root}/src/config/front-ends/${front_end}/${front_end}.make
fi

# there must be a front-end config header
front_end_config_h=${root}/src/config/front-ends/${front_end}/${front_end}.h

if [ -r ${root}/src/config/sound/${sound}/${sound}.make ]; then
  if [ -r ${root}/src/config/sound/${sound}/${sound}.sh ]; then
    ${root}/src/config/sound/${sound}/${sound}.sh			\
     "${host_gcc}"							\
     "${cflags}"							\
     ${root}/src/config/sound/${sound}/${sound}.make			\
     __config__.sound.make
    
    if [ $? != "0" ]; then
      echo "Fatal error: host os configuration failed.  Exiting."
      exit 1
    fi
    sound_make=__config__.sound.make
  else
    sound_make=${root}/src/config/sound/${sound}/${sound}.make
  fi
else
  sound_make="/dev/null"
fi

executor_make=${root}/src/executor.make

# link `front-end-config.h' to the front end config header
rm -f front-end-config.h
ln -s ${front_end_config_h} front-end-config.h

# arch determines syn68k usage
rm -f host-arch-config.h
if [ -r ${root}/src/config/arch/${host_arch}/${host_arch}.h ]; then
  host_arch_h=${root}/src/config/arch/${host_arch}/${host_arch}.h

  ln -s ${host_arch_h} host-arch-config.h

  cat > ./test.c.sed << __EOF__
#include <stdio.h>	
#include "@host_arch_h@"

int main ()
{
  printf (
#if defined (SYN68K)
	  "yes"
#else
	  ""
#endif /* SYN68K */
	  );
}
__EOF__

  sed -e "s:@host_arch_h@:${host_arch_h}:" \
    < ./test.c.sed > ./test.c

  gcc -I${root}/src/include -o ./test ./test.c
  if [ x"`./test`" = x"" ]; then
    syn68k=''
  else
    syn68k='yes'
  fi

  rm -f ./test.c.sed ./test.c ./test
else
  # create an empty `host-arch-config.h'
  touch host-arch-config.h
  
  host_conf_h=""
  syn68k=''
fi

rm -f host-os-config.h
if [ -r ${root}/src/config/os/${host_os}/${host_os}.h ]; then
  host_os_h=${root}/src/config/os/${host_os}/${host_os}.h

  ln -s ${host_os_h} host-os-config.h
else
  # create an empty `host-os-config.h'
  touch host-os-config.h
  host_os_h=""
fi

rm -f build-arch-config.h
if [ -r ${root}/src/config/arch/${build_arch}/${build_arch}.h ]; then
  build_arch_h=${root}/src/config/arch/${build_arch}/${build_arch}.h

  ln -s ${build_arch_h} build-arch-config.h
else
  # create an empty `build-arch-config.h'
  touch build-arch-config.h
  build_arch_h=""
fi

rm -f build-os-config.h
if [ -r ${root}/src/config/os/${build_os}/${build_os}.h ]; then
  build_os_h=${root}/src/config/os/${build_os}/${build_os}.h

  ln -s ${build_os_h} build-os-config.h
else
  # create an empty `build-os-config.h'
  touch build-os-config.h
  build_os_h=""
fi

rm -f sound-config.h
if [ -r ${root}/src/config/sound/${sound}/${sound}-sound.h ]; then
  sound_h=${root}/src/config/sound/${sound}/${sound}-sound.h

  ln -s ${sound_h} sound-config.h
else
  # create an empty `sound-config.h'
  touch sound-config.h
  sound_h=""
fi

if [ x"${syn68k}" = x"yes" ]; then
#  syn68k_define='-DSYN68K'
  syn68k_define=''
  syn68k_lib='libsyn68k.a'
  if [ x$syn68k_host = x"" ]; then
    syn68k_host="${host_arch}-${host_syn68k}-${host_file_format}"
  fi
else
  syn68k_define=''
  syn68k_lib=''
  syn68k_host=''
fi

cat > ./test.c << __EOF__
int foo;
__EOF__

${host_gcc} -c test.c
symbol=`${host_nm} test.o | awk '/foo/ { print $3; }'`
case ${symbol} in
  _foo)
	symbol_prefix='_'
	;;
  foo)
       symbol_prefix=''
       ;;
  *)
     echo "Error: unknown asm symbol \`${symbol}', exiting."
     exit 1
esac

rm -f ./test.c ./test.o

if ${host_gcc} --version | egrep -q 'egcs-2\.91'; then
  egcs_dcconvert_workaround=-fno-omit-frame-pointer
else
  egcs_dcconvert_workaround=
fi

${util_dir}/subst.pl				\
 @host_arch_make@:${host_arch_make} 	\
 @host_os_make@:${host_os_make} 		\
 @front_end_make@:${front_end_make}		\
 @sound_make@:${sound_make}			\
 @executor_make@:${executor_make} < ${root}/src/config/Makefile.in > ./tmp-Makefile.in

if [ x"${host_gcc_arch}" = x"" ]; then
  host_gcc_arch="$host_arch"
fi

sed -e "s:@symbol_prefix@:${symbol_prefix}:g

        s:@host@:${host}:g
        s:@canonical_host@:${canonical_host}:g
        s:@host_arch@:${host_arch}:g
        s:@host_os@:${host_os}:g
        s:@host_gcc@:${host_gcc}:g
        s:@host_strip@:${host_strip}:g
	s:@host_file_format@:${host_file_format}:g

        s:@build@:${build}:g
        s:@canonical_build@:${canonical_build}:g
        s:@build_arch@:${build_arch}:g
        s:@build_os@:${build_os}:g

        s:@front_end@:${front_end}:g
	s:@sound@:${sound}:g
        s:@front_end_make@:${front_end_make}:g
        s:@root@:${root}:g
        s:@syn68k_define@:${syn68k_define}:g
        s:@syn68k_lib@:${syn68k_lib}:g
        s:@syn68k_host@:${syn68k_host}:g
        s:@cflags@:${cflags}:g
	s:@egcs_dcconvert_workaround@:${egcs_dcconvert_workaround}:g
        s:@arch@:${host_gcc_arch}:g" < ./tmp-Makefile.in > ./Makefile
rm -f ./tmp-Makefile.in

if [ x"${syn68k}" = x"yes" ]; then
  sed -e "/^ifnosyn68k$/,/^end ifnosyn68k$/d" \
      -e "/^ifsyn68k$/d" \
      -e "/^end ifsyn68k$/d" < ./Makefile > ./tmp-Makefile
else
  sed -e "/^ifsyn68k$/,/^end ifsyn68k$/d" \
      -e "/^ifnosyn68k$/d" \
      -e "/^end ifnosyn68k$/d" < ./Makefile > ./tmp-Makefile
fi
rm -f ./Makefile
mv ./tmp-Makefile ./Makefile

if [ x"${objc}" = x"yes" ]; then
  sed -e "/^ifobjc$/d" \
      -e "/^end ifobjc$/d" < ./Makefile > ./tmp-Makefile
else
  sed -e "/^ifobjc$/,/^end ifobjc$/d" < ./Makefile > ./tmp-Makefile
fi
rm -f ./Makefile
mv ./tmp-Makefile ./Makefile

# cleanup
rm -f __config__.*

# create a config.status
mv config.status config.status.running
rm -rf config.status.running

echo '#!/bin/sh'                             > config.status
echo                                        >> config.status
echo "${root}/util/configure.sh $arguments" >> config.status

chmod +x config.status

# If we're using this old configure script, then we're probably not dealing
# with 64-bit machines.  The new build system constructs config.h for us.

echo '#define SIZEOF_CHAR_P 4' > config.h

echo "Executor is now configured for \`${host_arch}-${host_os}/${front_end}'."
