/*
 * twin_state.h
 *
 * Shared Digital Twin state.
 */

#ifndef TWIN_STATE_H
#define TWIN_STATE_H

#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>

#define TWIN_SHM_NAME "/smart_city_twin"


/*
 * Environment state.
 */

typedef struct
{
    double temperature;
    double pressure;

    uint64_t sequence;

    struct timespec timestamp;

} environment_state_t;


/*
 * Traffic state.
 */

typedef struct
{
    double distance;
    int vehicle_count;
    double traffic_level;

    uint64_t sequence;

    struct timespec timestamp;

} traffic_state_t;


/*
 * Energy state.
 */

typedef struct
{
    double voltage;
    double current;
    double power;

    uint64_t sequence;

    struct timespec timestamp;

} energy_state_t;


/*
 * Water state.
 */

typedef struct
{
    double level;
    double flow_rate;

    uint64_t sequence;

    struct timespec timestamp;

} water_state_t;


/*
 * Shared Digital Twin state.
 */

typedef struct
{
    /*
     * Process-shared mutex.
     */

    pthread_mutex_t mutex;


    /*
     * Sensor states.
     */

    environment_state_t environment;

    traffic_state_t traffic;

    energy_state_t energy;

    water_state_t water;


    /*
     * Synchronization statistics.
     */

    uint64_t total_messages;

    uint64_t sequence_gaps;

    double last_latency_ms;


    /*
     * Fault status.
     */

    int environment_fault;

    int traffic_fault;

    int energy_fault;

    int water_fault;


    /*
     * Process status.
     */

    int acquisition_alive;

    int synchronizer_alive;


    /*
     * PID of the current Acquisition process.
     *
     * Fault Monitor uses this value to distinguish
     * Acquisition death from unrelated process deaths.
     */

    pid_t acquisition_pid;

} twin_state_t;

#endif
