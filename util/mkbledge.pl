#!/usr/bin/perl -w


# Fetch the suffix
die "Usage: $0 version_suffix\n" if ($#ARGV != 0);
$suffix = $ARGV[0];

chop ($dst_dir = `pwd`);

&make_dos;
&make_linux_aout;
&make_linux_elf;
&make_nextstep;


sub make_dos
{
    local ($zip_name);

    print "Creating DOS archive...\n";

    if ($suffix =~ /^199[a-z]$/) {
	$zip_name = "exec$suffix";
    } elsif (($vers) = ($suffix =~ /^199([a-z][0-9])$/)) {
	$zip_name = "exec19$vers";
    } elsif (($vers) = ($suffix =~ /^199([a-z][0-9][0-9])$/)) {
	$zip_name = "exec9$vers";
    } else {
	die "I don't know how to construct a DOS archive name.\n";
    }

    $zip_name .= '.zip';

    system ("rm -f $dst_dir/$zip_name");
    system ("cd dos-$suffix ; zip -9 $dst_dir/$zip_name executor.exe")
	&& die "Unable to create DOS zip archive.\n";

    print "done.\n";
}


sub linux_common
{
    local ($exe_format) = @_;

    die if ($dst_dir eq "/" || $dst_dir eq "");

    print "Creating Linux/$exe_format archive...\n";

    $x_dir = "linux-$exe_format-$suffix";
    $svgalib_dir = "svgalib-$exe_format-$suffix";

    system ("rm -rf $dst_dir/usr");
    die "Can't mkdir: $!\n" unless mkdir ("$dst_dir/usr", 0755);
    die "Can't mkdir: $!\n" unless mkdir ("$dst_dir/usr/local", 0755);
    die "Can't mkdir: $!\n" unless mkdir ("$dst_dir/usr/local/bin", 0755);
    system ("cp -v $x_dir/executor $dst_dir/usr/local/bin/executor")
	&& die "cp failed\n";
    system ("strip $dst_dir/usr/local/bin/executor")
	&& die "strip failed\n";
    system ("cp -v $svgalib_dir/executor $dst_dir/usr/local/bin/executor-svga")
	&& die "cp failed\n";
    system ("strip $dst_dir/usr/local/bin/executor-svga")
	&& die "strip failed\n";

    if (system ("chown -R root.root $dst_dir/usr")) {
	print STDERR "Warning: unable to chown files to root.root\n";
    }

    if (system ("chmod 755 $dst_dir/usr/local/bin/executor")) {
	print STDERR "Warning: unable to chmod 755 executor\n";
    }

    if (system ("chmod 4755 $dst_dir/usr/local/bin/executor-svga")) {
	print STDERR "Warning: unable to chmod 4755 executor-svga\n";
    }

    if (system ("GZIP=-9v "
		. "tar --directory $dst_dir --create --verbose "
		. "--gzip --file $exe_format-$suffix.tar.gz "
		. "usr/local/bin/executor "
		. "usr/local/bin/executor-svga")) {
	die "tar failed.\n";
    }
    system ("rm -rf $dst_dir/usr");

    print "Done.\n";
}   

sub make_linux_aout
{
    &linux_common ("aout");
}


sub make_linux_elf
{
    &linux_common ("elf");
}


sub make_nextstep
{
}
