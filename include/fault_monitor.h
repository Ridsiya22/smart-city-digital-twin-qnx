/*
 * fault_monitor.h
 *
 * Fault monitoring definitions for the
 * Smart City Digital Twin.
 */

#ifndef FAULT_MONITOR_H
#define FAULT_MONITOR_H

#include <stdint.h>


/*
 * Fault types.
 */

#define FAULT_NONE           0
#define FAULT_STALE_DATA     1
#define FAULT_SEQUENCE_GAP   2
#define FAULT_PROCESS_DEATH  3


/*
 * Fault information.
 */

typedef struct
{
    int fault_type;
    int sensor_id;

    uint64_t detected_sequence;

    double detection_time_ms;

} fault_info_t;


/*
 * Fault monitor thread.
 */

void *fault_monitor_thread(void *arg);


/*
 * Start the Fault Monitor.
 */

int fault_monitor_start(void);

#endif
