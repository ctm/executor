# $Id: Makefile,v 2.1 1995/06/12 07:17:10 ctm Exp $

HFS_XFer.util:	HFS_XFer.util.c fs.h
	$(CC) -g -DNEXT -DNEXTSTEP -I../../../../include -O -object -arch m68k -arch i486 -o HFS_XFer.util HFS_XFer.util.c

%::	%,v
	$(CO) $(COFLAGS) $<

%::	RCS/%,v
	$(CO) $(COFLAGS) $<
