#include "../include/i2c_QMI8658.h"
#include "../include/qmi8658_reg.h"
#include "../include/kalman.h"

static Kalman_t filterAccX, filterAccY, filterAccZ;
static int16 data_raw_acceleration[3];

/* success returns 0 */
int qmi_i2c_read(UINT8 reg, UINT8 *data, UINT16 len)
{
    UINT16 local_reg = reg;

    if (iot_i2c_read(
            QMI8658_I2C_PORT,
            QMI8658_I2C_ADDR,
            &local_reg,
            data,
            (UINT32)len) != len)
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
            (UINT32)len) != len)
    {
        OPENAT_lua_print("iot_i2c_write error");
        return -1;
    }

    return 0;
}

int QMI8658_read_temp(void *L)
{
    UINT8 reg = Qmi8658Register_Tempearture_L;
    byte1bit16_t data_raw;
    int16 data;
    int rtn = -1;
    float temp_f;

    rtn = qmi_i2c_read(reg, &data_raw.u8bit[0],1);
    reg = Qmi8658Register_Tempearture_H;
    qmi_i2c_read(reg, &data_raw.u8bit[1], 1);
    data = data_raw.i16bit;

    temp_f = (float)data / 256.0f;
    OPENAT_lua_print("temp %f", temp_f);

    return 0;
}

int QMI8658_read_timestamp(void *L)
{
    uint8 buf[3];
    uint32 timestamp;
    UINT8 reg;
    reg = Qmi8658Register_Timestamp_L;
    qmi_i2c_read(reg, &buf[0], 1);
    reg = Qmi8658Register_Timestamp_M;
    qmi_i2c_read(reg, &buf[1], 1);
    reg = Qmi8658Register_Timestamp_H;
    qmi_i2c_read(reg, &buf[1], 1);
    OPENAT_lua_print("timestamp %02x %02x %02x", buf[0], buf[1], buf[2]);

    return 0;
}
int QMI8658_enableSensors(uint8 enableFlag)
{
    uint8 data;
    data = enableFlag;
    qmi_i2c_write(Qmi8658Register_Ctrl7, &data, 1);
    return 0;
}
int QMI8658_config_acc(enum qmi8658_AccRange range, enum qmi8658_AccOdr odr, enum qmi8658_LpfConfig lpfEnable, enum qmi8658_StConfig stEnable)
{
    uint8 ctrl_data;

    ctrl_data = range | odr;
    qmi_i2c_write(Qmi8658Register_Ctrl2, &ctrl_data, 1);

    // omit lpfConfig mode,
    qmi_i2c_read(Qmi8658Register_Ctrl5, &ctrl_data, 1);
    ctrl_data &= 0xF0;
    if (lpfEnable == Qmi8658Lpf_Enable)
    {
        ctrl_data |= A_LSP_MODE_2;
        ctrl_data |= 0x01;
    }
    else
    {
        ctrl_data &= ~0x01;
    }
    qmi_i2c_write(Qmi8658Register_Ctrl5, &ctrl_data, 1);
    return 0;
}

int QMI8658_on_command_cali(void)
{
    uint8 data;
    // OPENAT_lua_print("qmi8658 on demand cali started");
    data = 0xb0;
    qmi_i2c_write(Qmi8658Register_Reset, &data, 1);
    OPENAT_sleep(10);
    data = qmi8658_Ctrl9_Cmd_On_Demand_Cali;
    qmi_i2c_write(Qmi8658Register_Ctrl9, &data, 1);
    OPENAT_sleep(2200);
    data = qmi8658_Ctrl9_Cmd_NOP;
    qmi_i2c_write(Qmi8658Register_Ctrl9, &data, 1);
    OPENAT_sleep(100);
    // OPENAT_lua_print("qmi8658 on demand cali done");
    return 0;
}

