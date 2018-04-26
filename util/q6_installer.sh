#! /bin/sh



# This installer installs Quicken 6 from the Quicken 6 Deluxe CD.
#
# This installer itself is ugly, but the end-result illustrates some
# of the ways in which Executor can be configured to run things a bit
# nicer than by just the command line.
#
# It would be better to have a generic app-that-uses-Executor
# installer than to build one that's Quicken specific, but Linux Expo
# is just a few days away.  We'll ship this now and do more later
# after we build Executor enthusiasm in the Linux community and build
# Linux enthusiasm in the Mac-developer community (Carbonless Copies).
#
# This script is designed to work on any distribution, although if
# you're running a Red Hat system, it will make an entry in the
# wmconfig directory so Quicken can easily be started from a menu.

exedir=/home/executor
cdrom=/dev/cdrom
minfree=12000
wmconfigdir=/etc/X11/wmconfig
quickconfig=quicken
manualconfig=quicken_manual

# make sure executor-aux is installed

if [ ! -d "$exedir" ]; then
  echo You have to install executor-aux and Executor before you can install Quicken
  exit 1
fi

# check for enough disk space

if [ -d "$exedir/Commercial" ]; then
  prefq6="$exedir/Commercial"
else
  prefq6="$exedir"
fi

free=`df -k "$prefq6" | awk 'NR==2{print $4}'`
if [ $free -lt $minfree ]; then
  echo Not enough free space in $prefq6 -- $minfree needed during installation
  exit 1
fi

# Undo MacVolumes so it won't lead us away from the CD drive

unset MacVolumes

# verify the CD-ROM is readable

if [ ! -r "$cdrom" ]; then
  echo "The CD-ROM drive isn't readable"
  exit 1
fi

# verify that the Quicken 6 CD is in the drive

if dd < "$cdrom" bs=6k count=1 2>/dev/null | grep -q 'Quicken Deluxe 6' > /dev/null; then
  >/dev/null;
else
  echo "The Quicken 6 CD is not in $cdrom"
  exit 1
fi

# figure out which flavor of Executor they're using

sysdir="$exedir/System Folder"
cpanel="$sysdir/Control Panels"
pref_file1="$sysdir/Preferences/Quicken 6 Preferences"
pref_file2="$sysdir/Preferences/%Quicken 6 Preferences"

for dir in /usr/local/bin /opt/executor/bin; do
  for exe in executor executor-sdl executor-svga; do
    if [ -x "$dir/$exe" ]; then
      executor="$dir/$exe"
      break 2
    fi
  done
done

if [ "x$executor" = "x" ]; then
  echo "Can't find executor binary"
  exit 1
fi

svga=`expr "$executor" : '.*svga'`

# Make sure we can write where we need to

for f in "$prefq6" /opt/executor/bin /usr/local/bin "$sysdir"; do
  if [ ! -w "$f" ]; then
    echo "$f"' is not writable.  You may need to use the "su" command to
run this installer as root.  If you do, you should type "xhost localhost"
before you type "su".'
    exit 1
  fi
done

# oops -- create directory that should have been in 2.1pr2

if [ ! -e "$cpanel" ]; then
  mkdir "$cpanel"
fi

# make a time-stamp for later
timestamp=/tmp/q6_installer
> $timestamp.$$

# give some instructions

echo 'Starting the Quicken Installer in Executor.  In Executor, click
on each of the four highlited buttons ("Continue", "Continue",
"Install" and "Yes").  Then use the mouse to descend into
/home/executor/Commercial.  Once you get there, click the Install
button.  You don'\''t need to use your keyboard.
'

# start Executor, guess at likely reasons for failure

if $executor -system 7 'Quicken Deluxe 6 CD:Quicken Deluxe 6 Installer'; then
  > /dev/null
else
  echo '
'"$executor failed to run."'
'
  if [ $svga != 0 ]; then
    echo 'Make sure you have configured svgalib properly.'
  elseif [ x"$DISPLAY" = "x" ];
    echo 'You need to use "startx" to start your X server.'
  else
    echo 'If you'\''re using the "su" command, you may need to "suspend" it,
then run "xhost localhost" and then "fg" to resume your super-user session.'
  fi
  echo 'After that, try running this installer again.'
  exit 1    
fi

# find out where they really installed it

for dir in "$prefq6" "$exedir" "$exedir/.."; do
  cd "$dir"
  pwd=`pwd`
  q6folder=`find "$pwd" -follow -name "Quicken 6 Folder" -print 2>/dev/null`
  qd6folder=`expr "$q6folder" : '\(.*\)/.*'`
  if [ "x$qd6folder" != x ]; then
    break
  fi
done

# check for error

if [ "x$qd6folder" = x ]; then
  "Can't find the installed files.  You should have installed in $prefq6"
  exit 1
fi

# remove extraneous files

