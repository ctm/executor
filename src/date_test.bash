#! /bin/bash

set -o errexit -o nounset -o noclobber

# NOTE: this script creates a test program but doesn't actually compile
#       it into an executable or run it.

function main ()
{
  local -r sed_file="/tmp/foo.$$.sed"
  local -r c_file="/tmp/foo.$$.c"

cat << 'EOF' > "${sed_file}"
$a\
\
enum\
  {\
    OFFSET = 66 * N_SECS_IN_YEAR + 17 * N_SECS_IN_DAY\
  };\
\
\
PRIVATE void\
field_complain (long long mac_seconds, long long unix_seconds,\
		const char *field1, int value1,\
		const char *field2, int value2)\
{\
  fprintf (stderr, "mac_seconds = %lld, unix_seconds = %lld, "\
	   "%s = %d, %s = %d\n", mac_seconds, unix_seconds,\
	   field1, value1, field2, value2);\
  exit (1);\
}\
\
#define FIELD_COMPARE(var, field)					\\\
do							\\\
  {							\\\
    if (var != tm.field)				\\\
      field_complain (mac_seconds, unix_seconds,	\\\
                      #var, var, #field, tm.field);	\\\
  }							\\\
while (0)\
\
int\
main (void)\
{\
  long long unix_seconds;\
  long long mac_seconds;\
\
  for (mac_seconds = 0, unix_seconds = mac_seconds - OFFSET;\
       mac_seconds < (1ULL << 32);\
       ++mac_seconds, ++unix_seconds)\
    {\
      INTEGER year;\
      INTEGER month;\
      INTEGER day;\
      INTEGER hour;\
      INTEGER minute;\
      INTEGER second;\
      INTEGER dayofweek;\
      INTEGER dayofyear;\
      INTEGER weekofyear;\
      time_t t;\
      struct tm tm;\
\
      date_to_swapped_fields (mac_seconds,\
			      &year, &month, &day, &hour, &minute, &second,\
			      &dayofweek, &dayofyear, &weekofyear);\
      t = unix_seconds;\
      if (!gmtime_r (&t, &tm))\
	{\
	  fprintf (stderr, "bad time %ld(%lld)\n", t, unix_seconds);\
	  exit (1);\
	}\
      tm.tm_year += 1900;\
      FIELD_COMPARE (year, tm_year);\
\
      tm.tm_mon += 1;\
      FIELD_COMPARE (month, tm_mon);\
\
      FIELD_COMPARE (day, tm_mday);\
\
      FIELD_COMPARE (hour, tm_hour);\
\
      FIELD_COMPARE (minute, tm_min);\
\
      FIELD_COMPARE (second, tm_sec);\
\
      tm.tm_wday += 1;\
      FIELD_COMPARE (dayofweek, tm_wday);\
\
      tm.tm_yday += 1;\
      FIELD_COMPARE (dayofyear, tm_yday);\
    }\
\
  return 0;\
}\

1i\
#include <stdio.h>\
#include <time.h>\
#include <stdlib.h>\
\
#define PRIVATE static\
#define PUBLIC\
#define NULL_STRING ""\
\
typedef unsigned int ULONGINT;\
typedef short INTEGER;\
typedef int LONGINT;\
typedef int BOOLEAN;\
\
enum\
  {\
    FALSE,\
    TRUE\
  };\
\
#define A1(visibility, type, name, type0, arg0) \\\
visibility type name (type0 arg0)\
\
#define warning_unexpected(fmt, args...) fprintf (stderr, "UNEXPECTED " fmt, ## args);\
#define CW(x) (x)\
#define warning_unimplemented(fmt, args...) fprintf (stderr, "UNIMPLEMENTED " fmt, ## args);
/beginning of code to test/,/end of code to test/!d
EOF
  sed -f "${sed_file}" osutil.c > "${c_file}"
  rm "${sed_file}"
  gcc -O2 -Wall -c "${c_file}"
#  rm "${c_file}"
}

main "$@"