int QMI8658_polling_begin(void *L)
{
    uint8 rtn=1, times=3,data;
    uint8 qmi8658_chip_id = 0;
    OPENAT_lua_print("QMI8658_polling_begin");

    while (qmi8658_chip_id != 0x05 && (times--) > 0 )
    {
        qmi_i2c_read(Qmi8658Register_WhoAmI, &qmi8658_chip_id, 1);
        // OPENAT_lua_print("WhoAmI %02x", qmi8658_chip_id);
    }
    if(times <= 0){
         OPENAT_lua_print("Can not find QMI8568 chip id");
         rtn = 0;
         goto polling_begin_end;
    }
    
    if(QMI8658_on_command_cali() != 0){
        OPENAT_lua_print("Can not calibrate QMI8568");
        rtn = 0;
        goto polling_begin_end;
    }

    data = 0x60 | QMI8658_INT2_ENABLE | QMI8658_INT1_ENABLE;
    qmi_i2c_write(Qmi8658Register_Ctrl1, &data, 1);

    QMI8658_enableSensors(0);
    if(QMI8658_config_acc(Qmi8658AccRange_2g, Qmi8658AccOdr_31_25Hz, Qmi8658Lpf_Disable, Qmi8658St_Disable)!=0){
        OPENAT_lua_print("Can not config QMI8568");
        rtn = 0;
        goto polling_begin_end;
    }
    QMI8658_enableSensors(1);

    OPENAT_sleep(50);


polling_begin_end:

    Kalman_init(&filterAccX, KALMAN_R, KALMAN_Q, KALMAN_A, KALMAN_B, KALMAN_C);
    Kalman_init(&filterAccY, KALMAN_R, KALMAN_Q, KALMAN_A, KALMAN_B, KALMAN_C);
    Kalman_init(&filterAccZ, KALMAN_R, KALMAN_Q, KALMAN_A, KALMAN_B, KALMAN_C);


    lua_pushinteger(L, rtn);
    return 1;
}
int QMI8658_read_acc()
{
    uint8  acc[6];

    float acceleration_mg[3] = {0, 0, 0};

    if (qmi_i2c_read(Qmi8658Register_Ax_L, &acc[0], 6) != 0)
    {
        return -1;
    };

    data_raw_acceleration[0] = (int16)((int16)acc[1] << 8 | acc[0]);
    data_raw_acceleration[1] = (int16)((int16)acc[3] << 8 | acc[2]);
    data_raw_acceleration[2] = (int16)((int16)acc[5] << 8 | acc[4]);

    // Kalman filter the data
    data_raw_acceleration[0] = Kalman_filter(&filterAccX, data_raw_acceleration[0], 0);
    data_raw_acceleration[1] = Kalman_filter(&filterAccY, data_raw_acceleration[1], 0);
    data_raw_acceleration[2] = Kalman_filter(&filterAccZ, data_raw_acceleration[2], 0);

    return 0;
}

int QMI8658_polling_acc(void *L)
{
    uint8 ready = 0, status;
    double acc_sum = 0;
    float acceleration_norm[3] = {0, 0, 0};

    qmi_i2c_read(Qmi8658Register_Status0, &status, 1);

    if ((status & 0x01) && (QMI8658_read_acc() == 0))
    {
        ready = 1;
        acc_sum = sqrt(pow(data_raw_acceleration[0], 2) + pow(data_raw_acceleration[1], 2) + pow(data_raw_acceleration[2], 2));
        acceleration_norm[0] = data_raw_acceleration[0] / acc_sum;
        acceleration_norm[1] = data_raw_acceleration[1] / acc_sum;
        acceleration_norm[2] = data_raw_acceleration[2] / acc_sum;
    }

    lua_pushinteger(L, ready);

    lua_pushinteger(L, (int)(acceleration_norm[0] * QMI8658_MAX_DIGITS));
    lua_pushinteger(L, (int)(acceleration_norm[1] * QMI8658_MAX_DIGITS));
    lua_pushinteger(L, (int)(acceleration_norm[2] * QMI8658_MAX_DIGITS));
    return 4;
}

int QMI8658_polling_z_tilt(void *L)
{
    uint8 ready = 0, status;
    // double acc_sum = 0;
    float acceleration_tilt[3] = {0, 0, 0};

    qmi_i2c_read(Qmi8658Register_Status0, &status, 1);

    if ((status & 0x01) && (QMI8658_read_acc() == 0))
    {
        ready = 1;

        acceleration_tilt[0] = atan(data_raw_acceleration[0]/ \
        sqrt(pow(data_raw_acceleration[1],2) + pow(data_raw_acceleration[2],2)));
        acceleration_tilt[1] = atan(data_raw_acceleration[1]/ \
        sqrt(pow(data_raw_acceleration[0],2) + pow(data_raw_acceleration[2],2)));
        acceleration_tilt[2] = atan(
        sqrt(pow(data_raw_acceleration[0],2) + pow(data_raw_acceleration[1],2))/ data_raw_acceleration[2]);
    }

    lua_pushinteger(L, ready);

    lua_pushnumber(L, acceleration_tilt[0]);
    lua_pushnumber(L, acceleration_tilt[1]);
    lua_pushnumber(L, acceleration_tilt[2]);

    return 4;
}