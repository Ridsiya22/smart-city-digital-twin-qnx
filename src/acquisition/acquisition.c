/*
 * acquisition.c
 *
 * Sensor acquisition process for the
 * Smart City Digital Twin.
 *
 * Sensor values are currently simulated.
 * Four QNX threads generate sensor messages
 * and send them to the Synchronizer.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sched.h>
#include <sys/neutrino.h>

#include "common.h"
#include "messages.h"
#include "twin_state.h"


/*
 * Connection ID for the Synchronizer.
 */

static int sync_coid = -1;


/*
 * Sequence numbers for each sensor.
 */

static uint64_t environment_sequence = 0;

static uint64_t traffic_sequence = 0;

static uint64_t energy_sequence = 0;

static uint64_t water_sequence = 0;


/*
 * Send one sensor message to the Synchronizer.
 */

static int send_sensor_message(
        sensor_msg_t *msg)
{
    if (sync_coid == -1)
    {
        return -1;
    }

    if (MsgSend(
            sync_coid,
            msg,
            sizeof(sensor_msg_t),
            NULL,
            0) == -1)
    {
        perror("[ACQ] MsgSend");

        return -1;
    }

    return 0;
}


/*
 * Environment thread.
 */

static void *environment_thread(
        void *arg)
{
    sensor_msg_t msg;

    (void)arg;

    printf(
        "[ACQ] Environment thread started\n");

    while (1)
    {
        memset(
            &msg,
            0,
            sizeof(msg));

        environment_read(&msg);

        environment_sequence++;

        msg.sequence =
            environment_sequence;

        if (send_sensor_message(&msg) == -1)
        {
            printf(
                "[ACQ] Environment message failed\n");
        }

        usleep(
            ENVIRONMENT_PERIOD_MS * 1000);
    }

    return NULL;
}


/*
 * Traffic thread.
 */

static void *traffic_thread(
        void *arg)
{
    sensor_msg_t msg;

    (void)arg;

    printf(
        "[ACQ] Traffic thread started\n");

    while (1)
    {
        memset(
            &msg,
            0,
            sizeof(msg));

        traffic_read(&msg);

        traffic_sequence++;

        msg.sequence =
            traffic_sequence;

        if (send_sensor_message(&msg) == -1)
        {
            printf(
                "[ACQ] Traffic message failed\n");
        }

        usleep(
            TRAFFIC_PERIOD_MS * 1000);
    }

    return NULL;
}


/*
 * Energy thread.
 */

static void *energy_thread(
        void *arg)
{
    sensor_msg_t msg;

    (void)arg;

    printf(
        "[ACQ] Energy thread started\n");

    while (1)
    {
        memset(
            &msg,
            0,
            sizeof(msg));

        energy_read(&msg);

        energy_sequence++;

        msg.sequence =
            energy_sequence;

        if (send_sensor_message(&msg) == -1)
        {
            printf(
                "[ACQ] Energy message failed\n");
        }

        usleep(
            ENERGY_PERIOD_MS * 1000);
    }

    return NULL;
}


/*
 * Water thread.
 */

static void *water_thread(
        void *arg)
{
    sensor_msg_t msg;

    (void)arg;

    printf(
        "[ACQ] Water thread started\n");

    while (1)
    {
        memset(
            &msg,
            0,
            sizeof(msg));

        water_read(&msg);

        water_sequence++;

        msg.sequence =
            water_sequence;

        if (send_sensor_message(&msg) == -1)
        {
            printf(
                "[ACQ] Water message failed\n");
        }

        usleep(
            WATER_PERIOD_MS * 1000);
    }

    return NULL;
}


/*
 * Create one Acquisition thread.
 */

