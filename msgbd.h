/*
 * msgbd.h --- header for sample message board
 * 2012-07-12 by si
 */

#define MAXBUFLEN 1024

#define RESPONSE_HELLO "201 HELLO\r\n"
#define RESPONSE_POST_OK "202 POST OK\r\n"
#define RESPONSE_MESSAGE_ENDS "203 MESSAGE ENDS\r\n"
#define RESPONSE_BYE "209 BYE\r\n"
#define RESPONSE_POST_GOAHEAD "301 POST GO AHEAD\r\n"
#define RESPONSE_MESSAGE_FOLLOWS "302 MESSAGE FOLLOWS\r\n"

struct connection_info {
  int fd;
  struct sockaddr_storage from;
  socklen_t fromlen;
};

void server_doit (struct connection_info *); /* real work done in this */
int client_doit (const int);

void sanitize (char *buf, const int s);

