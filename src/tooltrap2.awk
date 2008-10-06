BEGIN	{
	exceptions["LoadSeg"] = 1
	exceptions["Pack0"] = 1
	exceptions["Pack2"] = 1
	exceptions["Pack3"] = 1
	exceptions["Pack4"] = 1
	exceptions["Pack5"] = 1
	exceptions["Pack6"] = 1
	exceptions["Pack7"] = 1
	exceptions["Pack12"] = 1
	exceptions["Pack14"] = 1

	exceptions["HandToHand"] = 1
	exceptions["PtrToHand"] = 1
	exceptions["PtrToXHand"] = 1
	exceptions["HandAndHand"] = 1
	exceptions["PtrAndHand"] = 1
	exceptions["Date2Secs"] = 1
	exceptions["Secs2Date"] = 1
	exceptions["Enqueue"] = 1
	exceptions["Dequeue"] = 1

	exceptions["SoundDispatch"] = 1
	exceptions["QDExtensions"] = 1
	exceptions["WackyQD32Trap"] = 1
	exceptions["PaletteDispatch"] = 1

	exceptions["_Fix2X"] = 1
	exceptions["_Frac2X"] = 1
	exceptions["_PrGlue"] = 1
	exceptions["R_X2Fix"] = 1
	exceptions["R_X2Frac"] = 1
	exceptions["TEDispatch"] = 1
	exceptions["ScriptUtil"] = 1
	exceptions["__GetResource"] = 1

	exceptions["GetMaskTable"] = 1
}

$1 !~ /^0x/ {
    table[substr($2, 6, length($2)-5)] = $3
}

$1 ~ /^0x/ && NF >= 2 && substr($1, 4, 1) ~ /[89ABCDEF]/ {
    if (!exceptions[$2]) {
	if ((t = table[$2]) == "") {
	    if ($2 ~ /^Pack[1-9]/)
		prefix = ""
	    else
		prefix = "_P"
	    print ".globl " prefix "_" $2
	    print prefix "_" $2 ":"
	    print ".word 0x4AFC"
	}
    }
}
