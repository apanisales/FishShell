/**
 * timer.c
 *
 * Contains the get_time() function, which is needed
 * to track how long each process runs.
 */

#include "timer.h"
#include <stdlib.h>

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

