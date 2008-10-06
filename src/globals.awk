BEGIN	{ FS=","; printf(".text\n.space 0x10000\n") }
$1 ~ /^DATA/ && NF==7	{ print ".set _" $2 ", " $4 ; print ".globl _" $2 }
