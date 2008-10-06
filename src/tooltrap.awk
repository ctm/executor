BEGIN {
    bits9and8["0"] = 0;
    bits9and8["1"] = 1;
    bits9and8["2"] = 2;
    bits9and8["3"] = 3;
    bits9and8["4"] = 0;
    bits9and8["5"] = 1;
    bits9and8["6"] = 2;
    bits9and8["7"] = 3;
    bits9and8["8"] = 0;
    bits9and8["9"] = 1;
    bits9and8["A"] = 2;
    bits9and8["B"] = 3;
    bits9and8["C"] = 0;
    bits9and8["D"] = 1;
    bits9and8["E"] = 2;
    bits9and8["F"] = 3;
    exceptions["HandToHand"] = 1
    exceptions["PtrToHand"] = 1
    exceptions["PtrToXHand"] = 1
    exceptions["HandAndHand"] = 1
    exceptions["PtrAndHand"] = 1
    exceptions["Date2Secs"] = 1
    exceptions["Secs2Date"] = 1
    exceptions["Enqueue"] = 1
    exceptions["Dequeue"] = 1
}

$1 !~ /^0x/ {
    table[$1] = $2
}

$1 ~ /^0x/ && NF >= 2 && substr($1, 4, 1) ~ /[89ABCDEF]/	{
    printf("0x0%0d%s ",  bits9and8[substr($1, 4, 1)], substr($1, 5, 2));
    if (exceptions[$2])
	printf("_")
    print $2
}
