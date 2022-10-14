#include "core_api.h"

#define i2c_print OPENAT_lua_print
#define DEMO_I2C_PORT OPENAT_I2C_2
#define I2CSLAVEADDR 0x51

UINT32 iot_i2c_write(E_AMOPENAT_I2C_PORT port, UINT8 salveAddr,  UINT16 *pRegAddr, CONST UINT8 *buf, UINT32 bufLen) {
  UINT8 psalveAddr = salveAddr << 1;
  return OPENAT_write_i2c(port, psalveAddr, pRegAddr, buf, bufLen);
}

UINT32 iot_i2c_read(E_AMOPENAT_I2C_PORT port, UINT8 slaveAddr,  UINT16 *pRegAddr, UINT8 *buf, UINT32 bufLen) {
  UINT8 psalveAddr = slaveAddr << 1;
  return OPENAT_read_i2c(port, psalveAddr, pRegAddr, buf, bufLen);
}

// int demo_i2c_test(void *L) {
//   i2c_print("[i2c] demo_i2c_open");
//   BOOL err;

//   int num1 = luaL_checkinteger(L, 1);

//   T_AMOPENAT_I2C_PARAM i2cCfg;
//   i2c_print("MACOS before memeset num is %d", num1);
//   memset(&i2cCfg, 0, sizeof(T_AMOPENAT_I2C_PARAM));
//   i2cCfg.freq = 400000;
// 	i2cCfg.regAddrBytes = 0;
// 	i2cCfg.noAck = FALSE;
// 	i2cCfg.noStop = FALSE;
// 	i2cCfg.i2cMessage = 0;

//   // uint8 test = 0x12;
//   // i2c_print("data is %hhx", test);

//   i2c_print("before i2c open");
//   if (OPENAT_open_i2c(DEMO_I2C_PORT, &i2cCfg) == FALSE) {
//     i2c_print("[i2c] err");
//   }

//   uint8 status[3];

//   uint16 addr = 0;
//   int i;
//   for (i = 0; i < 1; i++) {
//     addr = addr + i;
//     if (iot_i2c_read(DEMO_I2C_PORT, I2CSLAVEADDR, &addr, &status[i], 1) != 1) {
//       i2c_print("Fail i2c read");
//     }
//   }
//   i2c_print("status_1: %hhx status_2: %hhx sec: %hhx", status[0], status[1], status[2]);
//   OPENAT_sleep(1000);
//   // for (i = 0; i < 3; i++) {
//   //   addr = 0x2;
//   //   volatile uint8 data;
//   //   if (iot_i2c_read(DEMO_I2C_PORT, I2CSLAVEADDR, &addr, &data, 1) != 1) {
//   //     i2c_print("Fail i2c read");
//   //   }
//   //   i2c_print("data is %hhd", data);
//   //   OPENAT_sleep(1000);
//   // }

//   lua_pushnumber(L, 2);
//   return 1;
// }
