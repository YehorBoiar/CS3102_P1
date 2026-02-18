/********************************************************
simple UDP IPv4 chat, using pselect() to detect input

saleem: Jan2024, Jan2022, Jan2021, Jan2004, Nov2002
Checked February 2025, 2026 (sjm55)
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <sys/time.h>
#include <unistd.h>

#include "UdpSocket.h"

#include <errno.h>
void perror(const char *s);

#define G_MY_PORT ((uint16_t) 54321) // use 'id -u', or getuid(2)
#define G_SIZE    ((uint32_t) 256)

#define G_QUIT "quit"

int
readLine(int fd, UdpBuffer_t *buffer);

int
main(int argc, char *argv[])
{
  UdpSocket_t *local, *remote, receive;
  UdpBuffer_t buffer;
  uint8_t bytes[G_SIZE];
  int keyboard = fileno(stdin);
  int flag = 0;
  fd_set readfds;
  int nfds;
  int r;

  if (argc != 2) {
    perror("usage: udp-chat-1 <hostname>");
    exit(0);
  }

  if ((local = setupUdpSocket_t((char *) 0, G_MY_PORT)) == (UdpSocket_t *) 0) {
    perror("local problem");
    exit(0);
  }

  if ((remote = setupUdpSocket_t(argv[1], G_MY_PORT)) == (UdpSocket_t *) 0) {
    perror("remote hostname/port problem");
    exit(0);
  }

  if (openUdp(local) < 0) {
    perror("openUdp() problem");
    exit(0);
  }

  nfds = local->sd + 1; /* local->sd > keyboard */
  while(!flag) {

    FD_ZERO(&readfds);
    FD_SET(keyboard, &readfds);
    FD_SET(local->sd, &readfds);

    /* blocking pselect() */
    if (pselect(nfds, &readfds, (fd_set *) 0, (fd_set *) 0,
               (struct timespec *) 0, (sigset_t *) 0) < 0) {
      perror("pselect() problem");
      exit(0);
    }

    /* send any keyboard input */
    if (FD_ISSET(keyboard, &readfds)) {

      buffer.bytes = bytes;
      buffer.n = G_SIZE;
      r = readLine(keyboard, &buffer);
      buffer.bytes[r-1] = '\0'; /* stomp over the '\n' */
      --buffer.n;               /* stomp over the '\n' */
      if ((buffer.n > 0) && (sendUdp(local, remote, &buffer) != buffer.n))
        perror("sendUdp() problem");
      if (strcmp((char *) buffer.bytes, G_QUIT) == 0)
        flag = 1;
     }

    /* print any network input */
    if (FD_ISSET(local->sd, &readfds)) {
      buffer.bytes = bytes;
      buffer.n = G_SIZE;
      if ((r = recvUdp(local, &receive, &buffer)) < 0)
        perror("recvUdp()");
      else {
        bytes[r] = (unsigned char) 0;
        printf("-> %s\n", bytes);
      }
    }
  }

  closeUdp(local);
  closeUdp(remote);

  return 0;
}

int
readLine(int fd, UdpBuffer_t *buffer)
{
  int r = 0;

  if ((r = read(fd, (void *) buffer->bytes, buffer->n)) < 0) {
    perror("readLine(): read()");
    buffer->n = 0;
  }
  else
    buffer->n = r;

  return r;
}
