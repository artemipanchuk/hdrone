#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <linux/i2c-dev.h>

#ifndef I2C_DEVICE_H
#define I2C_DEVICE_H

#define I2C_DEVICE_ADXL345_I2C_ADDR  0x53
#define I2C_DEVICE_SRF02_I2C_ADDR    0x70
#define I2C_DEVICE_BCM2708_PERI_BASE 0x20000000
#define I2C_DEVICE_GPIO_BASE         (I2C_DEVICE_BCM2708_PERI_BASE + 0x00200000)

namespace I2C {
    void init_secondary(void);

    class Device {
    public:
        Device(const char* bus, uint8_t address);
        ~Device(void);

        void write(uint8_t address, uint8_t byte);
        void write(uint8_t* buffer, uint8_t size);
        void read(uint8_t address, uint8_t* buffer, uint8_t size);

        const char* bus;
        int   fd;
    private:
    };
}

#endif
