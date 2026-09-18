/*
 * cli.c
 *
 * Command Line Interface for the
 * Smart City Digital Twin.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>

#include "common.h"
#include "twin_state.h"


/*
 * Display the current Digital Twin state.
 */

static void display_twin_state(twin_state_t *state)
{
    pthread_mutex_lock(&state->mutex);

    printf("\033[2J");
    printf("\033[H");

    printf("========================================================\n");
    printf("              SMART CITY DIGITAL TWIN                  \n");
    printf("========================================================\n");

    printf("Platform : QNX 8.0 | Raspberry Pi 4\n");
    printf("Mode     : SENSOR SIMULATION\n");

    printf("--------------------------------------------------------\n");

    printf("SYSTEM STATUS\n");

    printf("Acquisition : %s\n",
           state->acquisition_alive ?
           "RUNNING" : "STOPPED");

    printf("Synchronizer: %s\n",
           state->synchronizer_alive ?
           "RUNNING" : "STOPPED");

    printf("Messages    : %llu\n",
           (unsigned long long)state->total_messages);

    printf("Sequence Gaps: %llu\n",
           (unsigned long long)state->sequence_gaps);

    printf("Sync Latency: %.3f ms\n",
           state->last_latency_ms);

    printf("--------------------------------------------------------\n");

    /*
     * Environment
     */

    printf("ENVIRONMENT\n");

    printf("Temperature : %.2f C\n",
           state->environment.temperature);

    printf("Pressure    : %.2f hPa\n",
           state->environment.pressure);

    printf("Sequence    : %llu\n",
           (unsigned long long)
           state->environment.sequence);

    printf("Fault       : %s\n",
           state->environment_fault ?
           "FAULT" : "NORMAL");

    printf("--------------------------------------------------------\n");

    /*
     * Traffic
     */

    printf("TRAFFIC\n");

    printf("Distance    : %.2f cm\n",
           state->traffic.distance);

    printf("Vehicles    : %d\n",
           state->traffic.vehicle_count);

    printf("Traffic     : %.2f\n",
           state->traffic.traffic_level);

    printf("Sequence    : %llu\n",
           (unsigned long long)
           state->traffic.sequence);

    printf("Fault       : %s\n",
           state->traffic_fault ?
           "FAULT" : "NORMAL");

    printf("--------------------------------------------------------\n");

    /*
     * Energy
     */

    printf("ENERGY\n");

    printf("Voltage     : %.2f V\n",
           state->energy.voltage);

    printf("Current     : %.2f A\n",
           state->energy.current);

    printf("Power       : %.2f W\n",
           state->energy.power);

    printf("Sequence    : %llu\n",
           (unsigned long long)
           state->energy.sequence);

    printf("Fault       : %s\n",
           state->energy_fault ?
           "FAULT" : "NORMAL");

    printf("--------------------------------------------------------\n");

    /*
     * Water
     */

    printf("WATER\n");

    printf("Level       : %.2f %%\n",
           state->water.level);

    printf("Flow Rate   : %.2f\n",
           state->water.flow_rate);

    printf("Sequence    : %llu\n",
           (unsigned long long)
           state->water.sequence);

    printf("Fault       : %s\n",
           state->water_fault ?
           "FAULT" : "NORMAL");

    printf("--------------------------------------------------------\n");

    printf("CLI Priority: %d\n",
           CLI_PRIORITY);

    printf("Press Ctrl+C to exit.\n");

    printf("========================================================\n");

    pthread_mutex_unlock(&state->mutex);
}


/*
 * Start the CLI.
 */

int cli_start(void)
{
    int fd;

    twin_state_t *state;


    printf("[CLI] Starting Smart City Digital Twin CLI\n");


    /*
     * Open the shared memory object.
     */

    fd = shm_open(
        TWIN_SHM_NAME,
        O_RDWR,
        0666);

    if (fd == -1)
    {
        perror("[CLI] shm_open failed");

        return 1;
    }


    /*
     * Map TwinState into this process.
     */

    state = mmap(
        NULL,
        sizeof(twin_state_t),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0);

    if (state == MAP_FAILED)
    {
        perror("[CLI] mmap failed");

        close(fd);

        return 1;
    }


    printf("[CLI] Connected to TwinState\n");
    printf("[CLI] Priority : %d\n",
           CLI_PRIORITY);


    /*
     * Continuously display the
     * current Digital Twin state.
     */

    while (1)
    {
        display_twin_state(state);

        sleep(1);
    }


    /*
     * Cleanup.
     */

    munmap(
        state,
        sizeof(twin_state_t));

    close(fd);


    return 0;
}
