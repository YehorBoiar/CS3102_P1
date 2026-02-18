/* *******************************************************
simple UDP IPv4 client

saleem: Jan2024, Jan2022, Jan2021, Nov2002, Dec2001
Checked February 2025, 2026 (sjm55)
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define G_SRV_PORT ((uint16_t) 54321) // use 'id -u' or getuid(2)

char *G_str = "hello world!";

#define ERROR(_s) fprintf(stderr, "%s\n", _s)

int
main(int argc, char *argv[])
{
    int sd;
    struct sockaddr_in saddr, caddr;
    int r;
    int l;
    struct hostent *hp;

    if (argc != 2) {
        ERROR("usage: udp-client-1 <hostname>");
        exit(0);
    }

    if ((hp = gethostbyname(argv[1])) == (struct hostent *) 0) {
        ERROR("unkown host");
        exit(0);
    }

    /* open a UDP socket */
    if ((sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        ERROR("socket() failed");
        exit(0);
    }

    (void) memset((void *) &caddr, 0, sizeof(caddr));
    caddr.sin_family      = AF_INET;
    caddr.sin_addr.s_addr = htonl(INADDR_ANY);
    caddr.sin_port        = htons(G_SRV_PORT);

    if (bind(sd, (struct sockaddr *) &caddr, sizeof(caddr)) < 0) {
        ERROR("bind() failed");
        exit(0);
    }

    /* send one packet then exit! */
    (void) memset((void *) &saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    memcpy((void *) &saddr.sin_addr.s_addr,
           (void *) *hp->h_addr_list, sizeof(saddr.sin_addr.s_addr));
    saddr.sin_port = htons(G_SRV_PORT);
    l = sizeof(saddr);

    if ((r = sendto(sd, (void *) G_str, strlen(G_str), 0,
                    (struct sockaddr *) &saddr, l)) < 0)
      ERROR("sendto() problem");

    (void) close(sd);

    return 0;
}
