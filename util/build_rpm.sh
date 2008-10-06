#! /bin/bash

# $Id: build_rpm.sh 95 2005-06-16 18:42:13Z ctm $

set -o pipefail 2> /dev/null || true
set -o errexit -o nounset -o noclobber

# This shell script creates the appropriate .spec files for any versions
# of Executor that are compiled on this machine.  It then runs rpm -ba
# and copies the resultant binary RPMs to the right place so that we can
# build an ISO with the RPMs

# The glibc in the package names are superfluous now that we no longer
# support libc5.  The X windows version doesn't have the "-x" suffix because
# it used to be the canonical version.  Eventually we will make the SDL
# version the canonical version.

function fill_template ()
{
  local -r executor_version=$2
  local -r minimum_sdl=$3

  sed -e "s/@executor_version@/$executor_version/" \
      -e "s/@minimum_executor_aux@/2.1pr12/" \
      -e "s/@minimum_sdl@/$minimum_sdl/" \
       < /home/ctm/ardi/trunk/executor/rpm/$1 >| /tmp/$1
}

cd /usr/local/builds/bleeding

if [ x`hostname` = xuni52.ardi.com ]; then
  PATH="/usr/local/bin:$PATH"
fi

build_aux=0
sdl=`rpm -q SDL`
minimum_sdl=`expr $sdl : 'SDL-\([^.]*\.[^.]*\).*'`.0
for build in *; do
  os=`echo $build | awk -F- '{print $2}'`
  if [ x$os = xlinux ]; then
    class=`echo $build | awk -F- '{if ($1 ~ /demo/) print "-demo"}'`
    frontend=`echo $build | awk -F- '{if ($3 !~ /x/) print "-" $3}'`
    spec_file=executor-glibc$class$frontend.spec
    executor_version=$($build/executor -version)
    fill_template $spec_file  $executor_version $minimum_sdl
    sudo rpmbuild -ba /tmp/$spec_file || ( echo failure ; kill -TERM 0 )
    build_aux=1
  fi
done
if [ $build_aux != 0 ]; then
  fill_template executor-aux.spec $executor_version $minimum_sdl
  sudo rpmbuild -ba /tmp/executor-aux.spec || ( echo failure ; kill -TERM 0 )
fi
