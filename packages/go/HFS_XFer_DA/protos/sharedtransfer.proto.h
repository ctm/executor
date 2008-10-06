
/* sharedtransfer.c */
long ischild (long dir1, long dir2, short volume);
OSErr CopyFork (forktype fork, StringPtr srcname, StringPtr destname,
		long fromid, long toid, short srcvrn, short dstvrn);
short copy1file (short srcvrn, short dstvrn, long srcdirid, long dstdirid,
		 Str255 s, BOOLEAN doit);
short move1file (short srcvrn, short dstvrn, long srcdirid, long dstdirid,
		 Str255 s, BOOLEAN doit);
short commontrans (long fromd, long tod, short fromv, short tov, StringPtr s,
		   short (*fp) (short, short, long, long, Str255, char));
short duplicate1file (short srcvrn, short dstvrn, long srcdirid,
		 long dstdirid, Str255 name, Str255 destname, BOOLEAN doit);
