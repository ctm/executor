#!/usr/bin/perl

die "Usage: mkstubdefns.pl low_arg_count high_arg_count\n"
    if ($#ARGV != 1);

$low = $ARGV[0];
$high = $ARGV[1];

for ($k = $low; $k <= $high; $k++) {
    print "\#undef P$k\n";
    print "\#undef Q$k\n";
    &output_p_defn ($k);
    &output_q_defn ($k);
    print "\#define Q_SAVED0D1A0A1_$k\tQ$k\n";
    print "\#define Q_SAVED1A0A1_$k\tQ$k\n";
}

sub make_list {
    local ($first, $last, $tmpl, $c) = @_;
    local ($i, $new);
    local ($ret);
    
    $ret = "";
    for ($i = $first; $i <= $last; $i++) {
	$ret .= "$c " if ($i != $first);
	($new = $tmpl) =~ s/\@/$i/g;
	$ret .= $new;
    }
    $ret;
}


sub output_p_defn {
    local ($num_args) = @_;
    local ($i);
    
    $leading_comma = $num_args ? ", " : "";

    $arg_list = ", " . &make_list (0, $num_args, 't@, n@', ',');
    $a_list = &make_list (1, $num_args, 'A@', ',');
    $assign_list = &make_list (1, $num_args, '\%s __stub_arg_@ = (A@)',
			       ";\\\\\\n  ");
    $lookup_list = ($leading_comma
		    . &make_list (1, $num_args, 'map_type_name ( #t@ )',
				  ','));
    $assign_list .= ";\\\\\\n" if ($num_args);
    $shared_arg_list = (&make_list (1, $num_args, '__stub_arg_@', ','));

    $defn = <<EOF;

\#define P$num_args(at, v $arg_list)
do \{
if (at)
  printf (
    "#define " \#n0 "($a_list) \\\\\\n"
    " ({ \\\\\\n"
    "   void *new_addr;\\\\\\n"
    "   $assign_list \\\\\\n"
    "   new_addr = tooltraptable\[0x\%04lx\];\\\\\\n"
    "   ((new_addr == toolstuff\[0x\%04lx\].orig)\\\\\\n"
    "    ? C_" \#n0 "($shared_arg_list)\\\\\\n"
    "    : (" \#t0 ") CToPascalCall (SYN68K_TO_US(new_addr), CTOP_" \#n0 " $leading_comma $shared_arg_list));\\\\\\n"
    "  })\\n"
    $lookup_list,
    (unsigned long) at - 0xA800, (unsigned long) at - 0xA800);
\} while (0);
EOF

    $old_multiline = $*;
    $* = 1;

    $defn =~ s/\n/\t\\\n/g;
    $defn =~ s/^\s*\\\n//g;

    $* = $old_multiline;
    print "$defn\n";
}


sub output_q_defn {
    local ($num_args) = @_;
    local ($i);

    $arg_list = &make_list (0, $num_args, 't@, n@', ',');
    $arg_list = ", " . $arg_list;
    $a_list = &make_list (1, $num_args, 'A@', ',');
    $paren_a_list = &make_list (1, $num_args, '(A@)', ',');

    $defn = <<EOF;

\#define Q$num_args(nn0, at, v $arg_list)
do \{
  printf (
    "#define " \#n0 "($a_list) \\\\\\n"
    "   ( "
    "    C_" \#n0 "($paren_a_list) "
    "    )\\n"
	);
\} while (0);
EOF

    chop $defn;

    $old_multiline = $*;
    $* = 1;

    $defn =~ s/\n/\t\\\n/g;
    $defn =~ s/^\s*\\\n//g;

    $* = $old_multiline;
    print "$defn\n";
}
