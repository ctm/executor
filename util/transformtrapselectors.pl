#!/usr/bin/perl

# This script was used to
# 1. transform calls to PASCAL_FUNCTION to PASCAL_SUBTRAP where appropriate

use warnings;

open(TRAPINFO, "trapinfo");

my(@l, @F, %traps, %selectors);

while($l = <TRAPINFO>) {
    @F = split(/[ \t\n]+/,$l);
    #print(scalar(@F), " ", $F[1], " | ", $F[1], "\n");

    $traps{$F[1]} = $F[0] if(@F >= 2);
    if(@F > 2) {
        for($i = 0; $i < @F; $i++) {
            $selectors{$F[$i]} = $F[1];
        }
    }
}

while($f = <include/*.h>) {
    open(INCLUDE, $f);
    open(OUT, ">temp.h");
    while($l = <INCLUDE>) {
        if($l =~ /^PASCAL_FUNCTION\(([A-Za-z0-9_]+)\)/) {
            $name = $1;
            if(exists $selectors{$name}) {
                $trapname = $selectors{$name};
                $trapnum = $traps{$trapname};
                print OUT "PASCAL_SUBTRAP($name, $trapnum, $trapname);\n";
            } else {
                print OUT $l;
            }
        } else {
            print OUT $l;
        }
    }
    close(OUT);
    close(INCLUDE);
    system "mv temp.h $f";
}
