#!/usr/bin/perl -w

die "Usage: $0 nm_-n_output_file traceback_file\n"
    if ($#ARGV != 1);

print STDERR "Parsing nm file...";

&read_nm_file ($ARGV[0]);

print STDERR "done.\n";

&process_traceback ($ARGV[1]);


sub read_nm_file {
    local ($file) = @_;

    open (NM_FILE, "< $file") || die "Unable to open $file: $!\n";

    $last_addr = 0;
    
    while (<NM_FILE>)
    {
	if (($addr, $label) = /([0-9a-fA-F]+)\s+\w+\s+([^L\.]\w*)\s*$/)
	{
	    if ($label ne '___gnu_compiled_c')
	    {
		$new_addr = hex ($addr);
		die "fatal error: nm file isn't sorted!\n"
		    if ($new_addr < $last_addr);
		$last_addr = $new_addr;

		push (@known_labels, $new_addr);
		push (@known_labels, $label);
	    }
	}
    }

    # Put known sentinels at the end
    push (@known_labels, hex ("100000000"));
    push (@known_labels, '<UNKNOWN>');
    push (@known_labels, hex ("100000001"));
    push (@known_labels, '<UNKNOWN>');

    close (NM_FILE);
}


sub process_traceback
{
    local ($file) = @_;
    local ($line);

    open (TRACEBACK, "< $file") || die "Unable to open $file: $!\n";

    while (!eof (TRACEBACK))
    {
	# Skip any leading junk
	while (<TRACEBACK>)
	{
	    s/\r//g;
	    $line = $_;
	    print $line;
	    last if ($line =~ /at eip=/i);
	}
	
	# Cruise through the register dumps
	while (<TRACEBACK>)
	{
	    s/\r//g;
	    print;
	    last if (/frame/i);
	}

	# Now process the traceback
	while (<TRACEBACK>)
	{
	    s/\r//g;
	    $line = $_;
	    if (!(($addr_string) = /\s*0x([0-9a-fA-F]+)\s*$/))
	    {
		print $line;
		last;
	    }
	    $addr = hex ($addr_string);
	    
	    for ($i = 2; $i <= $#known_labels; $i += 2)
	    {
		if ($known_labels[$i] > $addr)
		{
		    print ("  0x$addr_string\t$known_labels[$i - 1]");
		    printf (" + 0x%x\n", $addr - $known_labels[$i - 2]);
		    last;
		}
	    }
	}
    }
    
    close (TRACEBACK);
}
