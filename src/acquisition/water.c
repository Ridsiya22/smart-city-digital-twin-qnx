/*
 * water.c
 *
 * Simulated water sensor.
 *
 * Generates changing water level and flow rate values.
 */

#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include "common.h"
#include "messages.h"


/*
 * Simulated water level.
 */
static double level = 72.0;


/*
 * Controlled water-level changes.
 */
static const double level_change[] =
{
    -1.0, 0.5, -0.5, 1.0, -0.8, 0.3
};

static int level_index = 0;


/*
 * Generate one water sensor message.
 */
void water_read(sensor_msg_t *msg)
{
    double flow_rate;


    msg->msg_type = MSG_SENSOR_DATA;

    msg->sensor_id = SENSOR_WATER;


    snprintf(msg->sensor_name,
             SENSOR_NAME_LENGTH,
             "%s",
             WATER_NAME);


    /*
     * Change water level gradually.
     */
    level += level_change[level_index];

    level_index++;

    if (level_index >= 6)
    {
        level_index = 0;
    }


    /*
     * Keep water level between 0 and 100 percent.
     */
    if (level < MIN_WATER_LEVEL)
    {
        level = MIN_WATER_LEVEL;
    }

    if (level > MAX_WATER_LEVEL)
    {
        level = MAX_WATER_LEVEL;
    }


    /*
     * Calculate simulated flow rate.
     */
    flow_rate =
        15.0 + ((100.0 - level) * 0.20);


    if (flow_rate < MIN_FLOW_RATE)
    {
        flow_rate = MIN_FLOW_RATE;
    }

    if (flow_rate > MAX_FLOW_RATE)
    {
        flow_rate = MAX_FLOW_RATE;
    }


    msg->value1 = level;
    msg->value2 = flow_rate;
    msg->value3 = 0.0;


    /*
     * Record sensor update time.
     */
    clock_gettime(CLOCK_MONOTONIC,
                  &msg->timestamp);
}
