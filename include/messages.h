/*
 * messages.h
 *
 * QNX IPC message definitions used by the
 * Smart City Digital Twin.
 */

#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdint.h>
#include <time.h>

#include "common.h"


/*
 * Message types.
 *
 * The message type is the first member of every
 * QNX application message structure.
 */

#define MSG_SENSOR_DATA       1
#define MSG_STATUS_REQUEST    2
#define MSG_STATUS_REPLY      3


/*
 * Sensor data message.
 *
 * One common structure is used for all four
 * simulated sensor sources.
 */

typedef struct
{
    uint16_t msg_type;
    uint16_t sensor_id;

    char sensor_name[SENSOR_NAME_LENGTH];

    double value1;
    double value2;
    double value3;

    uint64_t sequence;

    struct timespec timestamp;

} sensor_msg_t;


/*
 * Status request message.
 */

typedef struct
{
    uint16_t msg_type;

} status_request_t;


/*
 * Status reply message.
 */

typedef struct
{
    uint16_t msg_type;

    uint64_t total_messages;
    uint64_t sequence_gaps;

    double last_latency_ms;

} status_reply_t;
/*
 * Sensor simulation functions.
 */
void environment_read(sensor_msg_t *msg);
void traffic_read(sensor_msg_t *msg);
void energy_read(sensor_msg_t *msg);
void water_read(sensor_msg_t *msg);

#endif
