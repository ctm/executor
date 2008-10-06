#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


#define BLOCK_SIZE 512

/* Fills each sector with the two byte sector number, stored in big endian
 * byte order.  Specify the device you want to use on the command line,
 * or "-" for stdout.
 */
int
main (int argc, char *argv[])
{
  int sec, fd;

  if (argc != 2)
    {
      fprintf (stderr, "Usage: %s device\n", argv[0]);
      exit (-1);
    }

  if (!strcmp (argv[1], "-"))
    fd = fileno (stdout);
  else
    {
      fd = open (argv[1], O_WRONLY);
      if (fd < 0)
	{
	  perror (argv[1]);
	  exit (-2);
	}
    }

  for (sec = 0; ; sec++)
    {
      unsigned char buf[BLOCK_SIZE];
      int i;
      
      for (i = 0; i < BLOCK_SIZE; i += 2)
	{
	  buf[i] = (sec >> 8);
	  buf[i + 1] = sec;
	}

      if (write (fd, buf, sizeof buf) <= 0)
	break;
    }

  close (fd);

  printf ("%d sectors written (%.02f MB)\n",
	  sec, sec * BLOCK_SIZE / (1024.0 * 1024));

  return EXIT_SUCCESS;
}
