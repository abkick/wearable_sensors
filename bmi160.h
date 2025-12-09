#ifndef BMI160_H
#define BMI160_H

#include "mbed.h"

class BMI160 {
public:
    BMI160(PinName sda, PinName scl, int address = 0x68);

    bool begin();
    bool read(int16_t accel[3], int16_t gyro[3]);

private:
    I2C i2c;
    int i2c_addr;

    bool writeReg(uint8_t reg, uint8_t val);
    bool readReg(uint8_t reg, uint8_t &val);
    bool readBytes(uint8_t reg, uint8_t *buf, int len);
};

#endif
