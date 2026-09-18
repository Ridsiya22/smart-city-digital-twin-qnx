/*
 * deadlock.c
 *
 * Deadlock avoidance demonstration.
 *
 * Two threads use two mutexes.
 * Both threads acquire the mutexes in the
 * same order to avoid circular waiting.
 */

#include <stdio.h>
#include <unistd.h>

#include <pthread.h>
#include "common.h"


/*
 * Two shared resources.
 */

static pthread_mutex_t resource1 =
    PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t resource2 =
    PTHREAD_MUTEX_INITIALIZER;


/*
 * Thread 1.
 */

static void *thread1(void *arg)
{
    (void)arg;

    printf("[DEADLOCK] Thread 1 started\n");


    /*
     * Always acquire Resource 1 first.
     */

    pthread_mutex_lock(&resource1);

    printf("[DEADLOCK] Thread 1 locked Resource 1\n");

    usleep(100000);


    /*
     * Acquire Resource 2 second.
     */

    pthread_mutex_lock(&resource2);

    printf("[DEADLOCK] Thread 1 locked Resource 2\n");

    printf("[DEADLOCK] Thread 1 completed safely\n");


    pthread_mutex_unlock(&resource2);
    pthread_mutex_unlock(&resource1);

    return NULL;
}


/*
 * Thread 2.
 */

static void *thread2(void *arg)
{
    (void)arg;

    printf("[DEADLOCK] Thread 2 started\n");


    /*
     * Thread 2 uses the SAME order:
     *
     * Resource 1 -> Resource 2
     */

    pthread_mutex_lock(&resource1);

    printf("[DEADLOCK] Thread 2 locked Resource 1\n");

    usleep(100000);


    pthread_mutex_lock(&resource2);

    printf("[DEADLOCK] Thread 2 locked Resource 2\n");

    printf("[DEADLOCK] Thread 2 completed safely\n");


    pthread_mutex_unlock(&resource2);
    pthread_mutex_unlock(&resource1);

    return NULL;
}


/*
 * Start deadlock avoidance demonstration.
 */

int deadlock_start(void)
{
    pthread_t t1;
    pthread_t t2;


    printf("\n");
    printf("========================================\n");
    printf(" SMART CITY DIGITAL TWIN\n");
    printf(" Deadlock Avoidance Demonstration\n");
    printf("========================================\n");


    /*
     * Create two threads.
     */

    pthread_create(
        &t1,
        NULL,
        thread1,
        NULL);

    pthread_create(
        &t2,
        NULL,
        thread2,
        NULL);


    /*
     * Wait for both threads.
     */

    pthread_join(
        t1,
        NULL);

    pthread_join(
        t2,
        NULL);


    printf("[DEADLOCK] No deadlock occurred\n");
    printf("[DEADLOCK] Consistent mutex ordering used\n");


    return 0;
}
