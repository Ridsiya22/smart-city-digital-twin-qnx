/*
 * common.h
 *
 * Common definitions for the Smart City Digital Twin.
 *
 * The application uses QNX scheduling, IPC, synchronization
 * and timing facilities to maintain a real-time digital twin.
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>


/*
 * Process priorities.
 *
 * Higher priority value means higher scheduling priority.
 *
 * Synchronizer : 30
 * Fault Monitor: 25
 * Acquisition  : 20
 * Dashboard    : 15
 * CLI          : 15
 */

#define SYNC_PRIORITY       30
#define FAULT_PRIORITY      25
#define ACQ_PRIORITY        20
#define DASHBOARD_PRIORITY  15
#define CLI_PRIORITY        15


/*
 * Sensor identifiers.
 */

#define SENSOR_ENVIRONMENT  1
#define SENSOR_TRAFFIC      2
#define SENSOR_ENERGY       3
#define SENSOR_WATER        4


/*
 * Sensor names.
 */

#define ENVIRONMENT_NAME    "Environment"
#define TRAFFIC_NAME        "Traffic"
#define ENERGY_NAME         "Energy"
#define WATER_NAME          "Water"


/*
 * Sensor update periods.
 */

#define ENVIRONMENT_PERIOD_MS   1000
#define TRAFFIC_PERIOD_MS       1000
#define ENERGY_PERIOD_MS        1000
#define WATER_PERIOD_MS         1000


/*
 * Fault monitoring period.
 */

#define FAULT_CHECK_PERIOD_MS   100
#define SENSOR_TIMEOUT_MS       3000


/*
 * Maximum sensor name length.
 */

#define SENSOR_NAME_LENGTH      20


/*
 * Environment validation limits.
 *
 * These are application-level simulation limits and are not
 * intended to represent the complete physical limits of
 * the BMP280 device.
 */

#define MIN_TEMPERATURE         -20.0
#define MAX_TEMPERATURE          60.0

#define MIN_PRESSURE             850.0
#define MAX_PRESSURE             1100.0


/*
 * Traffic validation limits.
 */

#define MIN_TRAFFIC_DISTANCE     2.0
#define MAX_TRAFFIC_DISTANCE     400.0


/*
 * Energy validation limits.
 */

#define MIN_VOLTAGE              0.0
#define MAX_VOLTAGE              30.0

#define MIN_CURRENT              0.0
#define MAX_CURRENT              20.0


/*
 * Water validation limits.
 */

#define MIN_WATER_LEVEL          0.0
#define MAX_WATER_LEVEL          100.0

#define MIN_FLOW_RATE            0.0
#define MAX_FLOW_RATE            100.0


/*
 * Sensor simulation mode.
 *
 * 1 = simulated sensor values
 * 0 = physical sensor implementation
 */

#define SENSOR_SIMULATION        1


/*
 * QNX synchronizer service name.
 */

#define SYNC_SERVER_NAME         "smart_city_sync"

#endif
