#ifndef __CLIENT_CAN_H__
#define __CLIENT_CAN_H__

#include "my_math.h"
#define A_VOLT_CURR_POWER           0x20
#define B_VOLT_CURR_POWER           0x21
#define TEMP_READINGS               0x22
#define POWER_CHARGING_MODE         0x23
#define FAULT_ALARM_SIGNALS         0x24
#define AB_COMPEN_TEMP              0x25
#define CHARGING_STATUS             0x26
#define FACTORY_RESET               0x40
#define POWER_STATUS                0x41
#define POWER_MODE                  0x42
#define CHARGING_MODE               0x43
#define A_BATTERY_TYPE              0x44
#define B_BATTERY_TYPE              0x45
#define BOOT_TIME_DELAY             0x46
#define SOFT_START_TIME_DELAY       0x47
#define TERMINAL_A_VOLTAGE          0x80
#define TERMINAL_A_CURRENT          0x81
#define TERMINAL_A_POWER            0x82
#define TERMINAL_B_VOLT             0x83
#define TERMINAL_B_CURR             0x84
#define TERMINAL_B_POWER            0x85
#define TERMINAL_A_UNDER            0x86
#define TERMINAL_A_UNDER_R          0x87
#define TERMINAL_A_OVER             0x88
#define TERMINAL_A_OVER_R           0x89
#define TERMINAL_B_UNDER            0x8A
#define TERMINAL_B_UNDER_R          0x8B
#define TERMINAL_B_OVER             0x8C
#define TERMINAL_B_OVER_R           0x8D
#define OVER_TEMPERATURE            0x8E
#define TERMINAL_A_CHARGING_LIGHT   0x8F
#define TERMINAL_B_CHARGING_LIGHT   0x90
#define AUTO_CHARGE_FORWARD_OPEN    0x91
#define AUTO_CHARGE_FORWARD_VEER    0x92
#define AUTO_CHARGE_FORWARD_SHUT    0x93
#define AUTO_CHARGE_REVERSE_OPEN    0x94
#define AUTO_CHARGE_REVERSE_SHUT    0x95
#define SET_TEMP_INNER              0x96
#define MOSFET_CONTROL_300A         0x10
#define MOSFET_CONTROL_150A         0x12

void Get_CAN_Communications_Content (uint32_t can_id, uint8_t *data, uint8_t len);
#endif

