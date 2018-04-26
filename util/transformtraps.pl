#!/usr/bin/perl

# This script was used to
# 1. insert calls to the PASCAL_TRAP and PASCAL_FUNCTION macros into the headers
# 2. transform some #defines to enums.
#
# Manual postprocessing was required. This script should not ever be needed again.
# All the changes are now in these two commits:
# 29e10aad3c1047f0613b9d945df56968fe09f86a
# d05a93ed9c0b358fa127c73e5a68c4404af3bb01

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
        if($l =~ /^\s*extern pascal trap .* C_([A-Za-z0-9_]+)/) {
            my($name);
            $name = $1;
            while($l !~ /\)\s*;/) {
                $l .= <INCLUDE>;
            }
            #print($name, "\n");
            print OUT $l;
            if(my $trapnum = $traps{$name}) {
                print OUT "PASCAL_TRAP($name, $trapnum);\n";
            } else {
                print OUT "PASCAL_FUNCTION($name);\n";
            }
        } else {
            $items = "";
            while($l =~ /^#define[ \t]+([a-zA-Z_0-9]+)[ \t]+(.+)$/) {
                my($name, $val);
                $name = $1;
                $val = $2;
                last if ($val =~ /[*,\\]/) or ($val =~ /CWC/) or ($val =~ /CLC/) or ($val =~ /GUEST/);
                $items .= "    $name = $val,\n";
                $l = <INCLUDE>;
            }

            if($items) {
                print OUT "enum\n{\n";
                print OUT $items;
                print OUT "};\n";
            }
       
            print OUT $l;
        }
    }
    close(OUT);
    close(INCLUDE);
    system "mv temp.h $f";
}
