/*
 * traffic.c
 *
 * Simulated traffic sensor.
 *
 * Generates changing vehicle distance, vehicle count
 * and traffic level.
 */

#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include "common.h"
#include "messages.h"

/*
 * Simulated distance from the vehicle.
 */
static double distance = 120.0;

/*
 * Controlled vehicle movement.
 */
static const double distance_change[] =
{
    -5.0, 3.0, -8.0, 6.0, -4.0, 2.0
};

static int distance_index = 0;

/*
 * Generate one traffic sensor message.
 */
void traffic_read(sensor_msg_t *msg)
{
    msg->msg_type = MSG_SENSOR_DATA;
    msg->sensor_id = SENSOR_TRAFFIC;

    snprintf(msg->sensor_name,
             SENSOR_NAME_LENGTH,
             "%s",
             TRAFFIC_NAME);

    /*
     * Simulate vehicle movement.
     */
    distance += distance_change[distance_index];

    distance_index++;

    if (distance_index >= 6)
    {
        distance_index = 0;
    }

    /*
     * Keep distance within sensor range.
     */
    if (distance < MIN_TRAFFIC_DISTANCE)
    {
        distance = MIN_TRAFFIC_DISTANCE;
    }

    if (distance > MAX_TRAFFIC_DISTANCE)
    {
        distance = MAX_TRAFFIC_DISTANCE;
    }

    msg->value1 = distance;

    /*
     * Estimate vehicle count.
     */
    if (distance < 50.0)
    {
        msg->value2 = 8;
    }
    else if (distance < 100.0)
    {
        msg->value2 = 5;
    }
    else if (distance < 200.0)
    {
        msg->value2 = 3;
    }
    else
    {
        msg->value2 = 1;
    }

    /*
     * Calculate traffic level.
     */
    if (distance < 50.0)
    {
        msg->value3 = 90.0;
    }
    else if (distance < 100.0)
    {
        msg->value3 = 65.0;
    }
    else if (distance < 200.0)
    {
        msg->value3 = 40.0;
    }
    else
    {
        msg->value3 = 15.0;
    }

    clock_gettime(CLOCK_MONOTONIC, &msg->timestamp);
}
