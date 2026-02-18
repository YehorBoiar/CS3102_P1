/********************************************************
simple UDP IPv4  chat with async, non-blocking i/o, and
wake-up alert call (bell) for snoozing chatters.
(event-driven program)

saleem: Jan2024, Jan2022, Jan2021, Jan2004, Nov2002
Checked February 2025, 2026 (sjm55)
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>

#include "UdpSocket.h"

#define G_MY_PORT    ((uint16_t) 54321) // use 'id -u', or getuid(2)
#define G_SIZE       ((uint32_t) 256)
#define G_ITIMER_S   ((uint32_t) 10) // seconds
#define G_ITIMER_US  ((uint32_t) 0) // microseconds

#define ERROR(_s) fprintf(stderr, "%s\n", _s)

#define G_QUIT "quit"
#define G_ALERT "** Wake up and type something! **\a\n"

/* descriptors to watch */
int G_kbd;
int G_net;

/* signals to block - signal handlers add to this mask */
sigset_t G_sigmask;

/* signal action / handler hooks */
struct sigaction G_sigio, G_sigalrm;

/* other globals */
UdpSocket_t *G_local;
UdpSocket_t *G_remote;
struct itimerval G_timer;
int G_flag;

/*
  i/o functions
*/

int setAsyncFd(int fd);
void handleSIGIO(int sig);
void setupSIGIO();
void checkKeyboard();
void checkNetwork();
int readLine(int fd, UdpBuffer_t *buffer);

/*
  alarm functions
*/

void handleSIGALRM(int sig);
void setupSIGALRM();
void setITIMER(uint32_t sec, uint32_t usec);



int
main(int argc, char *argv[])
{
  if (argc != 2) {
    ERROR("usage: udp-chat-3 <hostname>");
    exit(0);
  }

  if ((G_local = setupUdpSocket_t((char *) 0, G_MY_PORT))
      == (UdpSocket_t *) 0) {
    ERROR("local problem");
    exit(0);
  }

  if ((G_remote = setupUdpSocket_t(argv[1], G_MY_PORT))
      == (UdpSocket_t *) 0) {
    ERROR("remote hostname/port problem");
    exit(0);
  }

  if (openUdp(G_local) < 0) {
    ERROR("openUdp() problem");
    exit(0);
  }

  /* initialise global vars */
  G_net = G_local->sd;
  G_kbd = fileno(stdin);
  sigemptyset(&G_sigmask);

  /* setup signal handlers */
  setupSIGIO();
  setupSIGALRM();

  G_flag = 0;
  while(!G_flag)
    (void) pause(); // wait for signal, otherwise do nothing

  /* tidy up */
  closeUdp(G_local);
  closeUdp(G_remote);

  return 0;
}

int
readLine(int fd, UdpBuffer_t *buffer)
{
  int r = 0;

  if ((r = read(fd, (void *) buffer->bytes, buffer->n)) > 0) {
    buffer->n = r;
  } else if (errno == EWOULDBLOCK || r == 0) { /* nothing to read */
    r = 0;
    buffer->n = 0;
  }
  else if (r < 0)
    perror("readline(): read() problem");

  return r;
}


void
handleSIGIO(int sig)
{
  if (sig == SIGIO) {
    /* protect the network and keyboard reads from signals */
    sigprocmask(SIG_BLOCK, &G_sigmask, (sigset_t *) 0);

    checkKeyboard();
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

  if (setAsyncFd(G_kbd) < 0) {
    ERROR("setupSIGIO(): setAsyncFd(G_kbd) problem");
    exit(0);
  }

}


void
setITIMER(uint32_t sec, uint32_t usec)
{
  G_timer.it_interval.tv_sec = sec;
  G_timer.it_interval.tv_usec = usec;
  G_timer.it_value.tv_sec = sec;
  G_timer.it_value.tv_usec = usec;
  if (setitimer(ITIMER_REAL, &G_timer, (struct itimerval *) 0) != 0)
    ERROR("setITIMER(): setitimer() problem");
}


void
handleSIGALRM(int sig)
{
  if (sig == SIGALRM) {
    /* protect handler actions from signals */
    sigprocmask(SIG_BLOCK, &G_sigmask, (sigset_t *) 0);

    fprintf(stdout, G_ALERT); fflush(stdout);

    /* protect handler actions from signals */
    sigprocmask(SIG_UNBLOCK, &G_sigmask, (sigset_t *) 0);
  }
  else
    ERROR("handleSIGALRM() got a bad signal");
}

void
setupSIGALRM()
{
  sigaddset(&G_sigmask, SIGALRM);

  G_sigalrm.sa_handler = handleSIGALRM;
  G_sigalrm.sa_flags = 0;

  if (sigaction(SIGALRM, &G_sigalrm, (struct sigaction *) 0) < 0) {
    perror("setupSIGALRM(): sigaction() problem");
    exit(0);
  }
  else
    setITIMER(G_ITIMER_S, G_ITIMER_US);
}

void
checkKeyboard()
{
  UdpBuffer_t buffer;
  unsigned char bytes[G_SIZE];
  int r;

  /* send any keyboard input */
  buffer.bytes = bytes;
  buffer.n = G_SIZE;
  if ((r = readLine(G_kbd, &buffer)) > 0) {
    buffer.bytes[--buffer.n] = '\0'; /* stomp over the '\n' */

    if (strcmp((char *) buffer.bytes, G_QUIT) == 0)
      G_flag = 1;

    if (sendUdp(G_local, G_remote, &buffer) != buffer.n)
      ERROR("checkKeyboard(): sendUdp() problem");

    /* reset timeout alarm */
    setITIMER(G_ITIMER_S, G_ITIMER_US);
  }
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
  }
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
