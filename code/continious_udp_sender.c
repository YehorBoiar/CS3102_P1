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
#define G_SIZE ((uint32_t)128)

#define ERROR(_s) fprintf(stderr, "%s\n", _s)

#define G_QUIT "quit"

int readLine(int fd, UdpBuffer_t *buffer);

int main(int argc, char *argv[])
{
    srand(69);

    UdpSocket_t *local, *remote;
    UdpBuffer_t buffer;
    uint8_t bytes[G_SIZE];

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

    while (1)
    {
        buffer.bytes = bytes;
        buffer.n = G_SIZE;

        // fill with nonsense data
        for (uint8_t i = 0; i < buffer.n; i++)
            buffer.bytes[i] = rand() % 127;

        if ((buffer.n > 0) && (sendUdp(local, remote, &buffer) != buffer.n))
            ERROR("sendUdp() problem");

    }

    closeUdp(local);
    closeUdp(remote);

    return 0;
}
