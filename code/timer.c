/********************************************************
SIGALRM-based timer example

saleem: Jan2024, Jan2022, Jan2021, Jan2004, Nov2002
Checked February 2025, 2026 (sjm55)
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include <errno.h>
void perror(const char *s);

#define ERROR(s_) fprintf(stderr, s_)

#define G_ITIMER_S   ((unsigned int) 3) // seconds
#define G_ITIMER_US  ((unsigned int) 0) // microseconds


/* signals to block - signal handlers add to this mask */
sigset_t G_sigmask;

/* signal action / handler hook */
struct sigaction G_sigalrm;

/* timer value */
struct itimerval G_timer;

/*
  alarm functions
*/
#define G_ALERT "** The SIGALRM went off! **\a\n"
void handleSIGALRM(int sig);
void setupSIGALRM();
void setITIMER(unsigned int sec, unsigned int usec);

int
main()
{
  setupSIGALRM();
  while(1) // forever
    (void) pause(); // wait for signal, otherwise do nothing
}


void
setITIMER(unsigned int sec, unsigned int usec)
{
  G_timer.it_interval.tv_sec = sec;
  G_timer.it_interval.tv_usec = usec;
  G_timer.it_value.tv_sec = sec;
  G_timer.it_value.tv_usec = usec;
  if (setitimer(ITIMER_REAL, &G_timer, (struct itimerval *) 0) != 0)
    perror("setITIMER(): setitimer() problem");
}


void
handleSIGALRM(int sig)
{
  if (sig == SIGALRM) {
    /* protect handler actions from signals */
    sigprocmask(SIG_BLOCK, &G_sigmask, (sigset_t *) 0);

    /* print and fflush() to ensure print happens immediately */
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

