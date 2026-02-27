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
#define TIMER ((uint32_t)20 * 60)    // 20 min
#define LOST ((u_int32_t) -2)
#define ERROR_OR_CORRUPT ((u_int32_t) -1)
#define ERROR(_s) fprintf(stderr, "%s\n", _s)

typedef struct
{
    uint32_t seq_num;
    struct timespec send_ts;
    uint32_t probe_magic_val; // to check whether we received our own packet
} ProbePacket_t;

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

    if (ready == 0)
        return LOST;

    return ERROR_OR_CORRUPT;
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
    struct timespec start_run;
    clock_gettime(CLOCK_MONOTONIC, &start_run);

    printf("sequence_number,rtt_ms,bytes_received,status\n"); // csv header

    while (counter < 1200)
    { // 20 minutes @ 1 per second

        ProbePacket_t tx_pkt, rx_pkt;
        tx_pkt.seq_num = counter;
        tx_pkt.probe_magic_val = 0xDEADBEEF;
        clock_gettime(CLOCK_MONOTONIC, &tx_pkt.send_ts);

        buffer.bytes = (uint8_t *)&tx_pkt;
        buffer.n = sizeof(tx_pkt);

        // 1. Send Probe
        if (sendUdp(local, remote, &buffer) != (int)buffer.n)
        {
            ERROR("sendUdp() failed");
        }

        UdpBuffer_t rx_buffer;
        rx_buffer.bytes = (uint8_t *)&rx_pkt;
        rx_buffer.n = sizeof(rx_pkt);

        int r = get_udp_response(local, remote, &rx_buffer, 1000);

        if (r == (int)sizeof(ProbePacket_t) && rx_pkt.probe_magic_val == 0xDEADBEEF)
        {
            struct timespec recv_ts;
            clock_gettime(CLOCK_MONOTONIC, &recv_ts);

            // Calculate RTT: RTT = (T_recv - T_send)
            double rtt_ms = (recv_ts.tv_sec - tx_pkt.send_ts.tv_sec) * 1000.0 +
                            (recv_ts.tv_nsec - tx_pkt.send_ts.tv_nsec) / 1e6;

            printf("%u,%.3f,%d,SUCCESS\n", rx_pkt.seq_num, rtt_ms, r);
        }
        else if (r == LOST)
        {
            printf("%u,,0,LOST\n", counter);
        }
        else
        {
            printf("%u,,0,ERROR_OR_CORRUPT\n", counter);
        }

        fflush(stdout);
        counter++;
        sleep(1);
    }

    closeUdp(local);
    closeUdp(remote);

    return 0;
}