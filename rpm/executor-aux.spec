Summary:  Auxiliary files for Executor, a Macintosh environment for PCs
Name:  executor-aux
Version: @executor_version@
Release:  1
License:  Commercial
Group: Applications/Emulators
Vendor: ARDI
Packager: Executor Packager <questions@ardi.com>

%description
Executor allows your Linux system to run many Macintosh applications.
Executor includes ARDI's reimplementation of a large percentage of the
core MacOS Classic routines.  This package contains the auxiliary
files that Executor needs.

%prep
%build
%install

umask 022
if [ ! -e executor-aux ]; then
  mkdir executor-aux
fi

cd executor-aux
cp -af /home/ctm/ardi/trunk/executor/docs/README.linux README
cp -af /home/ctm/ardi/trunk/executor/docs/AppNotes app_notes
chown -R root.root .

rm -rf /var/opt/executor /home/executor /opt/executor /etc/opt/executor

if [ ! -e /var/opt ]; then
  mkdir /var/opt
  chown root.root /var/opt
fi

if [ ! -e /opt ]; then
  mkdir /opt
  chown root.root /opt
fi

if [ ! -e /etc/opt ]; then
  mkdir /etc/opt
  chown root.root /etc/opt
fi

cp -a /home/ctm/ardi/trunk/executor/var_opt /var/opt/executor
chown -R root.root /var/opt/executor

ln -s ../var/opt/executor/share/home /home/executor
chown root.root /home/executor

cp -a /home/ctm/ardi/trunk/executor/opt /opt/executor
chown -R root.root /opt/executor

find /var/opt/executor /opt/executor -name '.svn' -print0 | xargs -0 rm -rf

mkdir /etc/opt/executor
chown root.root /etc/opt/executor
chmod u+t,a+rwx /etc/opt/executor

ln -sf ../../../../opt/executor/man/man1/executor.1 /usr/man/man1
ln -sf executor.1 /usr/man/man1/executor-demo-svga.1
ln -sf executor.1 /usr/man/man1/executor-demo.1
ln -sf executor.1 /usr/man/man1/executor-svga.1
ln -sf ../../../../opt/executor/man/man5/ecf.5 /usr/man/man5
ln -sf ../../../../opt/executor/man/man5/printers.ini.5 /usr/man/man5
ln -sf printers.ini.5 /usr/man/man5/printdef.ini.5
ln -sf ../../../../opt/executor/man/man5/hfv.5 /usr/man/man5
ln -sf hfv.5 /usr/man/man5/exsystem.hfv.5
ln -sf ../../../../opt/executor/man/man5/AppleDouble.5 /usr/man/man5
ln -sf ../../../../opt/executor/man/man5/directory_map.5 /usr/man/man5
ln -sf ../../../../opt/executor/man/man7/ExecutorVolume.7 /usr/man/man7
ln -sf ExecutorVolume.7 /usr/man/man7/executor.7

chown root.root /usr/man/man1/executor{,-demo-svga,-demo,-svga}.1
chown root.root /usr/man/man5/{ecf,printers.ini,printdef.ini,hfv,AppleDouble,directory_map}.5
chown root.root /usr/man/man7/{ExecutorVolume,executor}.7

pushd /var/opt/executor > /dev/null
chmod -R go+w share/conf share/home/*"System Folder"
chmod go-w share/home/"System Folder"/*{Browser,Printer}
chmod go+w directory_map* printdef.ini share/home/*ware share/home share/home/Shareware/*Tex-Edit share/home/Shareware/Tex-Edit/*"Tex-Edit Prefs" share/home/Shareware/"speedometer3.23 Folder"
popd > /dev/null


%post

# the following chmods are necessary because the permissions get messed up
# when we convert our .rpm to a .deb using alien

pushd /var/opt/executor > /dev/null
chmod -R go+w share/conf share/home/*"System Folder"
chmod go-w share/home/"System Folder"/*{Browser,Printer}
chmod go+w directory_map* printdef.ini share/home/*ware share/home share/home/Shareware/*Tex-Edit share/home/Shareware/Tex-Edit/*"Tex-Edit Prefs" share/home/Shareware/"speedometer3.23 Folder"
popd > /dev/null

%preun

if [ -e /etc/opt/executor/.xp ]; then
  mv /etc/opt/executor/.xp /etc/.temporary_deleteme_xp
fi
%postun
if [ -e /etc/.temporary_deleteme_xp ]; then
  if [ ! -e /etc/opt ]; then
    mkdir /etc/opt
  fi
  if [ ! -e /etc/opt/executor ]; then
    mkdir /etc/opt/executor
  fi
  mv /etc/.temporary_deleteme_xp /etc/opt/executor/.xp
fi

%files

%doc /opt/executor/man
%doc /usr/man/man1/executor.1
%doc /usr/man/man1/executor-demo-svga.1
%doc /usr/man/man1/executor-demo.1
%doc /usr/man/man1/executor-svga.1
%doc /usr/man/man5/ecf.5
%doc /usr/man/man5/printers.ini.5
%doc /usr/man/man5/printdef.ini.5
%doc /usr/man/man5/hfv.5
%doc /usr/man/man5/exsystem.hfv.5
%doc /usr/man/man5/AppleDouble.5
%doc /usr/man/man5/directory_map.5
%doc /usr/man/man7/ExecutorVolume.7
%doc /usr/man/man7/executor.7

%doc executor-aux/README
%doc executor-aux/app_notes

/opt/executor/printers.ini
/opt/executor/tips.txt
/var/opt/executor
/etc/opt/executor
/home/executor
