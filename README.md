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

QMI8658_polling_acc(),
获取到gx,gy,gz归一化的值*1000000, 6个0, 

int QMI8658_polling_z_tilt(void *L),
获取到相对于水平面的Z轴的倾斜角度(弧度)*1000000, 