if [ "$qd6folder" -nt $timestamp.$$ ]; then

  cd "$qd6folder"

  rm -r *"Home Inventory Folder" \
        *"Mutual Fund Finder Folder" \
        *"Quicken Deluxe 6" \
        *"Quicken Preview Folder" \
        "Quicken 6 Folder"/*Guide
fi

rm $timestamp.$$

# TODO: extract bitmaps for menus and start button?
#       that requires an application that isn't ready to ship yet

# install quicken wrapper

qd6="$qd6folder/Quicken 6 Folder/Quicken 6"

echo '#! /bin/sh
exec "'"$executor"'" "'"$qd6"'" $*' > /opt/executor/bin/quicken
chmod +x /opt/executor/bin/quicken
cd /usr/local/bin
if [ ! -e quicken ]; then
  ln -s ../../../opt/executor/bin/quicken .
fi

# install quicken manual wrapper

# NOTE: Executor 2.1pr2 and previous versions have a bug that prevents
#       DocViewer from opening up the files that it is provided on the
#       command line.  This has been fixed internally at ARDI, but the
#       fix didn't make it into 2.1pr2 -- it will be present in the next
#       release, after Linux Expo.

# Preferred size is 600x800 (i.e. portrait), but need to leave 16 and
# 64 pixels for the window frame

if [ $svga != 0 ]; then
    size=""
else
    size=`xwininfo -root | grep geometry | tr 'x+' '  ' | awk '
	{
	  x=$2-16;
          y=$3-64;
	  if (x > 600)
            x = 600;
	  if (y > 800)
	    y = 800;
	  printf ("-size %dx%d", x, y);
        }'`
fi

dv="$qd6folder/User's Manuals Folder/Apple DocViewer"
echo '#! /bin/sh
exec "'"$executor"'" "'"$dv"'" '"$size"' $* "Quicken Deluxe 6 CD:Misc Files 4:Quicken User'"'"'s Manual"' > /opt/executor/bin/quicken_manual
chmod +x /opt/executor/bin/quicken_manual
cd /usr/local/bin
if [ ! -e quicken_manual ]; then
  ln -s ../../../opt/executor/bin/quicken_manual .
fi

# start quicken so they can register it -- this can create permission problems

/usr/local/bin/quicken

# change ownership so that non-root users won't get surprised

user="$USER"
group=`groups | awk '{print $1}'`
writable=n
acceptable=n
cdromfix=y

until expr "$acceptable" : '[yY].*' > /dev/null; do
  acceptable=y
  echo -n "Which user should own the Quicken files ($user)? "
  read newuser
  user=`expr "$newuser" "|" "$user"`
  echo -n "Which group should own the Quicken files ($group)? "
  read newgroup
  group=`expr "$newgroup" "|" "$group"`
  echo -n "Should members of $group be able to write to the Quicken files ($writable)? "
  read newwritable
  writable=`expr "$newwritable" "|" "$writable"`
  if expr "$writable" : '[yY].*' > /dev/null; then
    allowance=can
  else
    allowance=can\'t
  fi

  echo -n "Make CD-ROM readable so users can access Quicken 6 Manual ($cdromfix)? "
  read newcdromfix
  cdromfix=`expr "$newcdromfix" "|" "$cdromfix"`
  if expr "$cdromfix" : '[yY].*' > /dev/null; then
    cdromfix=y
  else
    cdromfix=n
  fi

  echo "user = $user, group = $group, group $allowance write to the Quicken files, fixcd = $cdromfix"
  if grep -q "^$user:" /etc/passwd; then
    > /dev/null
  else
    echo "WARNING: $user does not appear to be a valid user."
    acceptable=n
  fi
  if grep -q "^$group:" /etc/group; then
    > /dev/null
  else
    echo "WARNING: $group does not appear to be a valid group."
    acceptable=n
  fi
  echo -n "Do you want to use these values ($acceptable)? "
  read newacceptable
  acceptable=`expr "$newacceptable" "|" "$acceptable"`
done

if [ $allowance = "can" ]; then
  group_perm=+w
else
  group_perm=-w
fi

chown -R "$user.$group" "$qd6folder" "$pref_file1" "$pref_file2"
chmod -R g$group_perm "$qd6folder" "$pref_file1" "$pref_file2"

if [ $cdromfix = y ]; then
  chmod a+r /dev/cdrom
fi

# TODO: Install Zapf-dingbats font

# NOTE: if we just chuck them into /etc/X11/wmconfig then they'll be visible
# from the NextLevel menu, which is correct, since they're neither GNOME nor
# KDE compliant

if [ -d "$wmconfigdir" ]; then
  cd "$wmconfigdir"
  if [ ! -e "$quickconfig" ]; then
    echo 'quicken name "Quicken"
quicken description "Financial Tool"
quicken exec "quicken &"
quicken group "Applications/Finance"' > "$quickconfig"
  fi

  if [ ! -e "$manualconfig" ]; then
    echo 'quicken_manual name "Quicken Manual"
quicken_manual description "Documentation (requires Quicken CD-ROM in drive)"
quicken_manual exec "quicken_manual &"
quicken_manual group "Applications/Finance"' > "$manualconfig"
  fi

fi

echo '
Quicken is now installed.  You can start Quicken directly by using the
command "quicken" from a terminal, or using the features of your desktop,
window manager or control-panel to add a quicken entry.

When your Quicken CD-ROM is in your CD-ROM drive, you can use the
command "quicken_manual" to view the Quicken Manual.  When Apple
DocViewer starts up, you'\''ll need to choose "Open..." from the
"File" menu, then click on the drive button to find your Quicken CD
and descend into "Misc Files 4" to open the Quicken User'\''s Manual.
Some things that are described in the Quicken User'\''s Manual will work
on a Macintosh, but won'\''t work under Executor.'

exit 0
