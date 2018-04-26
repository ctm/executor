#!/usr/bin/perl



# This is how many times to unwrap the blitting loop
$max_unwrap_factor = 8;

print "/* This file is machine-generated; DO NOT EDIT! */\n\n";

print ("\#define JUMP_TO_NEXT                        \\\n" .
       "do \{                                        \\\n" .
       "  const void *next = s\[1\].label;            \\\n" .
       "  NEXT_ROW;				    \\\n" .
       "  arg = s\[1\].arg;                           \\\n" .
       "  s = (const blt_section_t *) ((const uint8_t *) s + sec_size); \\\n" .
       "  goto *next;                               \\\n" .
       "} while (0)\n" .
       "\n" .
       "/* Maximum number of times we'll unwrap any blitter loops. */\n" .
       "\#define MAX_LOOP_UNWRAP	$max_unwrap_factor" . "U\n" .
       "\#define REPEAT_MOD_0_STUB	0U\n" .
       "\#define MASK_STUB		(REPEAT_MOD_0_STUB + MAX_LOOP_UNWRAP)\n" .
       "\#define DONE_STUB		(MASK_STUB + 1)\n" .
       "\#define FUNC_PTR		(DONE_STUB + 1)\n");

$func_number = 0;
$func_empty = 1;

while (length ($line = &next_line)) {
    @cmd = split (/\s+/, $line);
    if ($cmd[0] eq 'begin_pat_func' || $cmd[0] eq 'begin_src_func') {
	&output_func;
	&reset_func;

	$patblt = ($cmd[0] eq 'begin_pat_func');
	$prefix = $patblt ? "xdblt" : "srcblt";

	$init = "";
	$inloop = "";
	while (length ($line = &next_line) && $line ne 'end_func') {
	    @cmd = split (/\s+/, $line);
	    ($rest) = ($line =~ /^\s*\w+\s+(.*)/);
	    $rest .= "\n";
	    if ($cmd[0] eq 'init') {
		$init .= $rest;
	    } elsif ($cmd[0] eq 'inloop') {
		$inloop .= $rest;
	    } else {
		die "Unknown func command $cmd[0] on line $lineno.\n";
	    }
	}
    } elsif ($cmd[0] eq 'begin_mode') {
	$mode_name = $cmd[1];

	$unwrap_factor = $cmd[2];
	if ($unwrap_factor eq 'max_unwrap') {
	    $unwrap_factor = $max_unwrap_factor;
	} elsif ($unwrap_factor < 1
		 || $unwrap_factor > $max_unwrap_factor
		 || $max_unwrap_factor % $unwrap_factor) {
	    die "Illegal unwrap factor on line $lineno.\n";
	}

	if ($unwrap_factor != $max_unwrap_factor) {
	    die ("unwrap factor must equal max_unwrap for now...the problem " .
		 "is with how much to offset dst by for the initial cruft..." .
		 "it would vary with the unwrap factor.\n");
	}

	$repeat = "";
	$mask = "";

	while (length ($line = &next_line) && $line ne 'end_mode') {
	    @cmd = split (/\s+/, $line);
	    ($rest) = ($line =~ /^\s*\w+\s+(.*)/);
	    $rest .= "\n";
	    if ($cmd[0] eq 'repeat') {
		$repeat .= $rest;
	    } elsif ($cmd[0] eq 'mask') {
		$mask .= $rest;
	    } else {
		die "Unknown mode command $cmd[0] on line $lineno.\n";
	    }
	}
	&process_mode ($mode_name, $repeat, $mask);
    }
}

# Handle any leftovers
&output_func;


sub output_func {
    return if ($func_empty);

    print ("\n" .
	   "\n" .
	   "extern const void \*$label_array_name\[$label_array_index\] " .
	   "asm (\"_$label_array_name\");\n" .
	   "\n" .
	   "const void * const *\n" .
	   "$prefix" . "_func_$func_number (const blt_section_t *section,\n");

    if ($patblt) {
	print "uint8_t *row_base, long num_rows, long y)\n";
    } else {
	print ("const uint8_t *src_row_base, uint8_t *dst_row_base, " .
	       "long num_rows)\n");
    }
	   
    print ("\{\n" .
	   $label_arrays .
	   "\};\n" .
	   "\#if defined (mc68000)\n" .
	   "/* Convince gcc to use one register addl, not two addqw's */\n" .
	   "register uint32_t sec_size = sizeof (section[0]);\n" .
	   "\#elif !defined (sec_size)\n" .
	   "#define sec_size (sizeof (section[0]))\n" .
	   "\#endif\n" .
	   $init .
	   "  do \{\n" .
	   $inloop .
	   "  const blt_section_t *s = section;\n" .
	   "  int32_t arg = s->arg;\n");

	   if ($patblt) {
	       print "uint32_t *dst = (uint32_t *) \&row_base\[s->offset\];\n";
	   } else {
	       print ("const uint32_t *src = (const uint32_t *) " .
		      "\&src_row_base\[s->offset\];\n" .
		      "uint32_t *dst = (uint32_t *) " .
		      "\&dst_row_base\[s->offset\];\n");
	   }

    print ("  goto *(s->label);\n" .
	   "\n" .
	   "$code" .
	   " done:\n");

    if ($patblt) {
	print "  row_base += $prefix" . "_dst_row_bytes;\n";
    } else {
	print "  src_row_base += $prefix" . "_src_row_stride;\n";
	print "  dst_row_base += $prefix" . "_dst_row_stride;\n";
    }

    print ("  \} while (--num_rows > 0);\n" .
	   "  return labels;  /* Let gcc know labels are used. */\n" .
	   "\}\n");

    ++$func_number;
}

