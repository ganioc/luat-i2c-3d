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
 * @brief  Software reset. Restore the default values in user registers.[set]
 *
 * @param  ctx    Read / write interface definitions.(ptr)
 * @param  val    Change the values of sw_reset in reg CTRL3_C
 * @retval        Interface status (MANDATORY: return 0 -> no Error).
 *
 */
int reset_set(uint8 val)
{
    lsm6dsr_ctrl3_c_t ctrl3_c;

    if (lsm6dsr_read_reg(&reg_ctx, LSM6DSR_CTRL3_C, (uint8 *)&ctrl3_c, 1) != LSM6DSR_OK)
    {
        return LSM6DSR_ERROR;
    }

    ctrl3_c.sw_reset = (uint8)val;
    if (lsm6dsr_write_reg(&reg_ctx, LSM6DSR_CTRL3_C, (uint8 *)&ctrl3_c, 1) != LSM6DSR_OK)
    {
        return LSM6DSR_ERROR;
    }
    return LSM6DSR_OK;
}
/**
 * @brief  Software reset. Restore the default values in user registers.[get]
 *
 * @param  ctx    Read / write interface definitions.(ptr)
 * @param  val    Change the values of sw_reset in reg CTRL3_C
 * @retval        Interface status (MANDATORY: return 0 -> no Error).
 *
 */
