#!/usr/bin/perl
use warnings;

while($f = <$ARGV[0]/*.h>) {
    open(INCLUDE, $f);

    while($l = <INCLUDE>) {
        if($l =~ /^#define ([A-Za-z0-9_]+)\(([^)]*)\) ([A-Za-z0-9_]+)\((.*)\) *$/) {
            if($2 eq $4) {
                $newnames{$1} = $3;
                $oldnames{$3} = $1;
                print "OLD=>NEW: $1 => $3\n";
            }
        } elsif($l =~ /^#define ([A-Za-z0-9_]+)\((.*)\)[ \t]*\\$/) {
            $old = $1;
            $oldarg = $2;
            $l = <INCLUDE>;
            if($l =~ /^[ \t]*([A-Za-z0-9_]+)\((.*)\)$/ && $1 ne "x") {
                $newnames{$old} = $1;
                $oldnames{$1} = $old;
                print "OLD->NEW: $old -> $1\n";
            }
        }

        if($l =~ /^EXTERN_API\(.*\)/) {
            $l = <INCLUDE>;
            if($l =~ /([A-Za-z0-9_]+)\(/) {
                $name = $1;
                $name =~ s/^Mac//;

                while($l && $l !~ /\)/) {
                    $l = <INCLUDE>;
                }
                if($l =~ /\b[A-Z]+WORDINLINE\((.*)\)/) {
                    $code = join(' ',map { s/0x//r } split(/, */,$1));
                    
                    $trap = $1 if($code =~ /(\bA...\b)/);

                    if($code =~ /^A...$/) {
                        print $name, " -> simple: ", $code ,"\n";
                        $kind = "simple";
                        $selector = 0;
                    } elsif($code =~ /^70(..) (A...)$/) {
                        print $name, " -> D0=$1 $2", "\n";
                        $kind = "D0W";
                        $selector = $1;
                        if($selector =~ /[0-7]./) {
                            $selector = "00$selector";
                        } else {
                            $selector = "FF$selector";
                        }
                    } elsif($code =~ /^303C (....) (A...)$/) {
                        print $name, " -> D0.W=$1 $2", "\n";
                        $kind = "D0W";
                        $selector = $1;
                    } elsif($code =~ /^203C (....) (....) (A...)$/) {
                        print $name, " -> D0.L=$1$2 $3", "\n";
                        $kind = "D0L";
                        $selector = "$1$2";
                    } elsif($code =~ /^4267 (A...)$/) {
                        print $name, " -> Stack.W=0 $1", "\n";
                        $kind = "StackW";
                        $selector = "0";
                    } elsif($code =~ /^70(..) 3F00 (A...)$/) {
                        print $name, " -> Stack.W=$1 $2", "\n";
                        $kind = "StackW";
                        $selector = $1;
                        if($selector =~ /[0-7]./) {
                            $selector = "00$selector";
                        } else {
                            $selector = "FF$selector";
                        }
                    } elsif($code =~ /^3F3C (....) (A...)$/) {
                        print $name, " -> Stack.W=$1 $2", "\n";
                        $kind = "StackW";
                        $selector = $1;
                    } elsif($code =~ /^2F3C (....) (....) (A...)$/) {
                        print $name, " -> Stack.L=$1$2 $3", "\n";
                        $kind = "StackL";
                        $selector = "$1$2";
                    } else {
                        print $name, " -> complicated: $code\n";
                        $kind = "complicated";
                    }

                    if($kind ne "complicated") {
                        $traps{$name} = $trap;
                        $entrypoints{$trap} = [] if(!exists $entrypoints{$trap});
                        push @{ $entrypoints{$trap} }, $name;
                        $selectors{$name} = $selector;
                        if(exists $kinds{$trap}) {
                            if($kinds{$trap} ne $kind) {
                                print "mismatch: $trap $name $kinds{$trap} $kind\n";
                            }
                        }
                        $kinds{$trap} = $kind;
                    }

                }
            }
        }
    }

    close(INCLUDE);
}

foreach $trap (keys %kinds) {
    print $trap, "(", $kinds{$trap}, ")";
    foreach $name (@{$entrypoints{$trap}}) {
        if($kinds{$trap} eq "simple") {
            print " ", $name;
        } else {
            print " ", $selectors{$name}, ":", $name;
        }
    }
    print "\n";
}

while($f = <include/*.h>) {
    open(INCLUDE, $f);
    open(OUT, ">temp.h");
    while($l = <INCLUDE>) {
        if($l =~ /^PASCAL_SUBTRAP\(([A-Za-z0-9_]+), (0x[0-9A-Z]+), ([A-Za-z0-9_]+)\)/) {
            $name = $1;
            $origname = $name;
            $trapname = $3;
            $oldtrapnum = $2;
            $name = $newnames{$name} if(exists $newnames{$name});
            if(exists $selectors{$name}) {
                $selector = "0x$selectors{$name}";
                $trapnum = "0x$traps{$name}";
                print OUT "PASCAL_SUBTRAP($origname, $trapnum, $selector, $trapname);\n";
                print $name, " ", $trapnum, " ", $selector, "\n";
            } else {
                print $name, ": unknown\n";
                print OUT "PASCAL_SUBTRAP_UNKNOWN($origname, $oldtrapnum, $trapname);\n";
            }
        } else {
            print OUT $l;
        }
    }
    close(OUT);
    close(INCLUDE);
    system "mv temp.h $f";
}
