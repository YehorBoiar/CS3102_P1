/********************************************************
simple UDP IPv4 chat with non-blocking i/o using fcntl()

saleem, Jan2024, Jan2022, Jan2021, Jan2004, Nov2002
Checked February 2025, 2026 (sjm55)
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
void perror(const char *s);

#include "UdpSocket.h"

#define G_MY_PORT   ((uint16_t) 54321) // use 'id -u', or getuid(2)
#define G_SIZE      ((uint32_t) 256)
#define G_SLEEPTIME ((uint32_t) 1) // seconds

#define ERROR(_s) fprintf(stderr, "%s\n", _s)

#define G_QUIT "quit"

int
readLine(int fd, UdpBuffer_t *buffer);

int
nonBlocking(int fd);

int
main(int argc, char *argv[])
{
  UdpSocket_t *local, *remote, receive;
  UdpBuffer_t buffer;
  uint8_t bytes[G_SIZE];
  int keyboard = fileno(stdin);
  int flag = 0;
  int r;

  if (argc != 2) {
    ERROR("usage: udp-chat-2 <hostname>");
    exit(0);
  }

  if ((local = setupUdpSocket_t((char *) 0, G_MY_PORT)) == (UdpSocket_t *) 0) {
    ERROR("local problem");
    exit(0);
  }

  if ((remote = setupUdpSocket_t(argv[1], G_MY_PORT)) == (UdpSocket_t *) 0) {
    ERROR("remote hostname/port problem");
    exit(0);
  }

  if (openUdp(local) < 0) {
    ERROR("openUdp() problem");
    exit(0);
  }

  /* set non-blocking i/o */
  if (nonBlocking(local->sd) < 0) {
    ERROR("nonBlocking(local->sd) problem");
    exit(0);
  }
  if (nonBlocking(keyboard) < 0) {
    ERROR("nonBlocking(keyboard) problem");
    exit(0);
  }

  while(!flag) {

    /* send any keyboard input */
    buffer.bytes = bytes;
    buffer.n = G_SIZE;
    if ((r = readLine(keyboard, &buffer)) > 0) {
      buffer.bytes[--buffer.n] = '\0'; /* stomp over the '\n' */
      if (strcmp((char *) buffer.bytes, G_QUIT) == 0) { flag = 1; }
      if (sendUdp(local, remote, &buffer) != buffer.n)
        ERROR("sendUdp() problem");
    }

    /* print any network input */
    buffer.bytes = bytes;
    buffer.n = G_SIZE;
    if ((r = recvUdp(local, &receive, &buffer)) < 0) {
      if (errno != EWOULDBLOCK)
        ERROR("recvUdp() problem");
    }
    else {
      bytes[r] = '\0';
      printf("-> %s\n", bytes);
    }

    (void) sleep(G_SLEEPTIME); // avoid hogging CPU in while() loop
  }

  closeUdp(local);
  closeUdp(remote);

  return 0;
}


int
readLine(int fd, UdpBuffer_t *buffer)
{
  int r = 0;

  if ((r = read(fd, (void *) buffer->bytes, buffer->n)) > 0) {
    buffer->n = r;
  } else if (errno == EWOULDBLOCK) {
    r = 0;
    buffer->n = 0;
  }
  else if (r < 0)
    perror("readLine(): read() problem");

  return r;
}

int
nonBlocking(int fd)
{
  int r, flags = O_NONBLOCK;

  if ((r = fcntl(fd, F_SETFL, flags)) < 0) {
    perror("setAsyncFd(): fcntl() problem");
    exit(0);
  }

  return r;
}
