#include "core_api.h"
#include "../include/i2c_LSM6DSR.h"
#include "../include/i2c_api.h"

INT32 st_i2c_read(void* handle, UINT8 reg, UINT8* data, UINT16 len){
    I2C_handle_t *local_handle = (I2C_handle_t *)handle;
    UINT16 local_reg = reg;

    if(iot_i2c_read(
        local_handle->port, 
        local_handle->addr,
        &local_reg,
        data,
        (UINT32)len) != 1){
            return -1;
    }
    else{
        return 0;
    }
}

INT32 st_i2c_write(void* handle, UINT8 reg, UINT8* data, UINT16 len){
    I2C_handle_t *local_handle = (I2C_handle_t *)handle;
    UINT16 local_reg = reg;

    if(iot_i2c_write(
        local_handle->port, 
        local_handle->addr,
        &local_reg,
        data,
        (UINT32)len) != 1){
            return -1;
    }
    else{
        return 0;
    }   
    return 0;
}
I2C_handle_t st_i2c_handle={
    DEMO_I2C_PORT,
    LSM6DSR_I2C_ADDR
};

lsm6dsr_ctx_t st_ctx = {
    st_i2c_write,
    st_i2c_read,
    &st_i2c_handle
};


int read_LSM6DSR_WHOAMI(void * L){
    UINT8 reg =  LSM6DSR_WHO_AM_I;
    UINT8 data;
    int rtn=-1;

    for(int i =0; i< 3; i++){
        rtn = st_i2c_read(st_ctx.handle, reg, &data, 1);

        if(rtn == 0){ break;}
    }



    if(rtn == 0){
        OPENAT_lua_print("Read who am i, %02x",  data);
    }else{
        OPENAT_lua_print("Read who am i failed %02x", data);
    }

    return 0;
}




