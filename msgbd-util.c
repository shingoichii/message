/*
 * msgbd-util.c
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

#include "msgbd.h"

void sanitize (char *buf, const int s)
{
  char *ch;
  
  buf[s - 1] = '\0'; /* safeguard */
  ch = strrchr (buf, '\n');
  if (ch != NULL)
    *ch = '\0';
  ch = strrchr (buf, '\r');
  if (ch != NULL)
    *ch = '\0';
}
