#!/usr/bin/perl

# This produces C code to translate from a 68k Pascal call to a C call.
# You give it as input a string whose first letter is the type of the
# function's return value, and whose subsequent letters are the types
# of the arguments.  Here are the legal letters:
#
#	v	void, only for return values
#	b	byte
#	w	word (16 bit)
#	l	long (32 bit)
#	p	Point (32 bit, but byte swapped differently)
#
# So "makeptocstub.pl wlbw" would generate C code to handle
# a Pascal call to a function such as:
#
#	INTEGER foo (LONGINT a, Byte b, INTEGER c);


$fmt = $ARGV[0];

die "Bogus character in format!\n" if ($fmt =~ /[^vbwlp]/);

@letters = split (//, $fmt);
$ret = shift (@letters);

$type{'v'} = 'void';
$type{'b'} = 'uint8';
$type{'w'} = 'uint16';
$type{'l'} = 'uint32';
$type{'p'} = 'uint32';

$size{'v'} = 0;
$size{'b'} = 2;		# not an error; all arguments are on even addresses
$size{'w'} = 2;
$size{'l'} = 4;
$size{'p'} = 4;

$swap{'v'} = "***ERROR***";
$swap{'b'} = "";
$swap{'w'} = "CW ";
$swap{'l'} = "CL ";
$swap{'p'} = "SWAP_POINT ";

$const = "const " if ($ret eq 'v');

print ("/\* This is machine-generated; DO NOT EDIT! \*/\n" .
       "syn68k_addr_t\n" .
       "call_ptoc_$fmt (syn68k_addr_t ignore_me, void \*func)\n" .
       "{\n" .
       "  $const" . "uint8 *a7 = ($const" . "uint8 *) EM_A7\n" .
       "  syn68k_addr_t retaddr = CL (*($const" . "uint32 *) a7);\n");

if ($ret ne 'v') {
    print "  $type{$ret} result;\n";
}

# Compute total size of arguments
$arg_size = 0;
for ($i = 0; $i <= $#letters; $i++) {
    $arg_size += $size{$letters[$i]};
}

# Bump up stack pointer
printf ("  a7 += %d;\n", $arg_size + 4); # skip return address
print "  EM_A7 = (syn68k_addr_t) a7;\n";

$cast = "(($type{$ret} \(\*\)\(";
$call = '(';

$offset = $arg_size;
while ($l = shift (@letters)) {
    if ($not_first) {
	$cast .= ', ';
	$call .= ', ';
    } else {
	$not_first = 1;
    }
    $cast .= $type{$l};
    $call .= "$swap{$l}(*($const$type{$l} *) (a7";

    if ($l eq 'b') {
	$sub = $offset - 1;
    } else {
	$sub = $offset;
    }

    $call .= " - $sub))";
    $offset -= $size{$l};
}

$cast .= ")) func)";
$call .= ');';

if ($ret ne 'v') {
    print ("  result = $cast $call\n");
    if ($ret eq 'b') {
	print "  *(uint16 *) a7 = CW ((uint16) result);\n";
    } else {
	print "  *($type{$ret} *) a7 = $swap{$ret}(result);\n";
    }
} else {
    print "  $cast $call\n";
}
print ("  return retaddr;\n" .
       "}\n");

