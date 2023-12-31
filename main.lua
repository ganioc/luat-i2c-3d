-- 必须在这个位置定义PROJECT和VERSION变量
-- PROJECT：ascii string类型，可以随便定义，只要不使用,就行
-- VERSION：ascii string类型，如果使用Luat物联云平台固件升级的功能，必须按照"X.X.X"定义，X表示1位数字；否则可随便定义
PROJECT = "DL_TEST"
VERSION = "1.0.0"

-- 加载日志功能模块，并且设置日志输出等级
-- 如果关闭调用log模块接口输出的日志，等级设置为log.LOG_SILENT即可
require "log"
LOG_LEVEL = log.LOGLEVEL_TRACE
require "sys"
require "ril"
require "pins"

require "uart"


-- 打印死机信息
ril.request("AT*EXINFO?")
rtos.sleep(3000)

local rtc_i2c_handle = 2
local pcf8563 = require("pcf8563")


--[[
dl模块接口定义
函数：handle=dl.open(libpath,usermap)
功能：加载c编译的lib文件
  参数：
      --path:lib的路径,string类型
      --usermap:lua函数接口注册表
  返回值：
      --handle:成功返回句柄，失败返回nil

  函数： ret = dl.close(handle)
  功能：卸载lib
      参数：
          handle:dl.open的返回值
      返回值：nil
]]

-- 常规测试
sys.taskInit(function()    
    
    sys.wait(1000)
    print("---------------------")
    -- enable RTC
    -- pmd.ldoset(2, pmd.LDO_VMMC)
    -- setOutputFnc = pins.setup(pio.P0_27, 1)


    sys.wait(1000)
    pcf8563.setup(2)
    -- pcf8563.enableAlarm(rtc_i2c_handle, 3)


    --uart.setup(0x81, 115200, 8, uart.PAR_NONE, uart.STOP_1)


    local handle = dl.open("/lua/user.lib", "user_main")
    if handle then
        -- 添加测试demo
        -- user.setup_I2C(2)


        -- local rtn = user.LSM6DSR_begin();
        -- print("begin - rtn", rtn)
        -- rtn = user.LSM6DSR_enable_X();
        -- print("enable_X - rtn", rtn)
        -- rtn = user.LSM6DSR_enable_G();
        -- print("enable_G - rtn", rtn)

        -- user.LSM6DSR_polling_begin();
        -- 31Hz update rate
        local rtn = user.QMI8658_polling_begin();
        if rtn == 1 then
            print("QmI8568 init OK")
        else 
            print("QMI8568 init FAIL")
        end

        while true do
            -- user.helloworld()
            sys.wait(100)
            -- user.QMI8658_read_temp()
            -- user.QMI8658_read_timestamp()
            -- pcf8563.dumpReg(rtc_i2c_handle)
            -- local time = pcf8563.getTime(rtc_i2c_handle)
            -- for k,v in pairs(time) do
            --     print(k .. '=' .. v)
            -- end
            -- user.read_LSM6DSR_WHOAMI()
            -- local num1,num2,num3 = user.Get_X_AxesRaw();
            -- print("X axes: ", num1, num2,num3)
            -- local gnum1,gnum2,gnum3 = user.Get_G_AxesRaw();
            -- print("G axes: ", gnum1, gnum2,gnum3)
            -- sys.wait(100)
            -- local wakeup,tiltx, tilty, tiltz = user.LSM6DSR_polling_acc();
            -- if wakeup == 1 then
            --     print("ACC:",tiltx,tilty,tiltz)
            -- end


            -- local ready,accx,accy,accz = user.QMI8658_polling_acc()
            -- if ready == 1 then
            --     print("ACC:",accx,accy,accz)
            -- end

            -- sys.wait(40)

            local ready2,tiltx,tilty,tiltz = user.QMI8658_polling_z_tilt()
            if ready2 == 1 then
                print("TILT:",tiltx,tilty,tiltz)
            end

        end
    end
end)

-- 启动系统框架
sys.init(0, 0)
sys.run()
