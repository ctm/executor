#!/usr/bin/perl

# useage:
# subst.pl pattern:file ...

while (<STDIN>)
  {
    @args = @ARGV;

    foreach $arg (@args)
      {
	($current_pattern, $current_file) = split (':', $arg);
	if (/$current_pattern/)
	  {
	    open (F, $current_file) || die $!;
	    while (<F>)
	      {
		print $_;
	      }
	    close (F);
	    goto end_line;
	  }
      }
    print $_;
   end_line:
  }
     
