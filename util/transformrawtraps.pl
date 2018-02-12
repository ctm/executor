#!/usr/bin/perl

# This script was used to
# 1. transform calls to RAW_68K_FUNCTION to RAW_68K_TRAP where appropriate

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

$f = "include/rsys/emustubs.h";
open(INCLUDE, $f);
open(OUT, ">temp.h");
while($l = <INCLUDE>) {
    if($l =~ /^RAW_68K_FUNCTION\(([A-Za-z0-9_]+)\)/) {
        $name = $1;
        $trapnum = 0;
        if(exists $traps{$name}) {
            $trapnum = $traps{$name};
        } elsif(exists $traps{"_$name"}) {
            $trapnum = $traps{"_$name"};
        } elsif(exists $traps{"R_$name"}) {
            $trapnum = $traps{"R_$name"};
        }
        
        if($trapnum) {
            print OUT "RAW_68K_TRAP($name, $trapnum);\n";
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
