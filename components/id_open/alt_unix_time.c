/* -*- tab-width: 2; mode: c; -*-
 * 
 * Seconds since 1/1/1970 for systems that don't have the unix time() function. 
 * 
 * The Julian day algorithm is from Jean Meeus's Astronomical Algorithms
 * as are the two test dates.
 * 
 * Converted for ESP-IDF (no Arduino dependency)
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

uint64_t      alt_unix_secs(int,int,int,int,int,int);
static double julian_day(int,int,float,int);

/*
 * Calculate Unix timestamp (seconds since 1970-01-01 00:00:00 UTC)
 * from date and time components
 */

uint64_t alt_unix_secs(int year, int month, int day, int hour, int minute, int second) {
    
    double jd, jd1970;
    uint64_t secs;
    
    // Julian Day of the given date
    jd = julian_day(year, month, (float)day, 1);
    
    // Julian Day of 1970-01-01
    jd1970 = julian_day(1970, 1, 1.0, 1);
    
    // Days since 1970-01-01
    double days = jd - jd1970;
    
    // Convert to seconds and add time of day
    secs = (uint64_t)(days * 86400.0) + (hour * 3600) + (minute * 60) + second;
    
    return secs;
}

/*
 * Calculate Julian Day Number (JD)
 * From Jean Meeus's Astronomical Algorithms
 */

static double julian_day(int year, int month, float day, int utc_flag) {
    
    double jd, a, b;
    
    if (month <= 2) {
        year -= 1;
        month += 12;
    }
    
    if (utc_flag) {
        a = floor((double)year / 100.0);
        b = 2.0 - a + floor(a / 4.0);
    } else {
        b = 0.0;
    }
    
    jd = floor(365.25 * (double)(year + 4716)) + 
         floor(30.6001 * (double)(month + 1)) + 
         (double)day + b - 1524.5;
    
    return jd;
}

/*
 * Test function (can be compiled standalone for verification)
 */

#if 0

int main(int argc, char *argv[]) {
    
    uint64_t alt_secs;
    time_t unix_secs;
    struct tm *gmt;
    
    time(&unix_secs);
    
    gmt = gmtime(&unix_secs);
    
    alt_secs = alt_unix_secs(1900 + gmt->tm_year, 1 + gmt->tm_mon, gmt->tm_mday,
                             gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
    
    printf("\nunix: %10lu\n", (unsigned long)unix_secs);
    printf("alt:  %10lu\n", (unsigned long)alt_secs);
    printf("      %10ld\n", (long)((int64_t)unix_secs - (int64_t)alt_secs));
    
    return 0;
}

#endif
