#ifndef __TEMPERATURE_H
#define __TEMPERATURE_H

#define FIXED_RES 10.0e3f
#define VOLT_MAX 3.3f
#define NTC_OFFSET (-55.0f)

typedef enum
{
    IS_DN_NTC,
    IS_UP_NTC,
} TEMPERATURE_MODE_E;

float cal_ntc_temp(float volt, TEMPERATURE_MODE_E mode);

#endif
