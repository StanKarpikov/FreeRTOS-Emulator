#ifndef PTHREAD_PAUSE_H
#define PTHREAD_PAUSE_H

#define _GNU_SOURCE
#include <pthread.h>

void pthread_pause_yield(void);
void pthread_pause_enable(void);
void pthread_pause_disable(void);
int pthread_pause(pthread_t thread);
int pthread_unpause(pthread_t thread);

#endif // PTHREAD_PAUSE_H
