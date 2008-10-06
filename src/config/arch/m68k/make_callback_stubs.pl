#!/usr/bin/perl

$max_callbacks = $ARGV[0];
$handler = $ARGV[1];
$stubs= $ARGV[2];

open (HANDLER, "<$handler") || die "Unable to open handler.\n";
open (STUBS, ">$stubs") || die "Unable to open output file.\n";

die "Too many callbacks for 16-bit offsets\n"
    if (($max_callbacks - 1) * 4 > 32767);

printf STUBS (".data\n" .
	      "\t.align 2\n" .
	      ".comm _callback_data,%d\n" .
	      ".text\n" .
	      "\t.align 2\n" .
	      ".globl _callback_stubs\n" .
	      "_callback_stubs:\n",
	      $max_callbacks * 8);

# Output a bunch of 4-byte BSR directives (w/precisely 2 byte operands)
for ($i = 0; $i < $max_callbacks; $i++) {
    printf STUBS ("\t.word 0x6100,0x%X\n", ($max_callbacks - $i) * 4 - 2);
}

while (<HANDLER>) {
    s/\s*\;.*$//g;
    print STUBS if (!/^\s*$/);
}

close (HANDLER);
close (STUBS);
