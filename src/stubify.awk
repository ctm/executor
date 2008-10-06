BEGIN	{ FS=","; OFS=","
	legitreadings["atrap"] = 1
	legitreadings["ptoc" ] = 1
	legitreadings["body" ] = 1
	}

NF == 1 { if (legitreadings[$1] == 1)
	      reading = $1
	  else
	      print
	}

$1 ~ /^0xA/	{
		    atrap[$2] = $1
		    for (i = 3 ; i <= NF ; i++) {
			atrap[$i] = $1
			comparetrap[$i] = $2
		    }
		}

NF == 2	{ if (reading == "ptoc")
		ptoc[$1] = $2
	  else if (reading != "atrap")
		print
	}

reading == "body" && NF > 2 {
	  split($3, x, ")")
	  split($1, y, "(")
	  fname = x[1]
	  if (ptoc[fname] != "") {
	      if (atrap[fname] == "")
		  atrap[fname] = "0"
	      $1 = "P" substr(y[1], 2, length(y[1])-1) "(" atrap[fname] ", " \
					  y[2]
	      if (comparetrap[fname]) {
		  split($1, y, "(")
		  $1 = "Q" substr(y[1], 2, length(y[1])-1) "(" \
						   comparetrap[fname] ", " y[2]
	      }
	  } else if ($1 ~ /^P/) {
	      $1 = "A" substr($1, 2, length($1)-1)
	  }
	  print
        }
