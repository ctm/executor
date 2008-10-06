Summary:  The commercial version of Executor, a Macintosh environment for PCs
Name:  executor-glibc
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
core MacOS Classic routines.  This package contains the X version of
Executor for glibc based Linux distributions.  This package is only
available under license from ARDI and is not to be redistributed.

Documentation is available at http://www.ardi.com
%prep
%build

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
  chown root.root /opt
fi

if [ ! -e /opt/executor ]; then
  mkdir /opt/executor
  chown root.root /opt/executor
fi

if [ ! -e /opt/executor/bin ]; then
  mkdir /opt/executor/bin
  chown root.root /opt/executor/bin
fi

cp /usr/local/builds/bleeding/comm-linux-x/executor /opt/executor/bin
chown root.root /opt/executor/bin/executor
ln -sf ../../../opt/executor/bin/executor /usr/bin/executor
chown root.root /usr/bin/executor
strip /opt/executor/bin/executor
custom /opt/executor/bin/executor /home/ctm/ardi/trunk/executor/packages/customize/executor_comm_linux.custom
chmod 755 /opt/executor/bin/executor
%files
/opt/executor/bin/executor
/usr/bin/executor
%post
/opt/executor/bin/executor
