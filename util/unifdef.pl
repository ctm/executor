#!/usr/bin/perl

$variables{'BINCOMPAT'} = 1;
$variables{'USE_BIOS_TIMER'} = 0;
$variables{'DOS'} = 0;

sub checkcond {
    my $cond = $1;

    #print "$cond\n";
    if($cond =~ /^\s*(!?)\s*defined\s*\(\s*([a-zA-Z0-9_]+)\s*\)\s*$/) {
        my $negate = $1;
        my $var = $2;
        if(exists $variables{$var}) {
            $val = $variables{$var};

            # print "VARIABLE ($negate) $var = $val\n";

            $val = !$val if $negate;
            return $val;
        }
    }
    return 'boring';
}

#while($f = <*.cpp include/*.h include/rsys/*.h>) {
foreach $f (@ARGV) {
#$f = "include/rsys/list.h";
    open(SRC, $f);
    open($OUT, ">temp.cpp");
    print $f, "\n";

    $level = 0;
    @stack = ();
    $state = 'active';

    while($l = <SRC>) {
        if($l =~ /^\s*#\s*if\b(.*)/) {
            my $rawcond = $1;
            my $cond = checkcond($rawcond);
            # print "IF statement: $rawcond == $cond, state == $state\n";
            $level++;
            push(@stack, $state);
            if($state eq 'active' || $state eq 'iftrue') {
                if($cond eq 'boring') {
                    $state = 'active';
                    print $OUT $l;
                } elsif($cond) {
                    $state = 'iftrue';
                } else {
                    $state = 'iffalse';
                }
            } else {
                $state = 'nested';
            }
        } elsif($l =~ /^\s*#\s*if(n?)def \s*([a-zA-Z0-9_]+)\s*/) {
            my $negate = $1;
            my $var = $2;
            my $cond = 'boring';
            if(exists $variables{$var}) {
                $val = $variables{$var};

                # print "VARIABLE ($negate) $var = $val\n";

                $val = !$val if $negate;
                $cond = $val;
            }
            # print "IFDEF statement: $rawcond == $cond, state == $state\n";
            $level++;
            push(@stack, $state);
            if($state eq 'active' || $state eq 'iftrue') {
                if($cond eq 'boring') {
                    $state = 'active';
                    print $OUT $l;
                } elsif($cond) {
                    $state = 'iftrue';
                } else {
                    $state = 'iffalse';
                }
            } else {
                $state = 'nested';
            }
        } elsif($l =~ /^\s*#\s*elif (.*)/) {
            my $rawcond = $1;
            my $cond = checkcond($rawcond);
            # print "ELIF statement: $rawcond == $cond, state == $state\n";
            
            if($state eq 'iffalse') {
                if($cond eq 'boring') {
                    $state = 'active';
                    $l =~ s/elif/if/;
                    print $OUT $l;
                } elsif($cond) {
                    $state = 'iftrue';
                } else {
                    $state = 'iffalse';
                }
            } elsif($state eq 'skipping') {
                $state = 'skipping';
            } elsif($state eq 'active') {
                print $OUT $l;
            }
        } elsif($l =~ /^\s*#\s*else/) {
            # print "ELSE statement: state == $state\n";

            if($state eq 'iffalse') {
                $state = 'iftrue';
            } elsif($state eq 'skipping' || $state eq 'iftrue') {
                $state = 'skipping';
            } elsif($state eq 'active') {
                print $OUT $l;
            }
        } elsif($l =~ /^\s*#\s*endif/) {
            # print "ENDIF statement: state == $state\n";

            $level--;
            print $OUT $l if($state eq 'active');
            $state = pop(@stack);
        } else {
            print $OUT $l if $state eq 'active' || $state eq 'iftrue';
        }
    }

    close($OUT);
    close(INCLUDE);
    system "mv temp.cpp $f";
}
