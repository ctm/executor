Summary:  A demo version of Executor, a Macintosh environment for PCs
Name:  executor-glibc-demo-sdl
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
core MacOS Classic routines.  This demo version of Executor can be
used for 30 days.  This package contains the SDL version of the
Executor demo for glibc based Linux distributions.

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

cp $bleeding_prefix/demo-linux-sdl/executor /opt/executor/bin/executor-demo-sdl
ln -sf ../../../opt/executor/bin/executor-demo-sdl /usr/bin/executor-demo-sdl
strip /opt/executor/bin/executor-demo-sdl
custom /opt/executor/bin/executor-demo-sdl /home/ctm/ardi/trunk/executor/packages/customize/executor_demo_linux.custom
chown root.root /opt/executor/bin/executor-demo-sdl /usr/bin/executor-demo-sdl
chmod 755 /opt/executor/bin/executor-demo-sdl
%files
/opt/executor/bin/executor-demo-sdl
/usr/bin/executor-demo-sdl
