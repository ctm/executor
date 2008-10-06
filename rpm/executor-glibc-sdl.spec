Summary:  The commercial version of Executor, a Macintosh environment for PCs
Name:  executor-glibc-sdl
Version: @executor_version@
Release:  1
License:  Commercial
Group: Applications/Emulators
Vendor: ARDI
Packager: Executor Packager
Requires: executor-aux >= @minimum_executor_aux@
Requires: SDL >= @minimum_sdl@

%description
Executor allows your Linux system to run many Macintosh applications.
Executor includes ARDI's reimplementation of a large percentage of the
core MacOS Classic routines.  This package contains the SDL version of
the Executor.  This package is only available under license from ARDI
and is not to be redistributed.

Documentation is available at http://www.ardi.com
%prep
%build
%install

umask 022
case `hostname` in
  uni52.ardi.com | sybil.ardi.com | newbie.ardi.com | breaker)
    ;;
  *)
    echo This is being run on an inappropriate host
    exit 1;; 
esac

bleeding_prefix=/usr/local/builds/bleeding

if [ ! -e /opt ]; then
  mkdir /opt
fi

if [ ! -e /opt/executor ]; then
  mkdir /opt/executor
fi

if [ ! -e /opt/executor/bin ]; then
  mkdir /opt/executor/bin
fi

cp $bleeding_prefix/comm-linux-sdl/executor /opt/executor/bin/executor-sdl
ln -sf ../../../opt/executor/bin/executor-sdl /usr/bin/executor-sdl
strip /opt/executor/bin/executor-sdl
custom /opt/executor/bin/executor-sdl /home/ctm/ardi/trunk/executor/packages/customize/executor_comm_linux.custom
chown root.root /opt/executor/bin/executor-sdl /usr/bin/executor-sdl
chmod 755 /opt/executor/bin/executor-sdl
%files
/opt/executor/bin/executor-sdl
/usr/bin/executor-sdl
%post
/opt/executor/bin/executor-sdl
