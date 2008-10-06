#!/usr/bin/perl

@globals = <STDIN>; $globals = join ("", @globals);
$prefix = $ARGV[0];

print "\t.space 0x10000\n";

while ($globals =~ m/\nDATA\([^,]*,([^,]*),[^,]*,([^,]*),.*\)/g)
{
    print "\t.set $prefix$1, $2\n";
    print "\t.globl $prefix$1\n";
}
