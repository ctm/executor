#!/usr/bin/perl

# Single use script for line-breaking function declarations
# Expanding the P1.. macros left some very long lines,
# and clang-format's line breaking seems to be an all-or-nothing
# proposition (it unwraps nicely formatted lines it considers too short)

use Text::Trim qw(trim);

while($f = <*.cpp>) {
    open(SRC, $f);
    open(OUT, ">temp.cpp");

    while($l = <SRC>) {
        while($l =~ /^(PUBLIC|PRIVATE)(( [A-Za-z0-9_:]+)+)\([^)]*$/) {
            $l .= <SRC>;
        }

        if($l =~ /^(PUBLIC|PRIVATE)(( [A-Za-z0-9_:*<>]+)+)\(((.|\n)*)\)(.*)$/) {            
            my $decl, $args, $comments;
            $decl = $1 . " " . $2;
            $allargs = trim($4);
            $comments = trim($6);

            $decl =~ s/\bPRIVATE\b/static/;
            $decl =~ s/\bPUBLIC\b//;
            $decl =~ s/\bpascal\b//;
            $decl =~ s/\btrap\b//;
            $decl =~ s/\s+/ /;
            trim($decl);
            #print $comments, "\n";
           
            @args = split(/,/, $allargs);
            for(my $i = 0; $i < scalar @args; $i++) {
                if($args[$i] =~ s/(\/\*.*\*\/)//) {
                    $comments = $1 . $comments;
                }
                trim $args[$i];
            }
            print OUT "$decl(";
            my $width;
            if(length $decl < 40 || length "$decl($allargs)$comment" <= 80) {
                $width = 80 - length $decl;
            } else {
                print OUT "\n";
                $width = 75;
            }
            my $remaining = $width;
            for(my $i = 0; $i < scalar @args; $i++) {
                my $arg = $args[$i];
                trim $arg;
                if($i == scalar @args - 1) {
                    $arg .= ")" . $comments;
                } else {
                    $arg .= ", ";
                }
                if($remaining - length $arg <= 0) {
                    print OUT "\n";
                    $remaining = $width - length $arg;
                } else {
                    $remaining -= length $arg;
                }
                print OUT $arg;
            }
            if(scalar @args == 0) {
                print OUT ")$comments";
            }
            print OUT "\n";
        } else {
            print OUT $l;
        }
    }

    close(OUT);
    close(INCLUDE);
    system "clang-format temp.cpp > $f";
    system "rm temp.cpp";
}
