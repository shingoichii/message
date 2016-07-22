/*
 * client-skelton.c
 * based on client-getaddrinfo.c from itojun book
 * 2012-07-12 by si
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int client_doit (const int fd);

int
main (int argc, char **argv)
{
  struct addrinfo hints, *res, *res0;
  int s;
  char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
  int error;

  if (argc != 3) {
    fprintf (stderr, "usage: test host port\n");
    exit (1);
    /*NOTREACHED*/
  }

  memset (&hints, 0, sizeof (hints));
  hints.ai_socktype = SOCK_STREAM;
  error = getaddrinfo (argv[1], argv[2], &hints, &res0);
  if (error) {
    fprintf (stderr, "%s %s: %s\n", argv[1], argv[2], gai_strerror (error));
    exit (1);
    /*NOTREACHED*/
  }

  for (res = res0; res; res = res->ai_next) {
    error = getnameinfo (res->ai_addr, res->ai_addrlen, hbuf, sizeof (hbuf), sbuf, sizeof (sbuf),
			 NI_NUMERICHOST | NI_NUMERICSERV);
    if (error) {
      fprintf (stderr, "%s %s: %s\n", argv[1], argv[2], gai_strerror (error));
      continue;
    }
    fprintf (stderr, "trying %s port %s\n", hbuf, sbuf);

    s = socket (res->ai_family, res->ai_socktype, res->ai_protocol);
    if (s < 0)
      continue;

    if (connect (s, res->ai_addr, res->ai_addrlen) < 0) {
      close (s);
      s = -1;
      continue;
    }

    //    while ((l = read (s, buf, sizeof (buf))) > 0)
    //      write (STDOUT_FILENO, buf, l);
    //    close (s);

    return (client_doit (s));

    //    exit (0);
    /*NOTREACHED*/
  }

  fprintf (stderr, "test: no destination to connect to\n");
  exit (1);
  /*NOTREACHED*/
}