sub reset_func {
    $code = "";
    $label_array_name = "$prefix" . "_func_$func_number" . "_labels";
    $label_arrays = ("static const void *labels\[\] " .
		     "asm (\"_$label_array_name\") = \{\n");
    $label_array_index = 0;
    $func_empty = 1;

    undef %mask_labels_seen;
    undef %repeat_labels_seen;
}


sub process_mode {
    local ($tag, $repeat, $mask) = @_;
    local ($l);

    if (defined ($mask_labels_seen{$mask})) {
	$mask_label = $mask_labels_seen{$mask};
    } else {
	$mask_label = "$tag" . "_mask";
	$mask_labels_seen{$mask} = $mask_label;
	($new = $mask) =~ s/\@dst\@/\*dst/g;
	$new =~ s/\@src\@/\*src/g;
	$new =~ s/\@src_plus_1\@/src\[1\]/g;
	$code .= ("$mask_label:\n" .
		  "$new" .
		  "JUMP_TO_NEXT;\n");
    }

    if (defined ($repeat_labels_seen{$repeat})) {
	$repeat_label = $repeat_labels_seen{$repeat};
    } else {
	$repeat_label = $tag . "_many";
	$repeat_labels_seen{$repeat} = $repeat_label;

	$loop = (" $repeat_label" . "_loop:\n" .
		 "\#if !defined (i386)\n");
	if (!$patblt) {
	    $loop .= "src += $unwrap_factor;\n";
	}
	$loop .= ("  dst += $unwrap_factor;\n" .
		  "\#endif\n");

	for ($i = 0; $i < $unwrap_factor; $i++) {
	    $loop .= "$repeat_label" . "_" . ($max_unwrap_factor - $i) . ":\n";
	    ($new = $repeat) =~ s/\@dst\@/dst\[$i\]/g;
	    $new =~ s/\@src\@/src\[$i\]/g;
	    $p1 = $i + 1;
	    $new =~ s/\@src_plus_1\@/src\[$p1\]/g;

	    $loop .= "  $new";
	}

	$loop .= "\#if defined (i386)\n";
	if (!$patblt) {
	    $loop .= "src += $unwrap_factor; /* Avoids AGI stall here. */\n";
	}
	$loop .= ("  dst += $unwrap_factor; /* Avoids AGI stall here. */\n" .
		  "\#endif\n" .
		  "  if ((arg -= $unwrap_factor) > 0)\n" .
		  "    goto $repeat_label" . "_loop;\n" .
		  "  JUMP_TO_NEXT;\n");

	$code .= $loop;
    }

    $this_label_array = "";
    for ($i = 0; $i < $max_unwrap_factor; $i++) {
	$this_label_array .= ("\&\&$repeat_label" . "_" .
			      ((($i - 1) & ($unwrap_factor - 1)) + 1) . ",\n");
    }

    $this_label_array .= "   \&\&$mask_label,\n";
    $this_label_array .= "   \&\&done,\n";
    $this_label_array .= "  (void*) $prefix" . "_func_$func_number,\n";

    if (defined ($label_arrays_seen{$this_label_array})) {
	$index = $label_arrays_seen{$this_label_array};
    } else {
	$index = $label_array_index;
	$label_arrays_seen{$this_label_array} = $index;
	$label_array_index += $max_unwrap_factor + 3;
	$label_arrays .= $this_label_array;
    }
    
    $label_arrays .= ("\#define $prefix" . "_$tag" . "_labels " .
		      "(\&$label_array_name\[$index\])\n");
    $func_empty = 0;
}


# Returns the next non-empty line after stripping out comments, or the
# empty string if there are no more lines.
sub next_line {
    local ($line);
    while (<>) {
	++$lineno;
	($line) = /(^[^\#]*)/;	# We begin comments with '#' characters
	$line =~ s/\s+$//;		# Strip trailing whitespace
	$line =~ s/^\s+//;		# Strip leading whitespace
	return $line if (length ($line));
    }

    "";
}
