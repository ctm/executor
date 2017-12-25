# /bin/bash



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