int reset_get(lsm6dsr_ctx_t *ctx, uint8 *val)
{
    lsm6dsr_ctrl3_c_t ctrl3_c;

    if (lsm6dsr_read_reg(&reg_ctx, LSM6DSR_CTRL3_C, (uint8 *)&ctrl3_c, 1) != LSM6DSR_OK)
    {
        return LSM6DSR_ERROR;
    }
    *val = ctrl3_c.sw_reset;

    return LSM6DSR_OK;
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

///////////////////////
// Activity scenario
///////////////////////
int LSM6DSR_activity_begin(void *L)
{
    uint8 whoamI, rst;
    lsm6dsr_pin_int1_route_t int1_route;

    /* Check device ID */
    lsm6dsr_device_id_get(&reg_ctx, &whoamI);

    OPENAT_lua_print("whoamI: %02x", whoamI);

    /* Restore default configuration */
    lsm6dsr_reset_set(&reg_ctx, PROPERTY_ENABLE);
    do
    {
        lsm6dsr_reset_get(&reg_ctx, &rst);
    } while (rst);

    /* Disable I3C interface */
    lsm6dsr_i3c_disable_set(&reg_ctx, LSM6DSR_I3C_DISABLE);
    /* Set XL and Gyro Output Data Rate */
    lsm6dsr_xl_data_rate_set(&reg_ctx, LSM6DSR_XL_ODR_208Hz);
    lsm6dsr_gy_data_rate_set(&reg_ctx, LSM6DSR_GY_ODR_104Hz);

    /* Set 2g full XL scale and 250 dps full Gyro */
    lsm6dsr_xl_full_scale_set(&reg_ctx, LSM6DSR_2g);
    lsm6dsr_gy_full_scale_set(&reg_ctx, LSM6DSR_250dps);

    /* Set duration for Activity detection to 9.62 ms (= 2 * 1 / ODR_XL) */
    lsm6dsr_wkup_dur_set(&reg_ctx, 0x02);
    /* Set duration for Inactivity detection to 4.92 s (= 2 * 512 / ODR_XL) */
    lsm6dsr_act_sleep_dur_set(&reg_ctx, 0x01);
    /* Set Activity/Inactivity threshold to 62.5 mg */
    lsm6dsr_wkup_threshold_set(&reg_ctx, 0x01);
    /* Set wake ths weight */
    lsm6dsr_wkup_ths_weight_set(&reg_ctx, 0x01);
    /* Inactivity configuration: XL to 12.5 in LP, gyro to Power-Down */
    lsm6dsr_act_mode_set(&reg_ctx, LSM6DSR_XL_12Hz5_GY_PD);
    /* Enable interrupt generation on Inactivity INT1 pin */
    lsm6dsr_pin_int1_route_get(&reg_ctx, &int1_route);
    int1_route.md1_cfg.int1_sleep_change = PROPERTY_ENABLE;
    lsm6dsr_pin_int1_route_set(&reg_ctx, &int1_route);

    return 0;
}
int LSM6DSR_activity_check(void *L)
{
    lsm6dsr_all_sources_t all_source;
    int rtn = 0;
    /* Check if Activity/Inactivity events */
    lsm6dsr_all_sources_get(&reg_ctx, &all_source);

    if (all_source.wake_up_src.sleep_state)
    {
        rtn = 0;
    }

    if (all_source.wake_up_src.wu_ia)
    {
        rtn = 1;
    }
    // OPENAT_lua_print("act check: %d", rtn);
    lua_pushinteger(L, rtn); // push returned value
    return 1;
}

///////////////////////
// Tilt Scenario
///////////////////////
int LSM6DSR_tilt_begin(void *L)
{
    uint8 whoamI, rst;
    lsm6dsr_pin_int2_route_t int2_route;

    /* Check device ID */
    lsm6dsr_device_id_get(&reg_ctx, &whoamI);

    OPENAT_lua_print("whoamI: %02x", whoamI);

    /* Restore default configuration */
    lsm6dsr_reset_set(&reg_ctx, PROPERTY_ENABLE);
    do
    {
        lsm6dsr_reset_get(&reg_ctx, &rst);
    } while (rst);

    /* Disable I3C interface */
    lsm6dsr_i3c_disable_set(&reg_ctx, LSM6DSR_I3C_DISABLE);

    /* Set XL Output Data Rate: The tilt function works at 26 Hz,
     * so the accelerometer ODR must be set at 26 Hz or higher values
     */
    lsm6dsr_xl_data_rate_set(&reg_ctx, LSM6DSR_XL_ODR_26Hz);
    /* Set 2g full XL scale. */
    lsm6dsr_xl_full_scale_set(&reg_ctx, LSM6DSR_2g);
    /* Enable Tilt in embedded function. */
    lsm6dsr_tilt_sens_set(&reg_ctx, PROPERTY_ENABLE);
    /* Uncomment if interrupt generation on Tilt INT2 pin */
    lsm6dsr_pin_int2_route_get(&reg_ctx, &int2_route);
    int2_route.emb_func_int2.int2_tilt = PROPERTY_ENABLE;
    lsm6dsr_pin_int2_route_set(&reg_ctx, &int2_route);

    /* Uncomment to have interrupt latched */
    lsm6dsr_int_notification_set(&reg_ctx, PROPERTY_ENABLE);

    return 0;
}
int LSM6DSR_tilt_check(void *L)
{
    int8 is_tilt, rtn = 0;
    /* Check if Tilt events */
    lsm6dsr_tilt_flag_data_ready_get(&reg_ctx, &is_tilt);

    if (is_tilt)
    {
        rtn = 1;
    }
    lua_pushinteger(L, rtn); // push returned value
    return 1;
}

////////////////////////////
// polling scenario
////////////////////////////
int LSM6DSR_polling_begin(void *L)
{
    uint8 whoamI, rst;

    /* Check device ID */
    lsm6dsr_device_id_get(&reg_ctx, &whoamI);

    OPENAT_lua_print("whoamI: %02x", whoamI);

    /* Restore default configuration */
    lsm6dsr_reset_set(&reg_ctx, PROPERTY_ENABLE);
    do
    {
        lsm6dsr_reset_get(&reg_ctx, &rst);
    } while (rst);

    /* Disable I3C interface */
    lsm6dsr_i3c_disable_set(&reg_ctx, LSM6DSR_I3C_DISABLE);
    lsm6dsr_auto_increment_set(&reg_ctx, PROPERTY_ENABLE);
    /* Enable Block Data Update */
    lsm6dsr_block_data_update_set(&reg_ctx, PROPERTY_ENABLE);
    lsm6dsr_fifo_mode_set(&reg_ctx, LSM6DSR_BYPASS_MODE);
    /* Set Output Data Rate */
    lsm6dsr_xl_data_rate_set(&reg_ctx, LSM6DSR_XL_ODR_12Hz5);
    lsm6dsr_gy_data_rate_set(&reg_ctx, LSM6DSR_GY_ODR_12Hz5);
    /* Set full scale */
    lsm6dsr_xl_full_scale_set(&reg_ctx, LSM6DSR_2g);
    lsm6dsr_gy_full_scale_set(&reg_ctx, LSM6DSR_2000dps);
    /* Configure filtering chain(No aux interface)
     * Accelerometer - LPF1 + LPF2 path
     */
    lsm6dsr_xl_hp_path_on_out_set(&reg_ctx, LSM6DSR_LP_ODR_DIV_100);
    lsm6dsr_xl_filter_lp2_set(&reg_ctx, PROPERTY_ENABLE);

    return 0;
}
float compute_tilt_acos(int16 gx, int16 gy, int16 gz){
    double sum = pow(gx,2)+ pow(gy,2)+ pow(gz,2);
    double square = sqrt(sum);
    double tilt = acos(gz/square);
    return (float) tilt;
}
float compute_tilt_asin(int16 gx, int16 gy, int16 gz){
    double sum = pow(gx,2)+ pow(gy,2)+ pow(gz,2);
    double square = sqrt(sum);
    double square_xy = sqrt(pow(gx,2)+ pow(gy,2));
    double tilt = asin(square_xy/square);
    return (float) tilt;
}
float compute_tilt_atan(int16 gx, int16 gy, int16 gz){
    if(gz == 0){
        return 1.570795;
    }
    double square_xy = sqrt(pow(gx,2)+ pow(gy,2));
    double tilt = atan2(square_xy, gz);
    return (float) tilt;
}
/*
** output, 0~3.14, tilt value

*/
int LSM6DSR_polling_check(void *L)
{
    uint8 reg, rtn = 0;
    axis3bit16_t data_raw;
    int16 data_raw_acceleration[3];
    float acceleration_mg[3]={0,0,0};
    /* Read output only if new xl value is available */
    lsm6dsr_xl_flag_data_ready_get(&reg_ctx, &reg);

    if (reg)
    {
        rtn = 1;
        /* Read acceleration field data */
        // memset(data_raw_acceleration, 0x00, 3 * sizeof(int16));
        // lsm6dsr_acceleration_raw_get(&reg_ctx, data_raw_acceleration);
        lsm6dsr_read_reg(&reg_ctx, LSM6DSR_OUTX_L_A, &data_raw.u8bit[0], 1);
        lsm6dsr_read_reg(&reg_ctx, LSM6DSR_OUTX_H_A, &data_raw.u8bit[1], 1);
        lsm6dsr_read_reg(&reg_ctx, LSM6DSR_OUTY_L_A, &data_raw.u8bit[2], 1);
        lsm6dsr_read_reg(&reg_ctx, LSM6DSR_OUTY_H_A, &data_raw.u8bit[3], 1);
        lsm6dsr_read_reg(&reg_ctx, LSM6DSR_OUTZ_L_A, &data_raw.u8bit[4], 1);
        lsm6dsr_read_reg(&reg_ctx, LSM6DSR_OUTZ_H_A, &data_raw.u8bit[5], 1);
        /* Format the data. */
        data_raw_acceleration[0] = data_raw.i16bit[0];
        data_raw_acceleration[1] = data_raw.i16bit[1];
        data_raw_acceleration[2] = data_raw.i16bit[2];

        acceleration_mg[0] =compute_tilt_acos(data_raw_acceleration[0], data_raw_acceleration[1], data_raw_acceleration[2]);
        acceleration_mg[1] =compute_tilt_asin(data_raw_acceleration[0], data_raw_acceleration[1], data_raw_acceleration[2]);
        acceleration_mg[2] =compute_tilt_atan(data_raw_acceleration[0], data_raw_acceleration[1], data_raw_acceleration[2]);

        OPENAT_lua_print("%f %f %f", acceleration_mg[0],acceleration_mg[1], acceleration_mg[2]);

        // acceleration_mg[0] =
        //     lsm6dsr_from_fs2g_to_mg(data_raw_acceleration[0]);
        // acceleration_mg[1] =
        //     lsm6dsr_from_fs2g_to_mg(data_raw_acceleration[1]);
        // acceleration_mg[2] =
        //     lsm6dsr_from_fs2g_to_mg(data_raw_acceleration[2]);
    }

    lua_pushinteger(L, rtn);
    lua_pushinteger(L, (int) (acceleration_mg[0] * 100000));
    lua_pushinteger(L, (int) (acceleration_mg[1] * 100000));
    lua_pushinteger(L, (int) (acceleration_mg[2] * 100000));

    // lua_pushnumber(L, acceleration_mg[0]);
    // lua_pushnumber(L, acceleration_mg[1]);
    // lua_pushnumber(L, acceleration_mg[2]);

    // lua_pushinteger(L, data_raw_acceleration[0]);
    // lua_pushinteger(L, data_raw_acceleration[1]);
    // lua_pushinteger(L, data_raw_acceleration[2]);
    return 4;
}