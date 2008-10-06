#!/usr/bin/perl

# Make sure we've got the right number of arguments.
if ($#ARGV < 3) {
    die "Usage: $0 [-define m1=v1 [-define m2=v2] ...] infile.meta "
	. "outfile.S outfile.h opfind.c\n";
}

$arg = 0;
while ($ARGV[$arg] eq '-define') {
    ($name, $val) = ($ARGV[$arg + 1] =~ /^([^=]*)=([^=]*)$/);
    die "bogus macro name\n" if (!$name);
    $macros{$name} = $val;
    $arg += 2;
}

die "Unable to open input file " . $ARGV[$arg] . ".\n"
    if (!open (INPUT,   "<" . $ARGV[$arg]));
($include_dir) = ($ARGV[$arg] =~ /^(.*)\/[^\/]*$/);
die "Unable to open output file " . $ARGV[$arg + 1] . ".\n"
    if (!open (ASM_OUT, ">" . $ARGV[$arg + 1]));
die "Unable to open output file " . $ARGV[$arg + 2] . ".\n"
    if (!open (C_OUT,   ">" . $ARGV[$arg + 2]));
$opfind_path = $ARGV[$arg + 3];


# Start the @include stack with the explicitly specified input file
push (@stream_stack, "INPUT");
push (@lineno_stack, 0);


# Start out with no asm's.
$num_asm = 0;

print STDERR "Pass 1: ";

