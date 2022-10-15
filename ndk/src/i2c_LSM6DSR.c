#include "core_api.h"
#include "../include/i2c_LSM6DSR.h"
#include "../include/i2c_api.h"
#include "../include/lsm6dsr_reg.h"

// 0 disabled, 1 enabled
static int acc_is_enabled = 0;
static int gyro_is_enabled = 0;
static int acc_odr = LSM6DSR_XL_ODR_104Hz;
static int gyro_odr = LSM6DSR_GY_ODR_104Hz;

INT32 st_i2c_read(void *handle, UINT8 reg, UINT8 *data, UINT16 len)
{
    I2C_handle_t *local_handle = (I2C_handle_t *)handle;
    UINT16 local_reg = reg;

    if (iot_i2c_read(
            local_handle->port,
            local_handle->addr,
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
INT32 st_i2c_write(void *handle, UINT8 reg, UINT8 *data, UINT16 len)
{
    I2C_handle_t *local_handle = (I2C_handle_t *)handle;
    UINT16 local_reg = reg;

    if (iot_i2c_write(
            local_handle->port,
            local_handle->addr,
            &local_reg,
            data,
            (UINT32)len) != 1)
    {
        return -1;
    }

    return 0;
}
I2C_handle_t st_i2c_handle = {
    DEMO_I2C_PORT,
    LSM6DSR_I2C_ADDR};

lsm6dsr_ctx_t reg_ctx = {
    st_i2c_write,
    st_i2c_read,
    &st_i2c_handle};

int read_LSM6DSR_WHOAMI(void *L)
{
    UINT8 reg = LSM6DSR_WHO_AM_I;
    UINT8 data;
    int rtn = -1;

    for (int i = 0; i < 3; i++)
    {
        rtn = st_i2c_read(reg_ctx.handle, reg, &data, 1);

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
/**
 * @brief  Configure the sensor in order to be used
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSR_begin(void *L)
{
    int rtn = -1;
    // Disable I3C
    if (lsm6dsr_i3c_disable_set(&reg_ctx, LSM6DSR_I3C_DISABLE) != LSM6DSR_OK)
    {
        goto END_BEGIN;
    }

    /* Enable register address automatically incremented during a multiple byte
access with a serial interface. */
    if (lsm6dsr_auto_increment_set(&reg_ctx, PROPERTY_ENABLE) != LSM6DSR_OK)
    {
        goto END_BEGIN;
    }

    /* Enable BDU */
    if (lsm6dsr_block_data_update_set(&reg_ctx, PROPERTY_ENABLE) != LSM6DSR_OK)
    {
        goto END_BEGIN;
    }

    /* FIFO mode selection */
    if (lsm6dsr_fifo_mode_set(&reg_ctx, LSM6DSR_BYPASS_MODE) != LSM6DSR_OK)
    {
        goto END_BEGIN;
    }

    // 设置ACC ODR速率

    /* Output data rate selection - power down. */
    if (lsm6dsr_xl_data_rate_set(&reg_ctx, LSM6DSR_XL_ODR_OFF) != LSM6DSR_OK)
    {
        goto END_BEGIN;
    }

    /* Full scale selection. */
    if (lsm6dsr_xl_full_scale_set(&reg_ctx, LSM6DSR_2g) != LSM6DSR_OK)
    {
        goto END_BEGIN;
    }

    // 设置Gyro ODR的速率,

    /* Output data rate selection - power down. */
    if (lsm6dsr_gy_data_rate_set(&reg_ctx, LSM6DSR_GY_ODR_OFF) != LSM6DSR_OK)
    {
        goto END_BEGIN;
    }

    /* Full scale selection. */
    if (lsm6dsr_gy_full_scale_set(&reg_ctx, LSM6DSR_2000dps) != LSM6DSR_OK)
    {
        goto END_BEGIN;
    }

    // 当前的acc, ryro都是关闭的,

    rtn = 0;
END_BEGIN:
    lua_pushinteger(L, rtn); // push returned value
    return 1;
}

/**
 * @brief  Disable the sensor and relative resources
 * @retval 0 in case of success, an error code otherwise
 */
int LSM6DSR_end(void *L)
{
    int rtn = -1;

    /* Disable both acc and gyro */
    if (Disable_X(L) != LSM6DSR_OK)
    {
        goto END_END;
    }

    if (Disable_G(L) != LSM6DSR_OK)
    {
        goto END_END;
    }

    rtn = 0;
END_END:
    lua_pushinteger(L, rtn); // push returned value
    return 1;
}
/**
 * @brief  Enable the LSM6DSR accelerometer sensor
 * @retval 0 in case of success, an error code otherwise
 */
int Enable_X(void *L)
{
    int rtn = -1;
    /* Check if the component is already enabled */
    if (acc_is_enabled == 1U)
    {
        goto END_ENABLE_X_NORM;
    }

    /* Output data rate selection. */
    if (lsm6dsr_xl_data_rate_set(&reg_ctx, acc_odr) != LSM6DSR_OK)
    {
        goto END_ENABLE_X;
    }

    acc_is_enabled = 1;

END_ENABLE_X_NORM:
    rtn = 0;
END_ENABLE_X:
    lua_pushinteger(L, rtn); // push returned value
    return 1;
}
/**
 * @brief  Disable the LSM6DSR accelerometer sensor
 * @retval 0 in case of success, an error code otherwise
 */
int Disable_X(void *L)
{
    int rtn = -1;
    /* Check if the component is already disabled */
    if (acc_is_enabled == 0U)
    {
        goto END_DISABLE_X_NORM;
    }

    /* Get current output data rate. */
    if (lsm6dsr_xl_data_rate_get(&reg_ctx, &acc_odr) != LSM6DSR_OK)
    {
        goto END_DISABLE_X;
    }

    /* Output data rate selection - power down. */
    if (lsm6dsr_xl_data_rate_set(&reg_ctx, LSM6DSR_XL_ODR_OFF) != LSM6DSR_OK)
    {
        goto END_DISABLE_X;
    }

    acc_is_enabled = 0;

END_DISABLE_X_NORM:
    rtn = 0;
END_DISABLE_X:
    lua_pushinteger(L, rtn); // push returned value
    return 1;
}
/**
 * @brief  Enable the LSM6DSR gyroscope sensor
 * @retval 0 in case of success, an error code otherwise
 */
int Enable_G(void *L)
{
    int rtn = -1;
    /* Check if the component is already enabled */
    if (gyro_is_enabled == 1U)
    {
        goto END_ENABLE_G_NORM;
    }

    /* Output data rate selection. */
    if (lsm6dsr_gy_data_rate_set(&reg_ctx, gyro_odr) != LSM6DSR_OK)
    {
        goto END_ENABLE_G;
    }

    gyro_is_enabled = 1;

END_ENABLE_G_NORM:
    rtn = 0;
END_ENABLE_G:
    lua_pushinteger(L, rtn); // push returned value
    return 1;
}
/**
 * @brief  Disable the LSM6DSR gyroscope sensor
 * @retval 0 in case of success, an error code otherwise
 */
int Disable_G(void *L)
{
    int rtn = -1;
    /* Check if the component is already disabled */
    if (gyro_is_enabled == 0U)
    {
        goto END_DISABLE_G_NORM;
    }

    /* Get current output data rate. */
    if (lsm6dsr_gy_data_rate_get(&reg_ctx, &gyro_odr) != LSM6DSR_OK)
    {
        goto END_DISABLE_G;
    }

    /* Output data rate selection - power down. */
    if (lsm6dsr_gy_data_rate_set(&reg_ctx, LSM6DSR_GY_ODR_OFF) != LSM6DSR_OK)
    {
        goto END_DISABLE_G;
    }

    gyro_is_enabled = 0;

END_DISABLE_G_NORM:
    rtn = 0;
END_DISABLE_G:
    lua_pushinteger(L, rtn); // push returned value
    return 1;
}

/**
 * @brief  Get the LSM6DSR accelerometer sensor raw axes
 * @param  Value pointer where the raw values of the axes are written
 * @retval 0 in case of success, an error code otherwise
 */
int Get_X_AxesRaw(void *L)
{
    axis3bit16_t data_raw;
    int16 Value[3] = {0, 0, 0};

    /* Read raw data values. */
    // if (lsm6dsr_acceleration_raw_get(&reg_ctx, data_raw.u8bit) != LSM6DSR_OK)
    // {
    //     OPENAT_lua_print("read acc raw failed");
    //     goto END_GET_X;
    // }

    lsm6dsr_read_reg(&reg_ctx, LSM6DSR_OUTX_L_A, &data_raw.u8bit[0], 1);
    lsm6dsr_read_reg(&reg_ctx, LSM6DSR_OUTX_H_A, &data_raw.u8bit[1], 1);
    lsm6dsr_read_reg(&reg_ctx, LSM6DSR_OUTY_L_A, &data_raw.u8bit[2], 1);
    lsm6dsr_read_reg(&reg_ctx, LSM6DSR_OUTY_H_A, &data_raw.u8bit[3], 1);
    lsm6dsr_read_reg(&reg_ctx, LSM6DSR_OUTZ_L_A, &data_raw.u8bit[4], 1);
    lsm6dsr_read_reg(&reg_ctx, LSM6DSR_OUTZ_H_A, &data_raw.u8bit[5], 1);

    /* Format the data. */
    Value[0] = data_raw.i16bit[0];
    Value[1] = data_raw.i16bit[1];
    Value[2] = data_raw.i16bit[2];

END_GET_X:
    lua_pushinteger(L, Value[0]);
    lua_pushinteger(L, Value[1]);
    lua_pushinteger(L, Value[2]);
    return 3;
}

/**
 * @brief  Get the LSM6DSR gyroscope sensor raw axes
 * @param  Value pointer where the raw values of the axes are written
 * @retval 0 in case of success, an error code otherwise
 */
int Get_G_AxesRaw(void *L)
{
    axis3bit16_t data_raw;
    int16 Value[3] = {0, 0, 0};

    /* Read raw data values. */
    // if (lsm6dsr_angular_rate_raw_get(&reg_ctx, data_raw.u8bit) != LSM6DSR_OK)
    // {
    //     OPENAT_lua_print("read gyro raw failed");
    //     goto END_GET_G;
    // }

    lsm6dsr_read_reg(&reg_ctx, LSM6DSR_OUTX_L_G, &data_raw.u8bit[0], 1);
    lsm6dsr_read_reg(&reg_ctx, LSM6DSR_OUTX_H_G, &data_raw.u8bit[1], 1);
    lsm6dsr_read_reg(&reg_ctx, LSM6DSR_OUTY_L_G, &data_raw.u8bit[2], 1);
    lsm6dsr_read_reg(&reg_ctx, LSM6DSR_OUTY_H_G, &data_raw.u8bit[3], 1);
    lsm6dsr_read_reg(&reg_ctx, LSM6DSR_OUTZ_L_G, &data_raw.u8bit[4], 1);
    lsm6dsr_read_reg(&reg_ctx, LSM6DSR_OUTZ_H_G, &data_raw.u8bit[5], 1);

    /* Format the data. */
    Value[0] = data_raw.i16bit[0];
    Value[1] = data_raw.i16bit[1];
    Value[2] = data_raw.i16bit[2];

END_GET_G:
    lua_pushinteger(L, Value[0]);
    lua_pushinteger(L, Value[1]);
    lua_pushinteger(L, Value[2]);
    return 3;
}
