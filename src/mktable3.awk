BEGIN {
    htod["0"] =  0; htod["1"] =  1; htod["2"] =  2; htod["3"] =  3;
    htod["4"] =  4; htod["5"] =  5; htod["6"] =  6; htod["7"] =  7;
    htod["8"] =  8; htod["9"] =  9; htod["A"] = 10; htod["B"] = 11;
    htod["C"] = 12; htod["D"] = 13; htod["E"] = 14; htod["F"] = 15;
    oldn = -1
    exceptions["GetMaskTable"] = 1
    exceptions["ScriptUtil"] = 1
    exceptions["SoundDispatch"] = 1
    exceptions["PaletteDispatch"] = 1
    exceptions["QDExtensions"] = 1
    exceptions["TEDispatch"] = 1
}

{
    a = substr($1, 3, 1)
    b = substr($1, 4, 1)
    c = substr($1, 5, 1)
    d = substr($1, 6, 1)
    n = htod[a] * 4096 + htod[b] *  256 + htod[c] *   16 + htod[d];
    if (oldn >= n) {
	print "bad entry -- exiting"
	exit(1)
    } else {
	while (++oldn < n)
	    print ".long	__UNKNOWN"
	if ($2 ~ /^_/ || $2 ~ /^Pack[0-9]/ || exceptions[$2] == 1)
	    print	".long	_" $2
	else
	    print	".long	_P_" $2
    }
}
