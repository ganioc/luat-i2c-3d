local pack = require "pack"
local misc = require "misc"
local pcf8563 = {}

local SLAVE_ADDR = 0x51
local CTRL1_ADDR = 0
local CTRL2_ADDR = 1
local SEC_ADDR = 2
local MIN_ADDR = 3
local HOUR_ADDR = 4
local DAY_ADDR = 5
local WDAY_ADDR = 6
local MONTH_ADDR = 7
local YEAR_ADDR = 8
local TIMER_CTRL_ADDR = 0xE
local TIMER_ADDR = 0xF

local I2C_FREQ = 400000

local function rtcBcd2Bin(bcd)
    return bit.band(bcd, 0xf) + bit.band(bit.rshift(bcd, 4), 0x0f) * 10
end

local function rtcBin2Bcd(bin)
    local high = math.floor(bin / 10)
    local low = bin % 10
    return bit.bor(bit.lshift(high, 4), low)
end

local function pcf8563Read(handle, addr)
    local max_try = 3
    local i = 0
    while i < max_try do
        local data = i2c.read(handle, addr, 1)
        if data ~= nil then
            local _, byte = pack.unpack(data, "b")
            return byte
        end
        i = i + 1
    end
    log.error("pcf8563", "i2c read fail")
    return nil
end

function pcf8563.enableAlarm(i2cHandle, count)
    i2c.write(i2cHandle, TIMER_ADDR, count) -- count down
    i2c.write(i2cHandle, TIMER_CTRL_ADDR, 0x83) -- timer 1/60 HZ

    local data = pcf8563Read(i2cHandle, CTRL2_ADDR)
    data = bit.band(bit.bor(data, 0x11), 0xFF)
    i2c.write(i2cHandle, CTRL2_ADDR, data)
end

function pcf8563.setup(i2cHandle)
    if i2c.setup(i2cHandle, I2C_FREQ, SLAVE_ADDR) ~= I2C_FREQ then
        log.warn("RTC fail to init i2c")
    end
    --i2c.close(2)
    log.info("end of pcf8563 setup")

    local data = pcf8563Read(i2cHandle, CTRL1_ADDR)
    if bit.isset(data, 5) then
        log.info("Starting RTC")
        i2c.write(i2cHandle, CTRL1_ADDR, bit.clear(data, 5))
    end
    log.info("RTC Setup done")
end

function pcf8563.dumpReg(handle)
    local addrs = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8}
    for i = 1,3, 1 do
        local data = pcf8563Read(handle, addrs[i])
        print("data ->", data)
        if data then
            print("addr -> ", string.format("%02x", addrs[i]), ", value ->", string.format("%02x", data))
        end
    end
end

function pcf8563.getTime(handle)
    local tm = {year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0}

    local sec = rtcBcd2Bin(bit.band(pcf8563Read(handle, SEC_ADDR), 0x7F))
    local min = rtcBcd2Bin(bit.band(pcf8563Read(handle, MIN_ADDR), 0x7F))
    local hour = rtcBcd2Bin(bit.band(pcf8563Read(handle, HOUR_ADDR), 0x3F))
    local day = rtcBcd2Bin(bit.band(pcf8563Read(handle, DAY_ADDR), 0x3F))
    local month = rtcBcd2Bin(bit.band(pcf8563Read(handle, MONTH_ADDR), 0xF))
    local year = rtcBcd2Bin(bit.band(pcf8563Read(handle, YEAR_ADDR), 0xFF)) + 2000

    tm.year = year
    tm.month = month
    tm.day = day
    tm.hour = hour
    tm.min = min
    tm.sec = sec

    return tm
end

function pcf8563.clockSyncToRTC(handle, ts)
    local t
    if ts then
        t = os.date("*t", ts)
    else
        t = os.date("*t")
    end
    print(json.encode(t))
    local year = rtcBin2Bcd(t.year - 2000)
    local month = rtcBin2Bcd(t.month)
    local day = rtcBin2Bcd(t.day)
    local weekDay = rtcBin2Bcd(t.wday)
    local hour = rtcBin2Bcd(t.hour)
    local min = rtcBin2Bcd(t.min)
    local sec = rtcBin2Bcd(t.sec)

    i2c.write(handle, YEAR_ADDR, year)
    i2c.write(handle, MONTH_ADDR, month)
    i2c.write(handle, DAY_ADDR, day)
    i2c.write(handle, WDAY_ADDR, weekDay)
    i2c.write(handle, HOUR_ADDR, hour)
    i2c.write(handle, MIN_ADDR, min)
    i2c.write(handle, SEC_ADDR, sec)
end

function pcf8563.rtcSyncToClock(handle)
    local tm = pcf8563.getTime(handle)
    misc.setClock(
        tm,
        function(time, result)
            if result then
                log.info("pcf8563", "set time success", json.encode(time))
            else
                log.error("pcf8563", "fail to set clock")
            end
        end
    )
end

function pcf8563.close(handle)
    i2c.close(handle)
end

return pcf8563