static int create_acquisition_thread(
        pthread_t *thread,
        void *(*function)(void *))
{
    pthread_attr_t attr;
    struct sched_param param;

    int ret;

    ret = pthread_attr_init(
        &attr);

    if (ret != 0)
    {
        printf(
            "[ACQ] pthread_attr_init failed\n");

        return -1;
    }

    ret = pthread_attr_setinheritsched(
        &attr,
        PTHREAD_EXPLICIT_SCHED);

    if (ret != 0)
    {
        printf(
            "[ACQ] pthread_attr_setinheritsched failed\n");

        pthread_attr_destroy(&attr);

        return -1;
    }

    ret = pthread_attr_setschedpolicy(
        &attr,
        SCHED_RR);

    if (ret != 0)
    {
        printf(
            "[ACQ] pthread_attr_setschedpolicy failed\n");

        pthread_attr_destroy(&attr);

        return -1;
    }

    param.sched_priority =
        ACQ_PRIORITY;

    ret = pthread_attr_setschedparam(
        &attr,
        &param);

    if (ret != 0)
    {
        printf(
            "[ACQ] pthread_attr_setschedparam failed\n");

        pthread_attr_destroy(&attr);

        return -1;
    }

    ret = pthread_create(
        thread,
        &attr,
        function,
        NULL);

    pthread_attr_destroy(
        &attr);

    if (ret != 0)
    {
        printf(
            "[ACQ] pthread_create failed\n");

        return -1;
    }

    return 0;
}


/*
 * Record Acquisition PID in shared state.
 */

static int register_acquisition_pid(void)
{
    int fd;

    twin_state_t *state;

    fd = shm_open(
        TWIN_SHM_NAME,
        O_RDWR,
        0666);

    if (fd == -1)
    {
        perror(
            "[ACQ] shm_open");

        return -1;
    }

    state = mmap(
        NULL,
        sizeof(twin_state_t),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0);

    close(fd);

    if (state == MAP_FAILED)
    {
        perror(
            "[ACQ] mmap");

        return -1;
    }

    pthread_mutex_lock(
        &state->mutex);

    state->acquisition_pid =
        getpid();

    state->acquisition_alive =
        1;

    pthread_mutex_unlock(
        &state->mutex);

    printf(
        "[ACQ] Acquisition PID : %d\n",
        getpid());

    munmap(
        state,
        sizeof(twin_state_t));

    return 0;
}


/*
 * Start the Acquisition process.
 */

int acquisition_start(void)
{
    pthread_t environment_tid;
    pthread_t traffic_tid;
    pthread_t energy_tid;
    pthread_t water_tid;

    printf("\n");

    printf(
        "========================================\n");

    printf(
        " SMART CITY DIGITAL TWIN\n");

    printf(
        " QNX Acquisition\n");

    printf(
        "========================================\n");


    /*
     * Connect to the Synchronizer.
     */

    sync_coid =
        name_open(
            SYNC_SERVER_NAME,
            0);

    if (sync_coid == -1)
    {
        perror(
            "[ACQ] name_open");

        return 1;
    }


    /*
     * Store our PID in TwinState.
     */

    if (register_acquisition_pid() == -1)
    {
        name_close(sync_coid);

        return 1;
    }


    printf(
        "[ACQ] Connected to synchronizer\n");


    /*
     * Create sensor threads.
     */

    if (create_acquisition_thread(
            &environment_tid,
            environment_thread) == -1)
    {
        return 1;
    }

    if (create_acquisition_thread(
            &traffic_tid,
            traffic_thread) == -1)
    {
        return 1;
    }

    if (create_acquisition_thread(
            &energy_tid,
            energy_thread) == -1)
    {
        return 1;
    }

    if (create_acquisition_thread(
            &water_tid,
            water_thread) == -1)
    {
        return 1;
    }


    /*
     * Wait for all threads.
     */

    pthread_join(
        environment_tid,
        NULL);

    pthread_join(
        traffic_tid,
        NULL);

    pthread_join(
        energy_tid,
        NULL);

    pthread_join(
        water_tid,
        NULL);


    name_close(
        sync_coid);

    return 0;
}
