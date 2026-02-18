/**
 * To collect data and put it into the data.csv do
 * stdbuf -oL ./udp_sender klovia $(id -u) > ../data/data.csv
 * (assuming you're at root)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>

#include <poll.h>
#include <time.h>
#include <sys/select.h>
#include "UdpSocket.h"

#define G_SRV_PORT ((uint16_t)24628) // use 'id -u' or getuid(2)
#define G_SIZE ((uint32_t)128)
#define TIMER ((uint32_t)20*60) // 20 min

#define ERROR(_s) fprintf(stderr, "%s\n", _s)

int get_udp_response(UdpSocket_t *local,
                     UdpSocket_t *remote,
                     UdpBuffer_t *buffer,
                     int timeout_ms)
{
    struct pollfd pfd;
    pfd.fd = local->sd;
    pfd.events = POLLIN;

    int ready = poll(&pfd, 1, timeout_ms);

    if (ready > 0)
        return recvUdp(local, remote, buffer);
    
    if (ready == 0) return -2; // Timeout code

    return -1; // System error code
}

int main(int argc, char *argv[])
{
    srand(69);

    if (argc != 3)
    {
        ERROR("usage: udp-sender <hostname> <local_port>\n\nhint: use 'id -u' or getuid(2) to get your port");
        exit(0);
    }

    char *hostname = argv[1];
    uint16_t local_port = (uint16_t)atoi(argv[2]);

    UdpSocket_t *local = NULL, *remote = NULL;
    UdpBuffer_t buffer;
    uint8_t bytes[G_SIZE];

    if ((local = setupUdpSocket_t((char *)0, local_port)) == (UdpSocket_t *)0)
    {
        ERROR("local problem");
        return 1;
    }

    if ((remote = setupUdpSocket_t(hostname, G_SRV_PORT)) == (UdpSocket_t *)0)
    {
        ERROR("remote hostname/port problem");
        closeUdp(local);
        return 1;
    }

    if (openUdp(local) < 0)
    {
        ERROR("openUdp() problem");
        closeUdp(local);
        closeUdp(remote);
        return 1;
    }

    uint32_t counter = 0;
    struct timespec start_time, current_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    double elapsed = 0;

    while (elapsed < TIMER)
    {
        struct timespec send_ts, recv_ts;
        buffer.bytes = bytes;
        buffer.n = G_SIZE;

        for (uint32_t i = 0; i < buffer.n; i++)
            buffer.bytes[i] = (uint8_t)(rand() % 127);

        clock_gettime(CLOCK_MONOTONIC, &send_ts);

        if (sendUdp(local, remote, &buffer) != (int)buffer.n)
            ERROR("sendUdp() problem");

        int r = get_udp_response(local, remote, &buffer, 1);

        if (r >= 0) // packet is valid even if 0 bytes
        {
            clock_gettime(CLOCK_MONOTONIC, &recv_ts);
            double rtt_ms = (recv_ts.tv_sec - send_ts.tv_sec) * 1000.0 + 
                            (recv_ts.tv_nsec - send_ts.tv_nsec) / 1e6;
            printf("%u,%.3f,%d\n", counter, rtt_ms, r);
            fflush(stdout);
        }
        else if (r == -2)
        {
            printf("LOST (Timeout),,\n");
            fflush(stdout);
        }
        else
        {
            ERROR("System Error in Poll/Recv");
        }

        clock_gettime(CLOCK_MONOTONIC, &current_time);
        elapsed = (double)(current_time.tv_sec - start_time.tv_sec) +
                  (double)(current_time.tv_nsec - start_time.tv_nsec) / 1e9;

        counter++;
        sleep(1);
    }

    closeUdp(local);
    closeUdp(remote);

    return 0;
}