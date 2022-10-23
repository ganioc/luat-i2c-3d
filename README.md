## LSM6DSR 读取倾角
返回值为弧度值*1000000, tilt x, tilt y, tilt z,

## 使用方法
 
```lua
// 设置I2C, port=2
user.setup_I2C(2) 

// 配置轮询方式
user.LSM6DSR_polling_begin();

// 读取倾角的数值
local wakeup,nx,ny,nz = user.LSM6DSR_polling_check();
if wakeup == 1 then
    print("Tilt detected",nx,ny,nz)
end
```

## QMI8658如何读取倾角呢？




