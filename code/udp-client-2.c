/********************************************************
simple UDP IPv4 client, using UdpSocket_t

saleem: Jan2024, Jan2022, Jan2021, Nov2002, Dec2001
Checked February 2025, 2026 (sjm55)
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>

#include "UdpSocket.h"

#define G_SRV_PORT ((uint16_t)24628) // use 'id -u' or getuid(2)
#define G_SIZE ((uint32_t)256)

#define ERROR(_s) fprintf(stderr, "%s\n", _s)

#define G_QUIT "quit"

int readLine(int fd, UdpBuffer_t *buffer);

int main(int argc, char *argv[])
{
  UdpSocket_t *local, *remote;
  UdpBuffer_t buffer;
  unsigned char bytes[G_SIZE];
  int flag = 0;

  if (argc != 2)
  {
    ERROR("usage: udp-client-2 <hostname>");
    exit(0);
  }

  if ((local = setupUdpSocket_t((char *)0, G_SRV_PORT)) == (UdpSocket_t *)0)
  {
    ERROR("local problem");
    exit(0);
  }

  if ((remote = setupUdpSocket_t(argv[1], G_SRV_PORT)) == (UdpSocket_t *)0)
  {
    ERROR("remote hostname/port problem");
    exit(0);
  }

  if (openUdp(local) < 0)
  {
    ERROR("openUdp() problem");
    exit(0);
  }

  while (!flag)
  {
    int r;

    buffer.bytes = bytes;
    buffer.n = G_SIZE;
    r = readLine(fileno(stdin), &buffer);
    buffer.bytes[r - 1] = '\0'; /* stomp over the '\n' */
    --buffer.n;                 /* stomp over the '\n' */
    if ((buffer.n > 0) && (sendUdp(local, remote, &buffer) != buffer.n))
      ERROR("sendUdp() problem");
    if (strcmp((char *)buffer.bytes, G_QUIT) == 0)
      flag = 1;
  }

  closeUdp(local);
  closeUdp(remote);

  return 0;
}

int readLine(int fd, UdpBuffer_t *buffer)
{
  int r = 0;

  r = read(fd, (void *)buffer->bytes, buffer->n);
  buffer->n = r > 0 ? r : 0;

  if (r < 0)
    ERROR("readline(): problem with read()");

  return r;
}
