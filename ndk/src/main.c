#include "core_api.h"
#include "../include/i2c_api.h"
#include "../include/i2c_LSM6DSR.h"

// extern int read_LSM6DSR_WHOAMI(void * L);


int helloworld(void *L)
{
	OPENAT_lua_print("ndk Go go :hello world!");
	return 0; //代表有0个返回值
}

int setup_I2C(void *L)
{
	OPENAT_lua_print("setup RTC()");
	BOOL err;
	UINT8 data;
	UINT16 reg;

	int num1 = luaL_checkinteger(L, 1);

	T_AMOPENAT_I2C_PARAM i2cCfg;
	OPENAT_lua_print("I2C PORT: %d", num1);
	memset(&i2cCfg, 0, sizeof(T_AMOPENAT_I2C_PARAM));
	i2cCfg.freq = 400000;
	i2cCfg.regAddrBytes = 0;
	i2cCfg.noAck = FALSE;
	i2cCfg.noStop = FALSE;
	i2cCfg.i2cMessage = 0;

	if (OPENAT_open_i2c(DEMO_I2C_PORT, &i2cCfg) == FALSE)
	{
		OPENAT_lua_print("Open [i2c] err");
	}

	OPENAT_lua_print("Open [i2c] done");

	// reg = CTRL1_ADDR;
	// iot_i2c_read(DEMO_I2C_PORT, I2CSLAVEADDR, &reg, &data, 1);
	// OPENAT_lua_print("data: %d", data);

	OPENAT_sleep(1000);

	lua_pushinteger(L, 0); // push returned value
	return 1;
}
int get_table(void *L){
	lua_pushinteger(L, 112);
	lua_pushinteger(L, 201);
	lua_pushinteger(L, 123);

	return 3;
}

luaL_Reg user_lib[] = {
	{"helloworld", helloworld},
	{"setup_I2C", setup_I2C},
	{"read_LSM6DSR_WHOAMI", read_LSM6DSR_WHOAMI},
	// {"LSM6DSR_begin", LSM6DSR_begin},
	// {"LSM6DSR_end", LSM6DSR_end},
	// {"LSM6DSR_enable_X", Enable_X},
	// {"LSM6DSR_enable_G", Enable_G},
	{"get_table", get_table},
	// {"Get_X_AxesRaw", Get_X_AxesRaw},
	// {"Get_G_AxesRaw", Get_G_AxesRaw},
	// {"LSM6DSR_activity_begin", LSM6DSR_activity_begin},
	// {"LSM6DSR_activity_check", LSM6DSR_activity_check},
	// {"LSM6DSR_tilt_begin", LSM6DSR_tilt_begin},
	// {"LSM6DSR_tilt_check", LSM6DSR_tilt_check},
	{"LSM6DSR_polling_begin", LSM6DSR_polling_begin},
	{"LSM6DSR_polling_check", LSM6DSR_polling_check},
	{"LSM6DSR_polling_acc", LSM6DSR_polling_acc},
	{NULL, NULL}};
	
/*入口函数*/
int user_main(void *L)
{
	/*C函数注册*/
	luaI_openlib(L, "user", user_lib, 0);
	return 1;
}
