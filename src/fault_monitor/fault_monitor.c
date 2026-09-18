/*
 * fault_monitor.c
 *
 * Fault monitoring for the Smart City Digital Twin.
 *
 * Checks stale sensor data, sequence gaps
 * and QNX process death.
 */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <pthread.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <process.h>
#include <sys/procmgr.h>

#include "common.h"
#include "twin_state.h"
#include "fault_monitor.h"


/*
 * Shared TwinState pointer.
 */

static twin_state_t *twin_state = NULL;


/*
 * Death Pulse channel.
 */

static int death_chid = -1;

static int death_coid = -1;

static struct sigevent death_event;


/*
 * Check one sensor for stale data.
 */

static void check_sensor(
        int sensor_id,
        const char *sensor_name,
        struct timespec *timestamp,
        int *fault)
{
    struct timespec now;

    double current_ms;
    double update_ms;
    double age_ms;

    (void)sensor_id;

    clock_gettime(
        CLOCK_MONOTONIC,
        &now);

    current_ms =
        ((double)now.tv_sec * 1000.0) +
        ((double)now.tv_nsec / 1000000.0);

    update_ms =
        ((double)timestamp->tv_sec * 1000.0) +
        ((double)timestamp->tv_nsec / 1000000.0);

    age_ms =
        current_ms - update_ms;


    /*
     * Sensor data is stale after 3 seconds.
     */

    if (age_ms > SENSOR_TIMEOUT_MS)
    {
        if (*fault != FAULT_STALE_DATA)
        {
            printf(
                "[FAULT] %-12s STALE DATA  AGE=%.0f ms\n",
                sensor_name,
                age_ms);
        }

        *fault =
            FAULT_STALE_DATA;
    }
    else
    {
        if (*fault == FAULT_STALE_DATA)
        {
            printf(
                "[FAULT] %-12s DATA RECOVERED\n",
                sensor_name);
        }

        *fault =
            FAULT_NONE;
    }
}


/*
 * Death Pulse monitor.
 */

static void *death_pulse_thread(
        void *arg)
{
    struct _pulse pulse;

    int ret;

    pid_t dead_pid;

    (void)arg;

    printf(
        "[FAULT] Death Pulse monitor started\n");

    while (1)
    {
        ret = MsgReceivePulse(
            death_chid,
            &pulse,
            sizeof(pulse),
            NULL);

        if (ret == -1)
        {
            perror(
                "[FAULT] MsgReceivePulse failed");

            continue;
        }


        if (pulse.code == 1)
        {
            dead_pid =
                pulse.value.sival_int;


            /*
             * Check whether the dead process
             * is our Acquisition process.
             */

            pthread_mutex_lock(
                &twin_state->mutex);

            if (dead_pid ==
                twin_state->acquisition_pid)
            {
                twin_state->acquisition_alive =
                    0;

                printf(
                    "[FAULT] ACQUISITION PROCESS DEATH detected\n");

                printf(
                    "[FAULT] Acquisition PID = %d\n",
                    dead_pid);
            }

            pthread_mutex_unlock(
                &twin_state->mutex);
        }
        else
        {
            printf(
                "[FAULT] Unexpected pulse code = %d\n",
                pulse.code);
        }
    }

    return NULL;
}


/*
 * Fault monitor thread.
 */

void *fault_monitor_thread(
        void *arg)
{
    uint64_t last_reported_gaps = 0;

    (void)arg;

    printf(
        "[FAULT] Fault monitor thread started\n");

    printf(
        "[FAULT] Priority : %d\n",
        FAULT_PRIORITY);

    printf(
        "[FAULT] Timeout  : %d ms\n",
        SENSOR_TIMEOUT_MS);


    while (1)
    {
        pthread_mutex_lock(
            &twin_state->mutex);


        /*
         * Check all four sensors.
         */

        check_sensor(
            SENSOR_ENVIRONMENT,
            ENVIRONMENT_NAME,
            &twin_state->environment.timestamp,
            &twin_state->environment_fault);

        check_sensor(
            SENSOR_TRAFFIC,
            TRAFFIC_NAME,
            &twin_state->traffic.timestamp,
            &twin_state->traffic_fault);

        check_sensor(
            SENSOR_ENERGY,
            ENERGY_NAME,
            &twin_state->energy.timestamp,
            &twin_state->energy_fault);

        check_sensor(
            SENSOR_WATER,
            WATER_NAME,
            &twin_state->water.timestamp,
            &twin_state->water_fault);


        /*
         * Report sequence gaps only when
         * the count changes.
         */

        if (twin_state->sequence_gaps !=
            last_reported_gaps)
        {
            printf(
                "[FAULT] Sequence gaps detected = %llu\n",

                (unsigned long long)
                twin_state->sequence_gaps);

            last_reported_gaps =
                twin_state->sequence_gaps;
        }


        /*
         * Report Acquisition status.
         */

        if (twin_state->acquisition_alive == 0)
        {
            /*
             * Status is already reported by
             * the Death Pulse thread.
             */
        }


        pthread_mutex_unlock(
            &twin_state->mutex);


        /*
         * Run every 100 ms.
         */

        usleep(
            FAULT_CHECK_PERIOD_MS * 1000);
    }

    return NULL;
}


