/*
 * server-doit.c --- real work done here
 * 2012-07-12 by si
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
#include <ctype.h>

#include <pthread.h>

pthread_mutex_t usertable_mutex = PTHREAD_MUTEX_INITIALIZER;

#include "msgbd.h"

void send_response (const int, const char *);

void server_doit (struct connection_info *c)
{
  ssize_t s;
  int error;
  char cmd;
  int i;
  char hbuf[NI_MAXHOST];
  char sbuf[NI_MAXSERV];
  char rbuf[MAXBUFLEN];

  void doit_post (const int, const char *, const char *);
  void doit_retrieve (const int);

  error = getnameinfo ((struct sockaddr *) &c->from, c->fromlen, hbuf, sizeof (hbuf),
		       sbuf, sizeof (sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
  if (error) {
    fprintf (stderr, "doit (): %s\n", gai_strerror (error));
    close (c->fd);
    return;
  }

  send_response (c->fd, RESPONSE_HELLO);
  printf ("*** %s connected\n", hbuf);

  memset (rbuf, 0, MAXBUFLEN);
  while ((s = read (c->fd, rbuf, MAXBUFLEN)) > 0) {
    //    printf ("[%s]\n", rbuf); fflush (stdout);
    sanitize (rbuf, s);
    //    printf ("[%s]\n", rbuf); fflush (stdout);

    cmd = rbuf[0];
    for (i = 1; isspace (rbuf[i]); i++) {
      if (rbuf[i] == '\0')
	break;
      //      printf ("[%c]\n", rbuf[i]);
    }
    switch (cmd) {
    case 'P':			/* post */
      //      printf ("%s\n", &rbuf[i]);
      doit_post (c->fd, hbuf, &rbuf[i]);
      break;
    case 'R':
      doit_retrieve (c->fd);
      break;
    case 'Q':			/* quit */
      printf ("*** %s quits\n", hbuf);
      send_response (c->fd, RESPONSE_BYE);
      close (c->fd);
      return;			/* no need for break */
    default:
      printf ("*** %s unknown command %c\n", hbuf, cmd);
      break;
    }

    memset (rbuf, 0, MAXBUFLEN);
  }
  close (c->fd);
  printf ("*** %s is gone\n", hbuf);
}

#define MAXNAMELEN 128
#define MAXMSGLEN (MAXBUFLEN)
#define MAXMSGNUM 100

struct msg_item {
  char poster_name[MAXNAMELEN];
  time_t time;
  char host[NI_MAXHOST];
  char msg[MAXMSGLEN];
} msg_table[MAXMSGNUM];
  
unsigned int msg_head = 0;

pthread_mutex_t msg_table_mutex = PTHREAD_MUTEX_INITIALIZER;

void doit_post (const int fd, const char hbuf[], const char *rbuf)
{
  time_t t;
  int s;
  char buf[MAXMSGLEN];

  send_response (fd, RESPONSE_POST_GOAHEAD);
  s = read (fd, buf, MAXMSGLEN);
  sanitize (buf, s);
  time (&t);
  pthread_mutex_lock (&msg_table_mutex);
  {
    msg_table[msg_head].time = t;
    strncpy (msg_table[msg_head].host, hbuf, NI_MAXHOST);
    strncpy (msg_table[msg_head].poster_name, rbuf, MAXNAMELEN);
    strncpy (msg_table[msg_head].msg, buf, MAXMSGLEN);

    printf ("*** %s @ %s posted new message at %s", msg_table[msg_head].poster_name,
	    msg_table[msg_head].host, ctime (&msg_table[msg_head].time));
    printf ("%s\n", msg_table[msg_head].msg);

    ++msg_head;
    msg_head %= MAXMSGNUM;
  }
  pthread_mutex_unlock (&msg_table_mutex);

  send_response (fd, RESPONSE_POST_OK);
}

void doit_retrieve (const int fd)
{
  unsigned int i;

  send_response (fd, RESPONSE_MESSAGE_FOLLOWS);
  pthread_mutex_lock (&msg_table_mutex);
  {
    char *s;
    unsigned int n;
    char buf[MAXBUFLEN];

    i = msg_head;
    n = 1;
    do {
      if (msg_table[i].host[0] != '\0') {
	sprintf (buf, "Message %d\r\n", n++);
	write (fd, buf, strlen (buf));
	write (fd, msg_table[i].poster_name, strlen (msg_table[i].poster_name));
	write (fd, "\r\n", 2);
	write (fd, msg_table[i].host, strlen (msg_table[i].host));
	write (fd, "\r\n", 2);
	s = ctime (&msg_table[i].time);
	write (fd, s, strlen (s) - 1);
	write (fd, "\r\n", 2);
	write (fd, msg_table[i].msg, strlen (msg_table[i].msg));
	write (fd, "\r\n", 2);
      }
      ++i;
      i %= MAXMSGNUM;
    } while (i != msg_head);
    send_response (fd, RESPONSE_MESSAGE_ENDS);
  }
  pthread_mutex_unlock (&msg_table_mutex);
}

void send_response (const int fd, const char *s)
{
  write (fd, s, strlen (s) + 1);
}

