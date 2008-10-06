substr($1, 4, 1) ~ /[0-7]/ && NF >= 2 	{ print "0x00" substr($1, 5, 2), $2 }