# Process all the input
while (length ($line = &next_line)) {
    @cmd = split (/\s+/, $line);
    if ($cmd[0] eq '@setup') {
	die "Wrong \# of args to $cmd[0] on line $lineno.\n" if (@cmd != 3);
	die "Botched param name \"$cmd[1]\" on line $lineno.\n"
	    if (!($cmd[1] =~ /^\@param[\w]*\@$/));
	&handle_setup ($cmd[1], $cmd[2]);
    } elsif ($cmd[0] eq '@define') {
	die "Missing \@define argument on line $lineno.\n" if (@cmd < 2);
	$macros{$cmd[1]} = join ("\t",  @cmd[2..$#cmd]);
	if ($cmd[1] eq 'PREFIX') {
	    $prefix = $macros{"PREFIX"};
	    $prefix_stub = $prefix . "_stub_t";
	}
    } elsif ($cmd[0] eq '@include') {
	die "Botched \@include argument on line $lineno.\n" if (@cmd != 2);
	die "Include depth too deep on line $lineno.\n" if (@cmd > 50);
	$new_input = "FILE" . ++$fileno; # Unique filehandle name

	if (!open ($new_input, $cmd[1])) {
	    if (!$include_dir || !open ($new_input, "$include_dir/$cmd[1]")) {
		die "Unable to \@include $cmd[1] or $include_dir/$cmd[1] " .
		    "on line $lineno.\n";
	    }
	    push (@stream_stack, $new_input);
	    push (@lineno_stack, 0);
	}
    } elsif ($cmd[0] eq '@meta') {
	die "Wrong \# of args to $cmd[0] on line $lineno.\n" if (@cmd != 2);
	&handle_meta ($cmd[1]);
    } else {
	die "Unknown command on line $lineno.\n";
    }
}


print STDERR "done.\nCompiling opfind...";


# Dump out everything we have to asmsamples.h
open (ASMSAMPLES, "> asmsamples.h")
    || die "Can't open asmsamples.h!";
print ASMSAMPLES "void dummy_func (void) \{\n" . $asm_list . "\}\n\n";
print ASMSAMPLES $asm_decls;
print ASMSAMPLES "static const asm_code_t asm_code\[$num_asm\] = \{\n"
    . $asm_code . "};\n";
close (ASMSAMPLES);

# Compile opfind
    unlink ("opfind");
    die "\nProblems making opfind.\n"
	if (system ("make -s opfind > /dev/null"));

print STDERR "done.\nPass 2: ";

foreach $name (keys %asm_index) {
    $index = $asm_index{$name};
    $template_code = $raw_code{$name};

    print STDERR "$name...";

    $gen_code = "";

    # First pass: crank out fixed bytes plus any operands that require
    # no computation.
    $cmd = "./opfind $index -genasm ";
    foreach $key (keys %setup_code) {
	if (!length ($setup_code{$key})) {
	    $cmd .= " $param_magic{$key} \'$setup_reg{$key}\'";
	    $template_code =~ s/$key/0/g;
	}
    }

    # Read in the assembly code and append it to our string.
    open (PASS1_PIPE, "$cmd |") || die "Can't get asm for pass 2!\n";
    while (<PASS1_PIPE>) {
	$gen_code .= $_;
    }
    close (PASS1_PIPE) || die "Problems with pass 1.\n";

    # Second pass: for each unprocessed operand, crank out code to compute
    # it, then crank out code to write it to memory.

    while (($param) = ($template_code =~ /(\@param_[\w]*\@)/)) {
	# Delete that parameter so we don't process it again.
	if (!defined ($setup_code{$param})) {
	    die "Unknown parameter \"$param\", near line $lineno.\n";
	}
	$template_code =~ s/$param/0/g;

	$gen_code .= $setup_code{$param};

	$cmd = "./opfind $index -genasm-skip-fixed $param_magic{$param} "
	    . "\'$setup_reg{$param}\'";

	# Read in the assembly code and append it to our string.
	open (PASS2_PIPE, "$cmd |") || die "Can't get asm for pass 2!\n";
	while (<PASS2_PIPE>) {
	    $gen_code .= $_;
	}
	close (PASS2_PIPE) || die "Problems with pass 2\n";
    }

    # Append code to increment the code pointer.
    open (SIZE_PIPE, "./opfind $index -print-size |")
	|| die "Can't get size!\n";
    $size = <SIZE_PIPE> + 0;
    close (SIZE_PIPE) || die "Problems getting size!\n";
    if ($size == 1) {
	$gen_code .= "\tincl\t%edi\n";
    } elsif ($size) {
	$gen_code .= "\taddl\t\$$size,%edi\n";
    }

    $genfunc_labels{$gen_code . $literal_code{$name} . "\tret\n"} .=
	"\t.globl\t_" . $prefix . "_" . $name . "\n_" . $prefix
	    . "_" . $name . ":\n";
}


print STDERR "done.\n";


print C_OUT "typedef char " . $prefix . "_stub_t;\n\n";
print ASM_OUT "\t.text\n\n";


# Sort the labels corresponding to each piece of code.
foreach $key (keys %genfunc_labels) {
    @label_array = split (/\n/, $genfunc_labels{$key});
    $genfunc_labels{$key} = join ("\n", sort (@label_array)) . "\n";
}


# Helper function to sort an associative array by value, rather than by key
sub by_value {
    $genfunc_labels{$a} cmp $genfunc_labels{$b};
}


# Now dump out the assembly-generating functions, sorted by first label.
foreach $key (sort by_value (keys %genfunc_labels)) {
    $labels = $genfunc_labels{$key};

    print ASM_OUT "\n" if ($first_line++);	# Skip first newline.
    print ASM_OUT "\t.align\t4,0x90\n";
    print ASM_OUT "$labels";
    print ASM_OUT $key;

    # Output phony C declarations for these symbols.

    $decls = $labels;
    $decls =~ s/^_$prefix(\w*)/extern $prefix_stub $prefix$1 asm ("_$prefix$1")/gm;
    $decls =~ s/.*\.globl.*//gm;
    $decls =~ s/\:/\;/gm;
    $decls =~ s/^\n+//gm;
    $decls =~ s/\n+/\n/gm;

    print C_OUT $decls;
}


# Returns the next non-empty line after stripping out comments, or the
# empty string if there are no more lines.
sub next_line {
    local ($line);
    local ($file_handle);
    while (@stream_stack) {
	$file_handle = pop @stream_stack;
	$lineno = pop @lineno_stack;
	while (<$file_handle>) {
	    ++$lineno;
	    ($line) = /(^[^\#]*)/;	# We begin comments with '#' characters
	    $line =~ s/\s*$//;		# Strip trailing whitespace
	    if (length ($line)) {
		push (@stream_stack, $file_handle);
		push (@lineno_stack, $lineno);
		$lineno_stack[$#lineno_stack] = $lineno; # Remember lineno
		return $line;
	    }
	}

	close ($file_handle);
    }

    "";
}


sub make_asm_string {
    local ($code, $which, %constants) = @_;
    local ($old_multiline);
    local ($s);

    # Switch to multiline pattern matching.

    $s = ("_asm_start_$which:\n"
	  . "asm_start_$which:\n"
	  . $code
	  . "_asm_end_$which:\n"
	  . "asm_end_$which:\n");
    $s =~ s/\n/\\n/gm;
    $s =~ s/\r/\\r/gm;
    $s =~ s/\t/\\t/gm;

    # Restore old multiline pattern matching behavior.

    while (($param) = ($s =~ /(\@param_[\w]*\@)/)) {
	if (!defined ($constants{$param})) {
	    die "Unknown parameter \"$param\", near line $lineno.\n";
	}
	$s =~ s/$param/$constants{$param}/g;
    }

    # Check for parameters missing the trailing @ sign.
    if (($param) = ($s =~ /(\@param[\w]*)/)) {
	die "Missing trailing \@ for $param, near line $lineno.\n";
    }

    "\"$s\"";
}


sub create_asm_samples {
    local ($code, $name) = @_;
    local ($cstring);

    $asm_index{$name} = $num_asm;
    $cstring = &make_asm_string ($code, $num_asm, %param_magic);
    $asm_list .= "asm volatile ($cstring);\n";
    $asm_decls .= "extern unsigned char asm_start_$num_asm;\n";
    $asm_decls .= "extern unsigned char asm_end_$num_asm;\n";
    $asm_code .= "\{ \&asm_start_$num_asm, \&asm_end_$num_asm, $cstring \},\n";
    ++$num_asm;

    $cstring = &make_asm_string ($code, $num_asm, %magic_a);
    $asm_list .= "asm volatile ($cstring);\n";
    $asm_decls .= "extern unsigned char asm_start_$num_asm, "
	. "asm_end_$num_asm;\n";
    $asm_code .= "\{ \&asm_start_$num_asm, \&asm_end_$num_asm, $cstring \},\n";
    ++$num_asm;

    $cstring = &make_asm_string ($code, $num_asm, %magic_5);
    $asm_list .= "asm volatile ($cstring);\n";
    $asm_decls .= "extern unsigned char asm_start_$num_asm, "
	. "asm_end_$num_asm;\n";
    $asm_code .= "\{ \&asm_start_$num_asm, \&asm_end_$num_asm, $cstring \},\n";
    ++$num_asm;

    $asm_code .= "\{ NULL, NULL, NULL \},\n";
    ++$num_asm;

    close (SAMPLES);
}


# This processes an @setup command.
sub handle_setup {
    local ($name, $reg) = @_;
    local ($code) = "";

    # Concatenate all of the code lines together.
    while (length ($line = &next_line) && $line ne '@endsetup') {
	$code .= "$line\n";
    }

    # Store the setup information for this parameter.
    $setup_code{$name}  = $code;
    $setup_reg{$name}   = $reg;
    $param_magic{$name} = sprintf ("0x%lX", 0xABC12345 + ++$last_magic);
    $magic_a{$name}     = "0xAAAAAAAA";
    $magic_5{$name}     = "0x55555555";
}


sub handle_meta {
    local ($name) = @_;
    local ($template_code) = "";
    local ($lit_code) = "";
    local ($cmd);
    local ($param);

    # Concatenate all of the code lines together.
    while (length ($line = &next_line) && $line ne '@endmeta') {
	if ($line =~ /^\@lit/) {
	    $lit_code .= substr ($line, 4) . "\n";
	} else {
	    $template_code .= "$line\n";
	}
    }

    # Apply macros to the code.
    $go = 1;
    while ($go) {
	$go = 0;
	foreach $key (keys %macros) {
	    $go += ($lit_code =~ s/$key/$macros{$key}/gm);
	    $go += ($template_code =~ s/$key/$macros{$key}/gm);
	    $go += ($name =~ s/$key/$macros{$key}/gm);
	}
    }

    print STDERR "$name...";
    $literal_code{$name} = $lit_code;
    $raw_code{$name} = $template_code;

    # Crank out the necessary assembly templates.
    &create_asm_samples ($template_code, $name);
}


# Clean up.  No need to close INPUT here, since that was closed already.
close (ASM_OUT);
close (C_OUT);
