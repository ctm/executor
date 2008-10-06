#!/usr/bin/perl

while (<STDIN>) {
    if (!$found && /^\#include/) {
	print "#include \"rsys/common.h\"\n";
	$found = 1;
    }
    print $_;
}
