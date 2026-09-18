/*
 * smart_city_digital_twin.c
 *
 * Smart City Digital Twin main launcher.
 */

#include <stdio.h>
#include <string.h>

#include "common.h"


/*
 * Module start functions.
 */

int acquisition_start(void);

int synchronizer_start(void);

int fault_monitor_start(void);

int deadlock_start(void);


/*
 * Main launcher.
 */

int main(
    int argc,
    char *argv[])
{
    if (argc < 2)
    {
        printf(
            "Usage: %s "
            "[sync|acq|fault|deadlock|dashboard|cli]\n",
            argv[0]);

        return 1;
    }


    /*
     * Start Synchronizer.
     */

    if (strcmp(
            argv[1],
            "sync") == 0)
    {
        return synchronizer_start();
    }


    /*
     * Start Acquisition.
     */

    if (strcmp(
            argv[1],
            "acq") == 0)
    {
        return acquisition_start();
    }


    /*
     * Start Fault Monitor.
     */

    if (strcmp(
            argv[1],
            "fault") == 0)
    {
        return fault_monitor_start();
    }


    /*
     * Start Deadlock demonstration.
     */

    if (strcmp(
            argv[1],
            "deadlock") == 0)
    {
        return deadlock_start();
    }


    printf(
        "Selected module: %s\n",
        argv[1]);

    return 0;
}
