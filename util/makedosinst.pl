#!/usr/bin/perl

# This perl script takes a directory containing an Executor/DOS
# distribution and creates a Jered-style installation script.  I'll
# clean this up once we have a better build procedure in place.
# Once the script is created, you copy it to DOS, make sure you have
# a copy of the Executor release in the DOS directory you specified,
# cd to the directory containing `makedisk', and type:
#
# makedisk C:\install.scr
#
# where C:\install.scr is the path where you put the script created
# by this program.
#
# Sample usage of this perl script:
#
# makedosinst /releases/dos/1.99c '1.99c' 'C:\EXECUTOR.19C' install.scr

# Make sure we've got the right number of arguments.
if ($#ARGV != 3 && ($#ARGV != 4 || $ARGV[0] ne '-self-extracting')) {
    die "Usage: $0 [-self-extracting] distribution_directory release_name "
	. "DOS_distribution_directory dest_script_file\n";
}

if ($ARGV[0] eq '-self-extracting') {
    shift @ARGV;
    $self_extracting = 1;
} else {
    $self_extracting = 0;
}

$src_dir      = $ARGV[0];
$version_name = $ARGV[1];
$dos_dir      = $ARGV[2];


open (STDOUT, ">$ARGV[3]") || die "Unable to open output file.\n";

$dos_dir =~ tr/a-z/A-Z/;
&check_dos_pathname ($dos_dir, $dos_dir);

open (PREAMBLE, "<dosinst.preamble")
    || die "Can't open \"dosinst.preamble\"!\n";
chdir ($src_dir) || die "Unable to read directory \"$src_dir\".\n";

# Run find to get all the relevant filenames.
open (FILES, "(cd $src_dir && find . -type f -print) | sort |")
    || die "Unable to run find!";

if ($self_extracting) {
    print "SFX=TRUE\r\nBUILDTO=C:\\TMP\r\nDRIVESIZE=1024000000\r\n";
} else {
    print "BUILDTO=A:\r\nDRIVESIZE=1457664\r\n";
}

# Copy the install script preamble to stdout
while (<PREAMBLE>) {
    chop;
    $_ =~ s/\@executor-version@/$version_name/g;
    print $_ . "\r\n";
}

# Print out the FILE section header.
print ("; Flag denoting the start of the file section.\r\n" .
       "*FILE\r\n");

while (<FILES>)
{
    chop;

    ($orig_path = $_) =~ s/^.\///g;		# Remove initial "./".
    ($path = $orig_path) =~ tr/a-z/A-Z/;	# Convert to upper case.
    $path =~ s/\//\\/g;				# Translate "/" to "\".

    &check_dos_pathname ($path, $_);

    # Prompt for overwrites for all files except the System File,
    # which they must overwrite.
    if ($path =~ /\.HFV/ && $path !~ /EXSYSTEM\.HFV/) {
	$prompt_before_overwrite = 1;
    } else {
	$prompt_before_overwrite = 0;
    }

    if ($self_extracting || $path !~ /README.1ST/) {
	if ($path =~ /\....$/) {
	    ($compressed_path = $path) =~ s/.$/\$/g;
	} elsif ($path =~ /\./) {
	    ($compressed_path = $path) =~ s/.$/\$/g;
	} else {
	    $compressed_path = $path . ".\$";
	}

	if ($paths_in_use{$compressed_path}) {
	    die "I end up using compressed filename \"$compressed_path\""
		. "for files \"$orig_path\" and \""
		    . $paths_in_use{$compressed_path} . "\"!\n";
	}
	$paths_in_use{$compressed_path} = $orig_path;

	# If it has any lower case letters, assume we want it compressed.
	$file_list .= "ORIG=$dos_dir\\$path; SOURCE=INSTDATA"
	    . "; DEST=$path; OPN=3 CPN=$prompt_before_overwrite\r\n";
    } else {
	# It's a README, so don't compress it, and put it first.
	$file_list = "ORIG=$dos_dir\\$path; SOURCE=$path; DEST=$path; "
	    . "OPN=0 CPN=$prompt_before_overwrite\r\n"
		. $file_list;
    }
}

# Print out the file list and the FILE section trailer.
print $file_list;
print (";End of File marker\r\n" .
       "*END\r\n");


# This subroutine verifies that a pathname is suitable for DOS.
sub check_dos_pathname {
    $dos_path = $_[0];
    $orig_path = $_[1];

    # Make sure this filename will work well in DOS land.
    if ($dos_path =~ /\..*\./) {
	die "Too many periods in filename \"" . $orig_path . "\"\n";
    }
    elsif ($dos_path =~ /[^\._[A-Z]\d\\]/) {
	die "Non-alphanumeric character in filename \"" . $orig_path . "\"\n";
    }
    elsif ($dos_path =~ /[_A-Z\d]{9,}/) {
	die "Too many characters before the `.' in filename \""
	    . $orig_path . "\"\n";
    }
    elsif ($dos_path =~ /\..{4,}/) {
	die "Too many characters after the `.' in filename \""
	    . $orig_path . "\"\n";
    }
    elsif ($dos_path =~ /\..*\\/) {
	die "There is a `.' in a directory name in filename \""
	    . $orig_path . "\"\n";
    }
}
