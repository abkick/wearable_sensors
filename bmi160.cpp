#include "bmi160.h"

#define BMI160_CHIP_ID         0x00
#define BMI160_CMD             0x7E
#define BMI160_SOFTRESET       0xB6
#define BMI160_CMD_ACC_NORMAL  0x11
#define BMI160_CMD_GYR_NORMAL  0x15

#define BMI160_ACC_CONF        0x40
#define BMI160_ACC_RANGE       0x41
#define BMI160_GYR_CONF        0x42
#define BMI160_GYR_RANGE       0x43

#define BMI160_DATA_START      0x12

BMI160::BMI160(PinName sda, PinName scl, int address)
    : i2c(sda, scl)
{
    i2c.frequency(400000); // fast mode
    i2c_addr = address << 1; // mbed uses 8-bit addr
}

bool BMI160::writeReg(uint8_t reg, uint8_t val)
{
    char data[2] = { (char)reg, (char)val };
    return (i2c.write(i2c_addr, data, 2) == 0);
}

bool BMI160::readReg(uint8_t reg, uint8_t &val)
{
    char r = reg;
    if (i2c.write(i2c_addr, &r, 1) != 0)
        return false;

    char buf;
    if (i2c.read(i2c_addr, &buf, 1) != 0)
        return false;

    val = buf;
    return true;
}

bool BMI160::readBytes(uint8_t reg, uint8_t *buf, int len)
{
    char r = reg;
    if (i2c.write(i2c_addr, &r, 1) != 0)
        return false;

    if (i2c.read(i2c_addr, (char*)buf, len) != 0)
        return false;

    return true;
}

bool BMI160::begin()
{
    uint8_t id;

    // Check CHIP ID
    if (!readReg(BMI160_CHIP_ID, id))
        return false;

    if (id != 0xD1)
        return false;

    // Soft reset
    writeReg(BMI160_CMD, BMI160_SOFTRESET);
    thread_sleep_for(100);

    // Enable accelerometer + gyro
    writeReg(BMI160_CMD, BMI160_CMD_ACC_NORMAL);
    thread_sleep_for(50);
    writeReg(BMI160_CMD, BMI160_CMD_GYR_NORMAL);
    thread_sleep_for(50);

    // Configure accelerometer
    writeReg(BMI160_ACC_CONF, 0x28);   // 100Hz ODR
    writeReg(BMI160_ACC_RANGE, 0x03);  // ±2g

    // Configure gyro
    writeReg(BMI160_GYR_CONF, 0x28);   // 100Hz ODR
    writeReg(BMI160_GYR_RANGE, 0x00);  // ±2000 dps

    return true;
}

bool BMI160::read(int16_t accel[3], int16_t gyro[3])
{
    uint8_t buf[12];
    if (!readBytes(BMI160_DATA_START, buf, 12))
        return false;

    // Accel
    accel[0] = (int16_t)((buf[1] << 8) | buf[0]);
    accel[1] = (int16_t)((buf[3] << 8) | buf[2]);
    accel[2] = (int16_t)((buf[5] << 8) | buf[4]);

    // Gyro
    gyro[0] = (int16_t)((buf[7] << 8) | buf[6]);
    gyro[1] = (int16_t)((buf[9] << 8) | buf[8]);
    gyro[2] = (int16_t)((buf[11] << 8) | buf[10]);

    return true;
}
