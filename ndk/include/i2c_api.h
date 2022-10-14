#ifndef I2C_DEMO_H
#define I2C_DEMO_H

#include "core_api.h"

// #define i2c_print OPENAT_lua_print
#define DEMO_I2C_PORT OPENAT_I2C_2
#define I2CSLAVEADDR 0x51


// For PCF8563T RTC IC
#define SLAVE_ADDR  0x51
#define CTRL1_ADDR  0
#define CTRL2_ADDR  1
#define SEC_ADDR  2
#define MIN_ADDR  3
#define HOUR_ADDR  4
#define DAY_ADDR  5
#define WDAY_ADDR  6
#define MONTH_ADDR  7
#define YEAR_ADDR  8
#define TIMER_CTRL_ADDR  0xE
#define TIMER_ADDR  0xF


UINT32 iot_i2c_write(E_AMOPENAT_I2C_PORT port, UINT8 salveAddr,  UINT16 *pRegAddr,  UINT8 *buf,UINT32 bufLen);

UINT32 iot_i2c_read(E_AMOPENAT_I2C_PORT port, UINT8 slaveAddr,  UINT16 *pRegAddr, UINT8 *buf, UINT32 bufLen);




#endif