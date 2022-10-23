#include "../include/i2c_QMI8658.h"
#include "../include/qmi8658_reg.h"


/* success returns 0 */
int qmi_i2c_read( UINT8 reg, UINT8 *data, UINT16 len){
    UINT16 local_reg = reg;

    if (iot_i2c_read(
            QMI8658_I2C_PORT,
            QMI8658_I2C_ADDR,
            &local_reg,
            data,
            (UINT32)len) != 1)
    {
        OPENAT_lua_print("iot_i2c_read error");
        return -1;
    }
    return 0;
}
/**
 * Succeed return 0 otherwise -1
 */
int qmi_i2c_write(UINT8 reg, UINT8 *data, UINT16 len)
{
    UINT16 local_reg = reg;

    if (iot_i2c_write(
            QMI8658_I2C_PORT,
            QMI8658_I2C_ADDR,
            &local_reg,
            data,
            (UINT32)len) != 1)
    {
        return -1;
    }

    return 0;
}
int read_QMI8658_WHOAMI(void *L)
{
    UINT8 reg = Qmi8658Register_WhoAmI;
    UINT8 data;
    int rtn = -1;

    for (int i = 0; i < 3; i++)
    {
        rtn = qmi_i2c_read( reg, &data, 1);

        if (rtn == 0)
        {
            break;
        }
    }

    if (rtn == 0)
    {
        OPENAT_lua_print("Read who am i, %02x", data);
    }
    else
    {
        OPENAT_lua_print("Read who am i failed %02x", data);
    }

    return 0;
}
int read_QMI8658_temp(void *L){
    UINT8 reg = Qmi8658Register_Tempearture_L;
    byte1bit16_t data_raw;
    int16 data;
    int rtn = -1;
    float temp_f;

    rtn = qmi_i2c_read( reg, &data_raw.u8bit[0], 1);
    reg = Qmi8658Register_Tempearture_H;
    qmi_i2c_read( reg, &data_raw.u8bit[1], 1);
    data = data_raw.i16bit;

    temp_f = (float)data/256.0f;
    OPENAT_lua_print("temp %f", temp_f);

    return 0;
}
int QMI8658_polling_begin(void *L){


    OPENAT_lua_print("QMI8658_polling_begin");
    
    
    read_QMI8658_WHOAMI(L);
    read_QMI8658_temp(L);

    return 1;
}
int QMI8658_polling_check(void *L){

    return 1;
}
int QMI8658_polling_acc(void *L){

    return 1;
}




