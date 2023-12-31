#ifndef I2C_QMI8658_H
#define I2C_QMI8658_H


#include "core_api.h"

#define  QMI8658_I2C_PORT   OPENAT_I2C_2
#define  QMI8658_I2C_ADDR   0x6B

#define QMI8658_WHO_AM_I 

#define QMI8658_MAX_DIGITS  1000000

typedef union{
  INT16 i16bit;
  UINT8 u8bit[2];
} byte1bit16_t;


int QMI8658_polling_begin(void *L);
int QMI8658_polling_check(void *L);
int QMI8658_polling_acc(void *L);
int QMI8658_read_temp(void *L);
int QMI8658_read_timestamp(void*L);
int QMI8658_polling_acc(void *L);
int QMI8658_polling_z_tilt(void *L);
#endif
