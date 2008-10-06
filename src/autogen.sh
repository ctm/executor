# /bin/bash

# $Id: autogen.sh 63 2004-12-24 18:19:43Z ctm $

set -o errexit -o nounset -o noclobber

main ()
{
  aclocal
  autoheader
  automake --add-missing -Wall
  autoconf
  echo "Now you're ready to run ./configure"
}

main "$@"
