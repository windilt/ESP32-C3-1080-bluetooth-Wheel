# ESP32-C3-1080-bluetooth-Wheel
DIY ESP32-C3 1080° bluetooth Wheel with arduino

所有的电路都固定在方向盘盘面上，使用锂电池供电

油门和刹车需要另外的模块，建议使用freejoy项目

**请把方向盘回中再接通电源，上电时的角度会自动设置为原点

**linux下会遇到方向盘有很大死区的问题，对于ETS2这样的游戏很不友好。原因是evdev API会自动设置死区和fuzz（移动死区）。参考Arch wiki, 可以运行以下命令设置死区为0

evdev-joystick --evdev /dev/input/eventXX --deadzone 0 --fuzz 0

请注意蓝牙设备不会自动在/dev/input/by-id/下建立链接，所以需要运行sudo evtest找出对应设备

# hardware:

ESP32-C3模块

tle5012

# 接线：

CS_PIN 7

SCK_PIN 2

MOSI_PIN 3

MISO_PIN 6

# Todo：

1,目前使用橡皮筋回中，感觉还行，后续可能会增加电动回中功能，也可能增加力回馈功能

2,所有的电路都固定在方向盘盘面上，后续会增加按键功能
