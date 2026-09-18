/*
 * environment.c
 *
 * Simulated environment sensor.
 *
 * Generates realistic temperature and pressure values
 * for the Smart City Digital Twin.
 */

#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include "common.h"
#include "messages.h"

/*
 * Simulated environment values.
 */
static double temperature = 28.0;
static double pressure = 1008.0;

/*
 * Small controlled changes are used instead of completely
 * random values so that the simulation behaves realistically.
 */
static const double temperature_change[] =
{
    0.2, -0.1, 0.3, 0.1, -0.2, 0.2
};

static const double pressure_change[] =
{
    0.5, -0.3, 0.2, -0.4, 0.3, 0.1
};

static int temperature_index = 0;
static int pressure_index = 0;

/*
 * Generate one environment sensor message.
 */
void environment_read(sensor_msg_t *msg)
{
    msg->msg_type = MSG_SENSOR_DATA;
    msg->sensor_id = SENSOR_ENVIRONMENT;

    snprintf(msg->sensor_name,
             SENSOR_NAME_LENGTH,
             "%s",
             ENVIRONMENT_NAME);

    /*
     * Change the simulated values gradually.
     */
    temperature += temperature_change[temperature_index];

    temperature_index++;

    if (temperature_index >= 6)
    {
        temperature_index = 0;
    }

    pressure += pressure_change[pressure_index];

    pressure_index++;

    if (pressure_index >= 6)
    {
        pressure_index = 0;
    }

    /*
     * Keep values within application limits.
     */
    if (temperature < MIN_TEMPERATURE)
    {
        temperature = MIN_TEMPERATURE;
    }

    if (temperature > MAX_TEMPERATURE)
    {
        temperature = MAX_TEMPERATURE;
    }

    if (pressure < MIN_PRESSURE)
    {
        pressure = MIN_PRESSURE;
    }

    if (pressure > MAX_PRESSURE)
    {
        pressure = MAX_PRESSURE;
    }

    msg->value1 = temperature;
    msg->value2 = pressure;
    msg->value3 = 0.0;

    /*
     * Timestamp the sensor update.
     */
    clock_gettime(CLOCK_MONOTONIC, &msg->timestamp);
}
