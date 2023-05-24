#include <asm-generic/errno-base.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <semaphore.h>
#include <unistd.h>
#include "pthread_pause.h"

// see https://stackoverflow.com/a/68119116

//#define _GNU_SOURCE

#define PTHREAD_XSIG_STOP (SIGRTMIN+0)
#define PTHREAD_XSIG_CONT (SIGRTMIN+1)
#define PTHREAD_XSIGRTMIN (SIGRTMIN+2) //First unused RT signal

sem_t pthread_pause_sem;
pthread_once_t pthread_pause_once_ctrl = PTHREAD_ONCE_INIT;
#define pthread_pause_init() (pthread_once(&pthread_pause_once_ctrl, &pthread_pause_once))

#define NSEC_PER_SEC (1000*1000*1000)
// timespec_normalise() from https://github.com/solemnwarning/timespec/
struct timespec timespec_normalise(struct timespec ts)
{
    while(ts.tv_nsec >= NSEC_PER_SEC) {
        ++(ts.tv_sec); ts.tv_nsec -= NSEC_PER_SEC;
    }
    while(ts.tv_nsec <= -NSEC_PER_SEC) {
        --(ts.tv_sec); ts.tv_nsec += NSEC_PER_SEC;
    }
    if(ts.tv_nsec < 0) { // Negative nanoseconds isn't valid according to POSIX.
        --(ts.tv_sec); ts.tv_nsec = (NSEC_PER_SEC + ts.tv_nsec);
    }
    return ts;
}

static void pthread_pause_once(void) {
    sem_init(&pthread_pause_sem, 0, 1);
}

static void pthread_nanosleep(struct timespec t) {
    //Sleep calls on Linux get interrupted by signals, causing premature wake
    //Pthread (un)pause is built using signals
    //Therefore we need self-restarting sleep implementation
    //IO timeouts are restarted by SA_RESTART, but sleeps do need explicit restart
    //We also need to sleep using absolute time, because relative time is paused
    //You should use this in any thread that gets (un)paused

    struct timespec wake;
    clock_gettime(CLOCK_MONOTONIC, &wake);

    t = timespec_normalise(t);
    wake.tv_sec += t.tv_sec;
    wake.tv_nsec += t.tv_nsec;
    wake = timespec_normalise(wake);

    while(clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake, NULL)) if(errno!=EINTR) break;
    return;
}

static void pthread_nsleep(time_t s, long ns) {
    struct timespec t;
    t.tv_sec = s;
    t.tv_nsec = ns;
    pthread_nanosleep(t);
}

static void pthread_sleep(time_t s) {
    pthread_nsleep(s, 0);
}

void pthread_pause_yield(void) {
    //Call this to give other threads chance to run
    //Wait until last (un)pause action gets finished
    sem_wait(&pthread_pause_sem);
    sem_post(&pthread_pause_sem);
        //usleep(0);
    //nanosleep(&((const struct timespec){.tv_sec=0,.tv_nsec=1}), NULL);
    //pthread_nsleep(0,1); //pthread_yield() is not enough, so we use sleep
//    pthread_yield();
    sched_yield();
}

static void pthread_pause_handler(int signal) {
    //Do nothing when there are more signals pending (to cleanup the queue)
    //This is no longer needed, since we use semaphore to limit pending signals
    /*
    sigset_t pending;
    sigpending(&pending);
    if(sigismember(&pending, PTHREAD_XSIG_STOP)) return;
    if(sigismember(&pending, PTHREAD_XSIG_CONT)) return;
    */

    //Post semaphore to confirm that signal is handled
    sem_post(&pthread_pause_sem);
    //Suspend if needed
    if(signal == PTHREAD_XSIG_STOP) {
        sigset_t sigset;
        sigfillset(&sigset);
        sigdelset(&sigset, PTHREAD_XSIG_STOP);
        sigdelset(&sigset, PTHREAD_XSIG_CONT);
        sigsuspend(&sigset); //Wait for next signal
    } else return;
}

void pthread_pause_enable(void) {
    //Having signal queue too deep might not be necessary
    //It can be limited using RLIMIT_SIGPENDING
    //You can get runtime SigQ stats using following command:
    //grep -i sig /proc/$(pgrep binary)/status
    //This is no longer needed, since we use semaphores
    //struct rlimit sigq = {.rlim_cur = 32, .rlim_max=32};
    //setrlimit(RLIMIT_SIGPENDING, &sigq);

    pthread_pause_init();

    //Prepare sigset
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, PTHREAD_XSIG_STOP);
    sigaddset(&sigset, PTHREAD_XSIG_CONT);

    //Register signal handlers
    //signal(PTHREAD_XSIG_STOP, pthread_pause_handler);
    //signal(PTHREAD_XSIG_CONT, pthread_pause_handler);
    //We now use sigaction() instead of signal(), because it supports SA_RESTART
    struct sigaction pause_sa;
    pause_sa.sa_handler = pthread_pause_handler;
    pause_sa.sa_mask = sigset;
    pause_sa.sa_flags = SA_RESTART;
    pause_sa.sa_restorer = NULL;
    sigaction(PTHREAD_XSIG_STOP, &pause_sa, NULL);
    sigaction(PTHREAD_XSIG_CONT, &pause_sa, NULL);

    //UnBlock signals
    pthread_sigmask(SIG_UNBLOCK, &sigset, NULL);
}

void pthread_pause_disable(void) {
    //This is important for when you want to do some signal unsafe stuff
    //Eg.: locking mutex, calling printf() which has internal mutex, etc...
    //After unlocking mutex, you can enable pause again.

    pthread_pause_init();

    //Make sure all signals are dispatched before we block them
    sem_wait(&pthread_pause_sem);

    //Block signals
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, PTHREAD_XSIG_STOP);
    sigaddset(&sigset, PTHREAD_XSIG_CONT);
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    sem_post(&pthread_pause_sem);
}

int pthread_pause(pthread_t thread) {
    sem_wait(&pthread_pause_sem);
    //If signal queue is full, we keep retrying
    while(pthread_kill(thread, PTHREAD_XSIG_STOP) == EAGAIN) usleep(1000);
    pthread_pause_yield();
    return 0;
}

int pthread_unpause(pthread_t thread) {
    sem_wait(&pthread_pause_sem);
    //If signal queue is full, we keep retrying
    while(pthread_kill(thread, PTHREAD_XSIG_CONT) == EAGAIN) usleep(1000);
    pthread_pause_yield();
    return 0;
}
