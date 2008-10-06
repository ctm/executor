BEGIN {
    bits9and8["0"] = "0";
    bits9and8["1"] = "1";
    bits9and8["2"] = "0";
    bits9and8["3"] = "1";
    bits9and8["4"] = "0";
    bits9and8["5"] = "1";
    bits9and8["6"] = "0";
    bits9and8["7"] = "1";

    bits9and8["8"] = "8";
    bits9and8["9"] = "9";
    bits9and8["A"] = "A";
    bits9and8["B"] = "B";
    bits9and8["C"] = "8";
    bits9and8["D"] = "9";
    bits9and8["E"] = "A";
    bits9and8["F"] = "B";
}

NF == 2 {
    printf("0xA%s%s ",  bits9and8[substr($1, 4, 1)], substr($1, 5, 2));
    print $2
}
