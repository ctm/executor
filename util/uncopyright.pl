#!/usr/bin/perl

# Match multi-line regexps
$* = 1;

$str = "";
while (<STDIN>) {
    $str .= $_;
}

if (($copyright_string)
    = ($str =~ /char ROMlib_copyright[^=]*=\s*\"([^\"]*)\"\;/)) {
    $copyright_string =~ s/\\\n//g;
    $copyright_string =~ s/Research and Development/Research and\n * Development/g;
    $str =~ s/char ROMlib_copyright[^=]*=\s*\"[^\"]*\"\;/\/\* $copyright_string\n \*\//g;
}

$str =~ s/(char ROMlib_rcsid_[^=]*=\s*\"[^"]*\$\"\;)/\#if !defined (OMIT_RCSID_STRINGS)\n$1\n\#endif/g;

print $str;
