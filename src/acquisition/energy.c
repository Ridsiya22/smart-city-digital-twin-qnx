/*
 * energy.c
 *
 * Simulated energy sensor.
 *
 * Generates voltage, current and power values.
 */

#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include "common.h"
#include "messages.h"


/*
 * Simulated electrical values.
 */
static double voltage = 12.0;
static double current = 1.50;


/*
 * Controlled voltage changes.
 */
static const double voltage_change[] =
{
    0.1, -0.1, 0.2, -0.2, 0.1, 0.0
};


/*
 * Controlled current changes.
 */
static const double current_change[] =
{
    0.05, -0.03, 0.08, -0.05, 0.02, 0.04
};


static int voltage_index = 0;
static int current_index = 0;


/*
 * Generate one energy sensor message.
 */
void energy_read(sensor_msg_t *msg)
{
    double power;


    msg->msg_type = MSG_SENSOR_DATA;

    msg->sensor_id = SENSOR_ENERGY;


    snprintf(msg->sensor_name,
             SENSOR_NAME_LENGTH,
             "%s",
             ENERGY_NAME);


    /*
     * Change simulated voltage.
     */
    voltage += voltage_change[voltage_index];

    voltage_index++;

    if (voltage_index >= 6)
    {
        voltage_index = 0;
    }


    /*
     * Change simulated current.
     */
    current += current_change[current_index];

    current_index++;

    if (current_index >= 6)
    {
        current_index = 0;
    }


    /*
     * Keep voltage within limits.
     */
    if (voltage < MIN_VOLTAGE)
    {
        voltage = MIN_VOLTAGE;
    }

    if (voltage > MAX_VOLTAGE)
    {
        voltage = MAX_VOLTAGE;
    }


    /*
     * Keep current within limits.
     */
    if (current < MIN_CURRENT)
    {
        current = MIN_CURRENT;
    }

    if (current > MAX_CURRENT)
    {
        current = MAX_CURRENT;
    }


    /*
     * Calculate electrical power.
     *
     * Power = Voltage × Current
     */
    power = voltage * current;


    msg->value1 = voltage;
    msg->value2 = current;
    msg->value3 = power;


    /*
     * Record sensor update time.
     */
    clock_gettime(CLOCK_MONOTONIC,
                  &msg->timestamp);
}
