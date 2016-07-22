/*
 * client-doit.c --- simple sample for KS I 2012
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

#include "msgbd.h"

int expect_response (const int, const char *);
int giveup (const char *);
void prompt (void);

void doit_post (const int, char *);
void doit_retrieve (const int);
int doit_quit (const int);

char *next_line (const int fd)
{
  static char *rbuf = NULL;	/* read buffer; NOT thread safe! */
  static unsigned int i = 0;	/* where we are in the read buffer */
  static char *wbuf;		/* work buffer to construct returning line */
  static char *w;		/* where we are in the work buffer */
  static int s = 0;

  if (rbuf == NULL) {
    /* initialize */
    rbuf = (char *) calloc (MAXBUFLEN, sizeof (char));
    wbuf = (char *) calloc (MAXBUFLEN * 2, sizeof (char));
    w = wbuf;
  }

 again:
  if (i >= s) {
    s = read (fd, rbuf, MAXBUFLEN);
    //    printf ("<<%d>>", s);fflush (stdout);
    if (s <= 0) {
      /* eof detected */
      return NULL;
    }
    i = 0;
  }

  switch (rbuf[i]) {
  case '\r':
  case '\0':
    //    putchar ('=');fflush (stdout);
    ++i;
    break;
  case '\n':
    /* eof detected */
    //    putchar ('_');fflush (stdout);
    *w = '\0';
    w = wbuf;
    ++i;
    return wbuf;
  default:
    //    putchar (rbuf[i]);fflush (stdout);
    *w++ = rbuf[i++];
    break;
  }
  goto again;
}

int client_doit (const int fd)
{
  char ubuf[MAXBUFLEN];
  char *c;

  if (expect_response (fd, RESPONSE_HELLO)) {
    close (fd);
    return giveup ("service rejected");
  }

  while (prompt (), fgets (ubuf, MAXBUFLEN, stdin) != NULL) {
    for (c = ubuf; c != NULL && *c != '\0' && isspace (*c); ++c)
      ;
    if (c == NULL || *c == '\0')
      continue;
    switch (toupper (*c)) {
    case 'P':
      doit_post (fd, ubuf);
      break;
    case 'R':
      doit_retrieve (fd);
      break;
    case 'Q':
      return doit_quit (fd);
    case 'H':
    default:
      printf ("P username ... post new message\n");
      printf ("R          ... read posted messages\n");
      printf ("Q          ... quit\n");
      printf ("H          ... help\n");
      break;
    }
  }
  /* ^D is pressed */
  return doit_quit (fd);
}

void prompt (void)
{
  printf ("command> ");
}

int expect_response (const int fd, const char m[])
{
  char *buf;

  buf = next_line (fd);
  if (buf == NULL)
    return -1;

  //  printf ("%c %c %c <> %c %c %c\n", buf[0], buf[1], buf[2], m[0], m[1], m[2]);
  if ((buf[0] == m[0]) && (buf[1] == m[1]) && (buf[2] == m[2]))
    return 0;
  else
    return -1;
}

int giveup (const char *s)
{
  fprintf (stderr, "!!! server is not working properly: %s\n", s);
  return 101;
}

int doit_quit (const int fd)
{
  int res = 0;

  write (fd, "Q\r\n", 3);
  if (expect_response (fd, RESPONSE_BYE)) {
    res = giveup ("adieu");
  }
  close (fd);
  return res;
}

void doit_retrieve (const int fd)
{
  char *b;
  int st = 0;
  
  write (fd, "R\r\n", 3);
  if (expect_response (fd, RESPONSE_MESSAGE_FOLLOWS)) {
    giveup ("retrieve not allowed"); /* return value not used */
    return;
  }

  while ((b = next_line (fd)) != NULL) {
    if (strncmp (b, RESPONSE_MESSAGE_ENDS, strlen (RESPONSE_MESSAGE_ENDS) - 2) == 0)
      return;
    if (strncmp (b, "Message", strlen ("Message")) == 0)
      printf ("[Message #%s]", b + strlen ("Message") + 1);
    else
      switch (st) {
      case 0:
	printf (" from %s", b);
	st = 1;
	break;
      case 1:
	printf (" @ %s", b);
	st = 2;
	break;
      case 2:
	printf (" at %s\n", b);
	st = 3;
	break;
      default:
	printf ("  %s\n", b);
	st = 0;
	break;
      }
  }
}

void doit_post (const int fd, char *ubuf)
{
  char *c;

  c = strchr (ubuf, '\n');
  *c++ = '\r';
  *c++ = '\n';
  *c = '\0';

  write (fd, ubuf, strlen (ubuf));
  
  if (expect_response (fd, RESPONSE_POST_GOAHEAD)) {
    giveup ("post not allowed");
    return;
  }

  printf ("message> ");
  if (fgets (ubuf, MAXBUFLEN - 1, stdin) == NULL)
    write (fd, "\r\n", 2);
  else {
    int s = strlen (ubuf);
    ubuf[s - 1] = '\r';
    ubuf[s] = '\n';
    ubuf[s + 1] = '\0';
    write (fd, ubuf, s + 1);
  }
  if (expect_response (fd, RESPONSE_POST_OK)) {
    giveup ("post failed");
    return;
  }

}
