#include <Arduino.h>
#include <NimBLEDevice.h>
#include <BleGamepad.h>
#include <SPI.h>

// 硬件配置
#define CS_PIN 7
#define SCK_PIN 2
#define MOSI_PIN 3
#define MISO_PIN 6
#define READ_ANGLE_VALUE 0x8020

// 方向盘配置
const float FULL_ROTATION = 360.0f;
const float MAX_ROTATIONS = 3.0f; // 1080度 = 3圈
const int16_t GAMEPAD_MIN = -32767;
const int16_t GAMEPAD_MAX = 32767;

BleGamepad bleGamepad("ESP32-C3 1080° Wheel", "ESP32-C3", 100);
SPIClass *vspi = nullptr;

// 角度跟踪状态
struct {
    float lastRawAngle = 0;
    float accumulatedAngle = 0;
    bool firstRead = true;
} angleState;

void setup() {
    Serial.begin(115200);
    while(!Serial); // 等待串口连接

    // 初始化SPI
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
    
    vspi = new SPIClass(FSPI);
    vspi->begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
    vspi->beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE1));

    // 配置游戏手柄
    BleGamepadConfiguration config;
    config.setAutoReport(false);
    config.setControllerType(CONTROLLER_TYPE_GAMEPAD);
    config.setAxesMin(GAMEPAD_MIN);
    config.setAxesMax(GAMEPAD_MAX);
    config.setWhichAxes(true, false, false, false, false, false, false, false);
    bleGamepad.begin(&config);
    
    Serial.println("1080°方向盘初始化完成，等待BLE连接...");
}

uint16_t readSensor(uint16_t reg) {
    uint8_t cmd[2] = {(uint8_t)(reg >> 8), (uint8_t)(reg & 0xFF)};
    uint8_t response[4] = {0};
    
    digitalWrite(CS_PIN, LOW);
    vspi->transfer(cmd, 2);
    vspi->transfer(response, 4);
    digitalWrite(CS_PIN, HIGH);
    
    return (response[0] << 8) | response[1];
}

float getRawAngle() {
    uint16_t raw = readSensor(READ_ANGLE_VALUE);
    return (raw & 0x7FFF) * FULL_ROTATION / 32768.0f;
}

float updateAngleTracking(float currentAngle) {
    if (angleState.firstRead) {
        angleState.lastRawAngle = currentAngle;
        angleState.firstRead = false;
        return 0;
    }

    // 计算角度变化，处理360°边界
    float delta = currentAngle - angleState.lastRawAngle;
    if (delta > 180.0f) delta -= FULL_ROTATION;
    else if (delta < -180.0f) delta += FULL_ROTATION;

    // 更新累计角度
    angleState.accumulatedAngle += delta;
    angleState.lastRawAngle = currentAngle;

    // 限制在±540°范围内
    angleState.accumulatedAngle = constrain(angleState.accumulatedAngle, 
                                         -MAX_ROTATIONS * 180.0f, 
                                         MAX_ROTATIONS * 180.0f);

    return angleState.accumulatedAngle;
}

// 修改点：反转输出方向（添加负号）
int16_t mapToGamepadRange(float angle) {
    // 将-540°~+540°映射到-32767~+32767（添加负号反转方向）
    return (int16_t)(-angle * (GAMEPAD_MAX / (MAX_ROTATIONS * 180.0f)));
}

void loop() {
    if (bleGamepad.isConnected()) {
        float currentAngle = getRawAngle();
        float trackedAngle = updateAngleTracking(currentAngle);
        int16_t steeringValue = mapToGamepadRange(trackedAngle);
        
        bleGamepad.setX(steeringValue);
        bleGamepad.sendReport();
        
        Serial.printf("当前角度: %.2f° | 累计角度: %.2f° | 输出值: %d\n", 
                     currentAngle, trackedAngle, steeringValue);
    } else {
        Serial.println("等待BLE连接...");
    }
    
    delay(10); // 100Hz更新率
}