/*
 * Start the Fault Monitor.
 */

int fault_monitor_start(void)
{
    int fd;

    pthread_t monitor_thread;

    pthread_t pulse_thread;

    pthread_attr_t attr;

    struct sched_param param;

    int ret;


    printf("\n");

    printf(
        "========================================\n");

    printf(
        " SMART CITY DIGITAL TWIN\n");

    printf(
        " QNX Fault Monitor\n");

    printf(
        "========================================\n");


    /*
     * Open existing shared memory.
     */

    fd = shm_open(
        TWIN_SHM_NAME,
        O_RDWR,
        0666);

    if (fd == -1)
    {
        perror(
            "[FAULT] shm_open failed");

        return 1;
    }


    /*
     * Map TwinState.
     */

    twin_state = mmap(
        NULL,
        sizeof(twin_state_t),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0);

    close(fd);

    if (twin_state == MAP_FAILED)
    {
        perror(
            "[FAULT] mmap failed");

        return 1;
    }


    /*
     * Create Death Pulse channel.
     */

    death_chid =
        ChannelCreate(
            _NTO_CHF_PRIVATE);

    if (death_chid == -1)
    {
        perror(
            "[FAULT] ChannelCreate failed");

        return 1;
    }


    /*
     * Connect to our channel.
     */

    death_coid =
        ConnectAttach(
            0,
            0,
            death_chid,
            _NTO_SIDE_CHANNEL,
            0);

    if (death_coid == -1)
    {
        perror(
            "[FAULT] ConnectAttach failed");

        return 1;
    }


    /*
     * Configure Death Pulse.
     */

    SIGEV_PULSE_INIT(
        &death_event,
        death_coid,
        10,
        1,
        0);

    SIGEV_MAKE_UPDATEABLE(
        &death_event);


    /*
     * Register process death event.
     */

    ret =
        MsgRegisterEvent(
            &death_event,
            SYSMGR_COID);

    if (ret == -1)
    {
        perror(
            "[FAULT] MsgRegisterEvent failed");

        return 1;
    }


    ret =
        procmgr_event_notify(
            PROCMGR_EVENT_PROCESS_DEATH,
            &death_event);

    if (ret == -1)
    {
        perror(
            "[FAULT] procmgr_event_notify failed");

        return 1;
    }


    printf(
        "[FAULT] Process death notification enabled\n");


    /*
     * Configure Fault Monitor priority.
     */

    pthread_attr_init(
        &attr);

    pthread_attr_setinheritsched(
        &attr,
        PTHREAD_EXPLICIT_SCHED);

    pthread_attr_setschedpolicy(
        &attr,
        SCHED_RR);

    param.sched_priority =
        FAULT_PRIORITY;

    pthread_attr_setschedparam(
        &attr,
        &param);


    /*
     * Create Fault Monitor thread.
     */

    if (pthread_create(
            &monitor_thread,
            &attr,
            fault_monitor_thread,
            NULL) != 0)
    {
        perror(
            "[FAULT] pthread_create failed");

        return 1;
    }


    /*
     * Create Death Pulse thread.
     */

    if (pthread_create(
            &pulse_thread,
            &attr,
            death_pulse_thread,
            NULL) != 0)
    {
        perror(
            "[FAULT] Death Pulse thread creation failed");

        return 1;
    }


    pthread_attr_destroy(
        &attr);


    pthread_join(
        monitor_thread,
        NULL);

    pthread_join(
        pulse_thread,
        NULL);


    return 0;
}
