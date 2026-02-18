/********************************************************
UDP IPv4 client using UdpSocket_t.

Every second sends fixed-size (128 byte) UDP packets containing
random uint8_t integers a specified host on specified port.
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

int readLine(int fd, UdpBuffer_t *buffer);

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        ERROR("usage: udp-sender <hostname> <local_port>\n\nhint: use 'id -u' or getuid(2) to get your port\n\nhint2: you can run `udp-sender <hostname> $(id -u)` to input your port automatically");
        exit(0);
    }

    char *hostname = argv[1];
    uint16_t local_port = (uint16_t)atoi(argv[2]);

    srand(69);

    UdpSocket_t *local, *remote;
    UdpBuffer_t buffer;
    uint8_t bytes[G_SIZE];

    if ((local = setupUdpSocket_t((char *)0, local_port)) == (UdpSocket_t *)0)
    {
        ERROR("local problem");
        exit(0);
    }

    if ((remote = setupUdpSocket_t(hostname, G_SRV_PORT)) == (UdpSocket_t *)0)
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

        if (sendUdp(local, remote, &buffer) != buffer.n)
            ERROR("sendUdp() problem");

        sleep(1);
    }

    closeUdp(local);
    closeUdp(remote);

    return 0;
}
