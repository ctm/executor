#! /bin/awk -f
BEGIN {
  FS=",";
  print "extern uint32 ROMlib_offset;";
}

$1 ~/^DATA/ {
  gsub("DATA[(]", "", $1);
  gsub("[ 	]", "", $2);
  gsub("[ 	]", "", $4);
  if ($3 == "")
    star = "*";
  else
    star = "(*)" $3;
  print "#define " $2 " (*(" $1 " " star ")(" $4 " + ROMlib_offset))";
}
