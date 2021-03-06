Summary:  A demo version of Executor, a Macintosh environment for PCs
Name:  executor-glibc-demo-svga
Version: @executor_version@
Release:  1
License:  Commercial
Group: Applications/Emulators
Vendor: ARDI
Packager: Executor Packager
Requires: executor-aux >= @minimum_executor_aux@

%description
Executor allows your Linux system to run many Macintosh applications.
Executor includes ARDI's reimplementation of a large percentage of the
core MacOS Classic routines.  This demo version of Executor can be
used for 30 days.  This package contains the SVGAlib version of the
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

if [ ! -e /opt ]; then
  mkdir /opt
fi

if [ ! -e /opt/executor ]; then
  mkdir /opt/executor
fi

if [ ! -e /opt/executor/bin ]; then
  mkdir /opt/executor/bin
fi

cp /usr/local/builds/bleeding/demo-linux-svga/executor /opt/executor/bin/executor-demo-svga
ln -sf ../../../opt/executor/bin/executor-demo-svga /usr/bin/executor-demo-svga
strip /opt/executor/bin/executor-demo-svga
custom /opt/executor/bin/executor-demo-svga /home/ctm/ardi/trunk/executor/packages/customize/executor_demo_linux.custom
chown root.root /opt/executor/bin/executor-demo-svga /usr/bin/executor-demo-svga
chmod 4755 /opt/executor/bin/executor-demo-svga
%files
/opt/executor/bin/executor-demo-svga
/usr/bin/executor-demo-svga
