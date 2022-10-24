#include "../include/i2c_QMI8658.h"
#include "../include/qmi8658_reg.h"
#include "../include/kalman.h"

static Kalman_t filterAccX, filterAccY, filterAccZ;

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
int QMI8658_read_temp(void *L){
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
int read_Qmi8568_dump_reg(void *L){
    // read out registers
    uint8 buf[8];
    UINT8 reg = Qmi8658Register_Ctrl1;
    qmi_i2c_read(reg, &buf[0],1);
    reg = Qmi8658Register_Ctrl2;
    qmi_i2c_read(reg, &buf[1],1);
    reg = Qmi8658Register_Ctrl3;
    qmi_i2c_read(reg, &buf[2],1);
    reg = Qmi8658Register_Ctrl4;
    qmi_i2c_read(reg, &buf[3],1);
    reg = Qmi8658Register_Ctrl5;
    qmi_i2c_read(reg, &buf[4],1);
    reg = Qmi8658Register_Ctrl6;
    qmi_i2c_read(reg, &buf[5],1);
    reg = Qmi8658Register_Ctrl7;
    qmi_i2c_read(reg, &buf[6],1);
    reg = Qmi8658Register_Ctrl8;
    qmi_i2c_read(reg, &buf[7],1);

    OPENAT_lua_print("dump: %02x %02x %02x %02x %02x %02x %02x %02x", 
        buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
    return 0;  
}
int QMI8658_read_timestamp(void*L){
    uint8	buf[3];
	uint32 timestamp;
    UINT8 reg;
    reg = Qmi8658Register_Timestamp_L;
    qmi_i2c_read( reg, &buf[0], 1);
    reg = Qmi8658Register_Timestamp_M;
    qmi_i2c_read( reg, &buf[1], 1);
    reg = Qmi8658Register_Timestamp_H;
    qmi_i2c_read( reg, &buf[1], 1);
    OPENAT_lua_print("timestamp %02x %02x %02x",  buf[0], buf[1], buf[2]);

    return 0;
}
int QMI8658_enableSensors(uint8 enableFlag){
    uint8 data;
    data = enableFlag;
    qmi_i2c_write(Qmi8658Register_Ctrl7,&data,1);
    return 0;
}
int QMI8658_config_acc(enum qmi8658_AccRange range, enum qmi8658_AccOdr odr,enum qmi8658_LpfConfig lpfEnable,enum qmi8658_StConfig stEnable){
    uint8 ctrl_data;

    ctrl_data = range | odr;
    qmi_i2c_write(Qmi8658Register_Ctrl2, &ctrl_data,1);

    // omit lpfConfig mode,
    qmi_i2c_read(Qmi8658Register_Ctrl5, &ctrl_data, 1);
    ctrl_data&=0xF0;
    if(lpfEnable == Qmi8658Lpf_Enable){
        ctrl_data |= A_LSP_MODE_2;
        ctrl_data |= 0x01;
    }else{
        ctrl_data &= ~0x01;
    }
    qmi_i2c_write(Qmi8658Register_Ctrl5, &ctrl_data, 1);
    return 0;
}
int QMI8658_config_reg(){
    QMI8658_enableSensors(0);
    QMI8658_config_acc(Qmi8658AccRange_2g, Qmi8658AccOdr_31_25Hz, Qmi8658Lpf_Disable,Qmi8658St_Disable );
    return 0;
}
int QMI8658_on_command_cali(void){
    uint8 data;
    OPENAT_lua_print("qmi8658 on demand cali started");
    data = 0xb0;
    qmi_i2c_write(Qmi8658Register_Reset,&data,1);
    OPENAT_sleep(10);
    data = qmi8658_Ctrl9_Cmd_On_Demand_Cali;
    qmi_i2c_write(Qmi8658Register_Ctrl9, &data, 1);
    OPENAT_sleep(2200);
    data = qmi8658_Ctrl9_Cmd_NOP;
    qmi_i2c_write(Qmi8658Register_Ctrl9, &data, 1);
    OPENAT_sleep(100);
    OPENAT_lua_print("qmi8658 on demand cali done");
    return 0;
}
int QMI8658_get_id(void){
    uint8 iCount = 0,data;
    uint8 qmi8658_chip_id = 0;
    uint8 qmi8658_revision_id;
    uint8 firmware_id[3];
    uint8 uuid[6];

    while(qmi8658_chip_id != 0x05){
        qmi_i2c_read(Qmi8658Register_WhoAmI, &qmi8658_chip_id,1);
        OPENAT_lua_print("WhoAmI %02x", qmi8658_chip_id);
    }

    QMI8658_on_command_cali();
    data = 0x60|QMI8658_INT2_ENABLE |QMI8658_INT1_ENABLE;
    qmi_i2c_write(Qmi8658Register_Ctrl1, &data, 1);
    qmi_i2c_read(Qmi8658Register_Revision, &qmi8658_revision_id, 1);
    qmi_i2c_read(Qmi8658Register_firmware_id, &firmware_id[0], 1);
    qmi_i2c_read(Qmi8658Register_firmware_id + 1, &firmware_id[1], 1);
    qmi_i2c_read(Qmi8658Register_firmware_id + 2, &firmware_id[2], 1);
    qmi_i2c_read(Qmi8658Register_uuid, &uuid[0],1);
    qmi_i2c_read(Qmi8658Register_uuid+1, &uuid[1],1);
    qmi_i2c_read(Qmi8658Register_uuid+2, &uuid[2],1);
    qmi_i2c_read(Qmi8658Register_uuid+3, &uuid[3],1);
    qmi_i2c_read(Qmi8658Register_uuid+4, &uuid[4],1);
    qmi_i2c_read(Qmi8658Register_uuid+5, &uuid[5],1);
    data = 0;
    qmi_i2c_write(Qmi8658Register_Ctrl7,&data, 1);
    // data = 0xc0;
    // qmi_i2c_write(Qmi8658Register_Ctrl8，&data, 1);
    OPENAT_lua_print("revision %02x", qmi8658_revision_id);
    OPENAT_lua_print("firmware id: %02x %02x  %02x", firmware_id[0], firmware_id[1], firmware_id[2]);

    return 0;
}
int QMI8658_polling_begin(void *L){
    OPENAT_lua_print("QMI8658_polling_begin");

    QMI8658_get_id();
    QMI8658_config_reg();
    QMI8658_enableSensors(1);

    OPENAT_sleep(50);

    read_Qmi8568_dump_reg(L);

    Kalman_init(&filterAccX, KALMAN_R, KALMAN_Q, KALMAN_A,KALMAN_B, KALMAN_C);
    Kalman_init(&filterAccY, KALMAN_R, KALMAN_Q, KALMAN_A,KALMAN_B, KALMAN_C);
    Kalman_init(&filterAccZ, KALMAN_R, KALMAN_Q, KALMAN_A, KALMAN_B, KALMAN_C);
    return 1;
}
int QMI8658_polling_acc(void *L){
    uint8 status, acc_x[2],acc_y[2],acc_z[2];
    uint8 ready = 0;
    int16 data_raw_acceleration[3];
    float acceleration_mg[3]={0,0,0};

    qmi_i2c_read(Qmi8658Register_Status0, &status, 1);

    if(status&0x01){
        ready = 1;
        qmi_i2c_read(Qmi8658Register_Ax_L,&acc_x[0],1);
        qmi_i2c_read(Qmi8658Register_Ax_H,&acc_x[1],1);
        qmi_i2c_read(Qmi8658Register_Ay_L,&acc_y[0],1);
        qmi_i2c_read(Qmi8658Register_Ay_H,&acc_y[1],1);
        qmi_i2c_read(Qmi8658Register_Az_L,&acc_z[0],1);
        qmi_i2c_read(Qmi8658Register_Az_H,&acc_z[1],1);

        data_raw_acceleration[0] = (int16)((int16)acc_x[1]<<8|acc_x[0]);
        data_raw_acceleration[1] =  (int16)((int16)acc_y[1]<<8|acc_y[0]);
        data_raw_acceleration[2] =  (int16)((int16)acc_z[1]<<8|acc_z[0]);

        // Kalman filter the data
        data_raw_acceleration[0] = Kalman_filter(&filterAccX,data_raw_acceleration[0], 0);
        data_raw_acceleration[1] = Kalman_filter(&filterAccY,data_raw_acceleration[1], 0);
        data_raw_acceleration[2] = Kalman_filter(&filterAccZ,data_raw_acceleration[2], 0); 

    }
    lua_pushinteger(L, ready);
    lua_pushinteger(L, (int) (data_raw_acceleration[0]));
    lua_pushinteger(L, (int) (data_raw_acceleration[1]));
    lua_pushinteger(L, (int) (data_raw_acceleration[2]));
    return 4;
}




