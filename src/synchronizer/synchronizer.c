/*
 * synchronizer.c
 *
 * QNX Synchronizer process for the Smart City Digital Twin.
 *
 * Receives sensor messages using QNX native message passing
 * and updates the shared Digital Twin state.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>

#include "common.h"
#include "messages.h"
#include "twin_state.h"
#include "fault_monitor.h"

/*
 * Shared Digital Twin state.
 */

static twin_state_t *twin_state;


/*
 * Calculate time difference in milliseconds.
 */

static double calculate_latency_ms(
        struct timespec *start,
        struct timespec *end)
{
    double seconds;
    double nanoseconds;

    seconds = (double)(end->tv_sec - start->tv_sec);
    nanoseconds = (double)(end->tv_nsec - start->tv_nsec);

    return (seconds * 1000.0) +
           (nanoseconds / 1000000.0);
}


/*
 * Initialize the shared Digital Twin state.
 */

static int initialize_twin_state(void)
{
    int shm_fd;
    pthread_mutexattr_t mutex_attr;

    /*
     * Create POSIX shared memory.
     */

    shm_fd = shm_open(
        TWIN_SHM_NAME,
        O_CREAT | O_RDWR,
        0666);

    if (shm_fd == -1)
    {
        perror("[SYNC] shm_open");
        return -1;
    }

    /*
     * Set the shared memory size.
     */

    if (ftruncate(
            shm_fd,
            sizeof(twin_state_t)) == -1)
    {
        perror("[SYNC] ftruncate");
        close(shm_fd);
        return -1;
    }

    /*
     * Map shared memory.
     */

    twin_state = mmap(
        NULL,
        sizeof(twin_state_t),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        shm_fd,
        0);

    close(shm_fd);

    if (twin_state == MAP_FAILED)
    {
        perror("[SYNC] mmap");
        twin_state = NULL;
        return -1;
    }

    /*
     * Clear shared state.
     */

    memset(
        twin_state,
        0,
        sizeof(twin_state_t));

    /*
     * Initialize mutex attributes.
     */

    if (pthread_mutexattr_init(&mutex_attr) != 0)
    {
        printf("[SYNC] Mutex attribute initialization failed\n");
        return -1;
    }

    /*
     * Make mutex process-shared.
     */

    if (pthread_mutexattr_setpshared(
            &mutex_attr,
            PTHREAD_PROCESS_SHARED) != 0)
    {
        printf("[SYNC] Failed to set process-shared mutex\n");

        pthread_mutexattr_destroy(&mutex_attr);

        return -1;
    }

    /*
     * Initialize shared mutex.
     */

    if (pthread_mutex_init(
            &twin_state->mutex,
            &mutex_attr) != 0)
    {
        printf("[SYNC] Mutex initialization failed\n");

        pthread_mutexattr_destroy(&mutex_attr);

        return -1;
    }

    pthread_mutexattr_destroy(&mutex_attr);

    /*
     * Initialize Digital Twin values.
     */

    twin_state->total_messages = 0;
    twin_state->sequence_gaps = 0;
    twin_state->last_latency_ms = 0.0;

    twin_state->environment_fault = FAULT_NONE;
    twin_state->traffic_fault = FAULT_NONE;
    twin_state->energy_fault = FAULT_NONE;
    twin_state->water_fault = FAULT_NONE;

    twin_state->acquisition_alive = 1;
    twin_state->synchronizer_alive = 1;

    return 0;
}


/*
 * Update the Digital Twin using a received sensor message.
 */

