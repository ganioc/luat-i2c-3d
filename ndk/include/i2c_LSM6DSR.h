#ifndef I2C_LSM6DSR_H
#define I2C_LSM6DSR_H

#include "core_api.h"

#define LSM6DSR_I2C_PORT   OPENAT_I2C_2
#define LSM6DSR_I2C_ADDR   0x6A
// #define LSM6DSR_I2C_ADDR   0x6B


#include "./lsm6dsr_reg.h"

struct I2C_handle{
    UINT8 port;
    UINT8 addr;
};
typedef struct I2C_handle I2C_handle_t;

typedef enum
{
  LSM6DSR_OK = 0,
  LSM6DSR_ERROR =-1
} LSM6DSRStatusTypeDef;



int read_LSM6DSR_WHOAMI(void * L);
int LSM6DSR_begin(void *L);
int LSM6DSR_end(void *L);
int Enable_X();
int Disable_X();
int Enable_G();
int Disable_G();



#endif

