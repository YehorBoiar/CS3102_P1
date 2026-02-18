/********************************************************
simple UDP IPv4 server using UdpSocket_t

saleem: Jan2024, Jan2022, Jan2021, Nov2002, Dec2001
Checked February 2025, 2026 (sjm55)
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "UdpSocket.h"

#define G_SRV_PORT ((uint16_t)54321) // use 'id -u' or getuid(2)
#define G_SIZE ((uint32_t)256)

#define ERROR(_s) fprintf(stderr, "%s\n", _s)

#define G_QUIT "quit"

int main()
{
  UdpSocket_t *local, remote;
  UdpBuffer_t buffer;
  int flag = 0;
  unsigned char bytes[G_SIZE];
  int r;

  if ((local = setupUdpSocket_t((char *)0, G_SRV_PORT)) == (UdpSocket_t *)0)
  {
    ERROR("local problem");
    exit(0);
  }

  if (openUdp(local) < 0)
  {
    ERROR("openUdp() problem");
    exit(0);
  }
  printf("ready on port %d ...\n\n", ntohs(local->addr.sin_port));

  buffer.bytes = bytes;
  buffer.n = G_SIZE;
  while (!flag)
  {
    if ((r = recvUdp(local, &remote, &buffer)) < 0)
      ERROR("recvUdp() problem");
    else
    {
      bytes[r] = (unsigned char)0;
      printf("%s\n", bytes);
      if (strcmp((char *)bytes, G_QUIT) == 0)
      {
        flag = 1;
      }
    }
  }

  closeUdp(local);

  return 0;
}
