/*
 * server-skelton.c
 * basaed on server-getaddrinfo.c from itojun book
 * 2012-07-09 by si
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define MAXSOCK 20

#include <time.h>
#include <pthread.h>

#include "msgbd.h"

int
main (int argc, char **argv)
{
  struct addrinfo hints, *res, *res0;
  int error;
  struct sockaddr_storage from;
  socklen_t fromlen;
  int ls;
  int s[MAXSOCK];
  int smax;
  int sockmax;
  fd_set rfd, rfd0;
  int n;
  int i;
  char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
#ifdef IPV6_V6ONLY
  const int on = 1;
#endif

  if (argc != 2) {
    fprintf (stderr, "usage: test port\n");
    exit (1);
    /*NOTREACHED*/
  }

  memset (&hints, 0, sizeof (hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  error = getaddrinfo (NULL, argv[1], &hints, &res0);
  if (error) {
    fprintf (stderr, "%s: %s\n", argv[1], gai_strerror (error));
    exit (1);
    /*NOTREACHED*/
  }

  smax = 0;
  sockmax = -1;
  for (res = res0; res && smax < MAXSOCK; res = res->ai_next) {
    s[smax] = socket (res->ai_family, res->ai_socktype, res->ai_protocol);
    if (s[smax] < 0)
      continue;

    if (s[smax] >= FD_SETSIZE) {
      close (s[smax]);
      s[smax] = -1;
      continue;
    }

#ifdef IPV6_V6ONLY
    if (res->ai_family == AF_INET6 && setsockopt (s[smax], IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof (on)) < 0) {
      perror ("setsockopt");
      s[smax] = -1;
      continue;
    }
#endif

    if (bind (s[smax], res->ai_addr, res->ai_addrlen) < 0) {
      close (s[smax]);
      continue;
    }
    if (listen (s[smax], 5) < 0) {
      close (s[smax]);
      s[smax] = -1;
      continue;
    }

    error = getnameinfo (res->ai_addr, res->ai_addrlen, hbuf, sizeof (hbuf), sbuf, sizeof (sbuf),
			 NI_NUMERICHOST | NI_NUMERICSERV);
    if (error) {
      fprintf (stderr, "test: %s\n", gai_strerror (error));
      exit (1);
      /*NOTREACHED*/
    }
    fprintf (stderr, "listen to %s %s\n", hbuf, sbuf);

    if (s[smax] > sockmax)
      sockmax = s[smax];
    smax++;
  }

  if (smax == 0) {
    fprintf (stderr, "test: no socket to listen to\n");
    exit (1);
    /*NOTREACHED*/
  }

  FD_ZERO (&rfd0);
  for (i = 0; i < smax; i++)
    FD_SET (s[i], &rfd0);

  while (1) {
    rfd = rfd0;
    n = select (sockmax + 1, &rfd, NULL, NULL, NULL);
    if (n < 0) {
      perror ("select");
      exit (1);
      /*NOTREACHED*/
    }
    for (i = 0; i < smax; i++) {
      if (FD_ISSET (s[i], &rfd)) {
	fromlen = sizeof (from);
	ls = accept (s[i], (struct sockaddr *)&from, &fromlen);
	if (ls < 0)
	  continue;

	{
	  pthread_t tid;
	  struct connection_info *c = malloc (sizeof (struct connection_info));

	  c->fd = ls;
	  c->from = from;
	  c->fromlen = fromlen;

	  pthread_create (&tid, NULL, (void *) server_doit, c);
	}

      }
    }
  }

  /*NOTREACHED*/
}
