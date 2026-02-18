/********************************************************
SIGIO-based network input example (event-driven)

saleem: Jan2024, Jan2022, Jan2021, Jan2004, Nov2002
Checked February 2025, 2026 (sjm55)
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <inttypes.h>
#include <sys/time.h>
#include <errno.h>

#include "UdpSocket.h"

#define G_MY_PORT ((uint16_t) 54321) // use 'id -u' or getuid(2)
#define G_SIZE    ((uint32_t) 256)

#define ERROR(_s) fprintf(stderr, "%s\n", _s)

#define G_QUIT "quit"

/* descriptor to watch */
int G_net;

/* signals to block - signal handlers add to this mask */
sigset_t G_sigmask;

/* signal action / handler hooks */
struct sigaction G_sigio;

/* other globals */
UdpSocket_t *G_local;
int G_quit = 0;

/*
  i/o functions
*/

int setAsyncFd(int fd);
void checkNetwork();
void handleSIGIO(int sig);
void setupSIGIO();

int main()
{
  if ((G_local = setupUdpSocket_t((char *) 0, G_MY_PORT))
      == (UdpSocket_t *) 0) {
    ERROR("local problem");
    exit(0);
  }

  if (openUdp(G_local) < 0) {
    ERROR("openUdp() problem");
    exit(0);
  }
  printf("ready on port %d ...\n\n", ntohs(G_local->addr.sin_port));

  G_net = G_local->sd;
  setupSIGIO();
  while(!G_quit)
    (void) pause(); // wait for signal, otherwise do nothing
}



int
setAsyncFd(int fd)
{
  int r, flags = O_NONBLOCK | O_ASYNC; // man 2 fcntl

  if (fcntl(fd, F_SETOWN, getpid()) < 0) {
    perror("setAsyncFd(): fcntl(fd, F_SETOWN, getpid()");
    exit(0);
  }

  if ((r = fcntl(fd, F_SETFL, flags)) < 0) {
    perror("setAsyncFd(): fcntl() problem");
    exit(0);
  }

  return r;
}


void
checkNetwork()
{
  UdpSocket_t receive;
  UdpBuffer_t buffer;
  unsigned char bytes[G_SIZE];
  int r;

  /* print any network input */
  buffer.bytes = bytes;
  buffer.n = G_SIZE;

  if ((r = recvUdp(G_local, &receive, &buffer)) < 0) {
    if (errno != EWOULDBLOCK)
      ERROR("checkNetwork(): recvUdp() problem");
  }
  else {
    bytes[r] = '\0';
    printf("-> %s\n", bytes);
    if (r > 0 && strcmp((char *) bytes, G_QUIT) == 0) G_quit = 1;
  }
}


void
handleSIGIO(int sig)
{
  if (sig == SIGIO) {
    /* protect the network and keyboard reads from signals */
    sigprocmask(SIG_BLOCK, &G_sigmask, (sigset_t *) 0);

    checkNetwork();

    /* allow the signals to be delivered */
    sigprocmask(SIG_UNBLOCK, &G_sigmask, (sigset_t *) 0);
  }
  else
    ERROR("handleSIGIO(): got a bad signal number");
}

void
setupSIGIO()
{
  sigaddset(&G_sigmask, SIGIO);

  G_sigio.sa_handler = handleSIGIO;
  G_sigio.sa_flags = 0;

  if (sigaction(SIGIO, &G_sigio, (struct sigaction *) 0) < 0) {
    perror("setupSIGIO(): sigaction() problem");
    exit(0);
  }

  if (setAsyncFd(G_net) < 0) {
    ERROR("setupSIGIO(): setAsyncFd(G_net) problem");
    exit(0);
  }
}
