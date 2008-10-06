Summary:  The commercial version of Executor, a Macintosh environment for PCs
Name:  executor-glibc-svga
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
core MacOS Classic routines.  This package contains the SVGAlib
version of Executor for glibc based Linux distributions.  This package
is only available under license from ARDI and is not to be
redistributed.

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

cp /usr/local/builds/bleeding/comm-linux-svga/executor /opt/executor/bin/executor-svga
ln -sf ../../../opt/executor/bin/executor-svga /usr/bin/executor-svga
strip /opt/executor/bin/executor-svga
custom /opt/executor/bin/executor-svga /home/ctm/ardi/trunk/executor/packages/customize/executor_comm_linux.custom
chown root.root /opt/executor/bin/executor-svga /usr/bin/executor-svga
chmod 4755 /opt/executor/bin/executor-svga
%files
/opt/executor/bin/executor-svga
/usr/bin/executor-svga
%post
if [ "$TERM"x = "linuxx" ]; then
  /opt/executor/bin/executor-svga
  chown root.root /opt/executor/bin/executor-svga
  chmod 4755 /opt/executor/bin/executor-svga
else
  echo You must run /opt/executor/bin/executor-svga once as root so that
  echo your serial number and authorization key can be entered.
fi