static void update_twin_state(sensor_msg_t *msg)
{
    struct timespec receive_time;
    uint64_t previous_sequence = 0;

    /*
     * Record receive time.
     */

    clock_gettime(
        CLOCK_MONOTONIC,
        &receive_time);

    /*
     * Protect shared state.
     */

    pthread_mutex_lock(
        &twin_state->mutex);

    /*
     * Calculate synchronization latency.
     */

    twin_state->last_latency_ms =
        calculate_latency_ms(
            &msg->timestamp,
            &receive_time);

    /*
     * Count received messages.
     */

    twin_state->total_messages++;

    /*
     * Environment sensor.
     */

    if (msg->sensor_id == SENSOR_ENVIRONMENT)
    {
        previous_sequence =
            twin_state->environment.sequence;

        if (previous_sequence != 0 &&
            msg->sequence > previous_sequence + 1)
        {
            twin_state->sequence_gaps++;

            printf(
                "[SYNC] Environment sequence gap: "
                "previous=%llu current=%llu\n",
                (unsigned long long)previous_sequence,
                (unsigned long long)msg->sequence);

            twin_state->environment_fault =
                FAULT_SEQUENCE_GAP;
        }
        else
        {
            twin_state->environment_fault =
                FAULT_NONE;
        }

        twin_state->environment.temperature =
            msg->value1;

        twin_state->environment.pressure =
            msg->value2;

        twin_state->environment.sequence =
            msg->sequence;

        twin_state->environment.timestamp =
            msg->timestamp;
    }

    /*
     * Traffic sensor.
     */

    else if (msg->sensor_id == SENSOR_TRAFFIC)
    {
        previous_sequence =
            twin_state->traffic.sequence;

        if (previous_sequence != 0 &&
            msg->sequence > previous_sequence + 1)
        {
            twin_state->sequence_gaps++;

            printf(
                "[SYNC] Traffic sequence gap: "
                "previous=%llu current=%llu\n",
                (unsigned long long)previous_sequence,
                (unsigned long long)msg->sequence);

            twin_state->traffic_fault =
                FAULT_SEQUENCE_GAP;
        }
        else
        {
            twin_state->traffic_fault =
                FAULT_NONE;
        }

        twin_state->traffic.distance =
            msg->value1;

        twin_state->traffic.vehicle_count =
            (int)msg->value2;

        twin_state->traffic.traffic_level =
            msg->value3;

        twin_state->traffic.sequence =
            msg->sequence;

        twin_state->traffic.timestamp =
            msg->timestamp;
    }

    /*
     * Energy sensor.
     */

    else if (msg->sensor_id == SENSOR_ENERGY)
    {
        previous_sequence =
            twin_state->energy.sequence;

        if (previous_sequence != 0 &&
            msg->sequence > previous_sequence + 1)
        {
            twin_state->sequence_gaps++;

            printf(
                "[SYNC] Energy sequence gap: "
                "previous=%llu current=%llu\n",
                (unsigned long long)previous_sequence,
                (unsigned long long)msg->sequence);

            twin_state->energy_fault =
                FAULT_SEQUENCE_GAP;
        }
        else
        {
            twin_state->energy_fault =
                FAULT_NONE;
        }

        twin_state->energy.voltage =
            msg->value1;

        twin_state->energy.current =
            msg->value2;

        twin_state->energy.power =
            msg->value3;

        twin_state->energy.sequence =
            msg->sequence;

        twin_state->energy.timestamp =
            msg->timestamp;
    }

    /*
     * Water sensor.
     */

    else if (msg->sensor_id == SENSOR_WATER)
    {
        previous_sequence =
            twin_state->water.sequence;

        if (previous_sequence != 0 &&
            msg->sequence > previous_sequence + 1)
        {
            twin_state->sequence_gaps++;

            printf(
                "[SYNC] Water sequence gap: "
                "previous=%llu current=%llu\n",
                (unsigned long long)previous_sequence,
                (unsigned long long)msg->sequence);

            twin_state->water_fault =
                FAULT_SEQUENCE_GAP;
        }
        else
        {
            twin_state->water_fault =
                FAULT_NONE;
        }

        twin_state->water.level =
            msg->value1;

        twin_state->water.flow_rate =
            msg->value2;

        twin_state->water.sequence =
            msg->sequence;

        twin_state->water.timestamp =
            msg->timestamp;
    }

    /*
     * Release shared state.
     */

    pthread_mutex_unlock(
        &twin_state->mutex);
}


/*
 * Start the Synchronizer.
 */

int synchronizer_start(void)
{
    name_attach_t *attach;
    sensor_msg_t msg;
    int rcvid;

    /*
     * Initialize Digital Twin state.
     */

    if (initialize_twin_state() == -1)
    {
        return -1;
    }

    /*
     * Register QNX synchronizer service.
     */

    attach = name_attach(
        NULL,
        SYNC_SERVER_NAME,
        0);

    if (attach == NULL)
    {
        perror("[SYNC] name_attach");
        return -1;
    }

    printf("\n");
    printf("========================================\n");
    printf(" SMART CITY DIGITAL TWIN\n");
    printf(" QNX Synchronizer\n");
    printf("========================================\n");

    printf("[SYNC] Service name : %s\n",
           SYNC_SERVER_NAME);

    printf("[SYNC] Priority     : %d\n",
           SYNC_PRIORITY);

    printf("[SYNC] Shared memory: %s\n",
           TWIN_SHM_NAME);

    printf("[SYNC] Waiting for sensor messages...\n");
    printf("\n");

    /*
     * Receive messages continuously.
     */

    while (1)
    {
        memset(
            &msg,
            0,
            sizeof(msg));

        rcvid = MsgReceive(
            attach->chid,
            &msg,
            sizeof(msg),
            NULL);

        if (rcvid == -1)
        {
            perror("[SYNC] MsgReceive");
            continue;
        }

        /*
         * A pulse has no message body.
         */

        if (rcvid == 0)
        {
            continue;
        }

        /*
         * Process sensor data.
         */

        if (msg.msg_type == MSG_SENSOR_DATA)
        {
            update_twin_state(&msg);

            printf(
                "[SYNC] %-12s "
                "SEQ=%llu "
                "V1=%.2f "
                "V2=%.2f "
                "V3=%.2f "
                "LAT=%.3f ms\n",

                msg.sensor_name,

                (unsigned long long)
                msg.sequence,

                msg.value1,
                msg.value2,
                msg.value3,

                twin_state->last_latency_ms);

            /*
             * Reply to Acquisition.
             *
             * MsgSend() waits until this reply.
             */

            if (MsgReply(
                    rcvid,
                    EOK,
                    NULL,
                    0) == -1)
            {
                perror("[SYNC] MsgReply failed");
            }
        }

        /*
         * Process status request.
         */

        else if (msg.msg_type == MSG_STATUS_REQUEST)
        {
            status_reply_t reply;

            memset(
                &reply,
                0,
                sizeof(reply));

            reply.msg_type =
                MSG_STATUS_REPLY;

            pthread_mutex_lock(
                &twin_state->mutex);

            reply.total_messages =
                twin_state->total_messages;

            reply.sequence_gaps =
                twin_state->sequence_gaps;

            reply.last_latency_ms =
                twin_state->last_latency_ms;

            pthread_mutex_unlock(
                &twin_state->mutex);

            /*
             * Send status response.
             */

            if (MsgReply(
                    rcvid,
                    EOK,
                    &reply,
                    sizeof(reply)) == -1)
            {
                perror("[SYNC] Status MsgReply failed");
            }
        }

        /*
         * Unknown message type.
         */

        else
        {
            MsgError(
                rcvid,
                ENOSYS);
        }
    }

    name_detach(
        attach,
        0);

    return 0;
}
