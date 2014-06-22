#include "device.hh"

/*******************
 * Local variables *
 *******************/

static uint8_t cache[2];

/*******************
 * Local functions *
 *******************/

static void error(const char* status);

/************
 * Auxiliary
 */

static void error(const char* status) {
    printf("[ERROR] [I2C] %s.\n", status);
    printf("              Unable to handle error. Terminating.\n");

    exit(EXIT_FAILURE);    
}

/**********
 * Export *
 **********/

/******
 * I2C
 */

void I2C::init_secondary(void) {
    int fd = open("/dev/mem", O_RDWR|O_SYNC);
    if(fd < 0)
        error("Failed to open /dev/mem initializing secondary bus");

    void* map = mmap(
        0,
        4096,
        PROT_READ|PROT_WRITE,
        MAP_SHARED,
        fd,
        I2C_DEVICE_GPIO_BASE
    );

    if (map == MAP_FAILED)
        error("Failed to mmap initializing secondary bus");

    volatile unsigned int *address = (volatile unsigned int*) map;

    int reg0 = address[0];
    int m0   = 0b00000000000000000000111111111111;
    int s0   = 0b00000000000000000000100100000000;
    int b0   = reg0 & m0;
    if (b0 != s0)
        address[0] = (reg0 & ~m0) | s0;

    int reg2 = address[2];
    int m2   = 0b00111111000000000000000000000000;
    int s2   = 0b00100100000000000000000000000000;
    int b2   = reg2 & m2;
    if (b2 != s2)
        address[2] = (reg2 & ~m2) | s2;

    munmap(map, 4096);
    close(fd);
}

/**************
 * I2C::Device
 */

I2C::Device::Device(const char* bus, uint8_t address) {
    int fd;
    if((fd = open(bus, O_RDWR)) < 0)
        error("Failed to open I2C bus");

    if(ioctl(fd, I2C_SLAVE, address) < 0)
        error("Failed to select device");

    this->bus = bus;
    this->fd  = fd;
}

I2C::Device::~Device(void) {    
    close(this->fd);
}

void I2C::Device::write(uint8_t address, uint8_t byte) {
    cache[0] = address;
    cache[1] = byte;
    if(::write(this->fd, cache, 2) != 2)
        printf("Failed to write byte (I2C::Device::write). Ignoring.\n");     
}

void I2C::Device::write(uint8_t* buffer, uint8_t size) {
    if(::write(this->fd, buffer, size) != size)
        printf("Failed to write buffer (I2C::Device::write). Ignoring.\n");     
}

void I2C::Device::read(uint8_t address, uint8_t* buffer, uint8_t size) {
    cache[0] = address;
    if(::write(this->fd, cache, 1) != 1) {
        printf("Failed to write address (I2C::Device::read). Ignoring.\n");

        buffer = NULL;
        
        return;
    }

    if(::read(this->fd, buffer, size) != size) {
        printf("Failed to read buffer (I2C::Device::read). Ignoring.\n");

        buffer = NULL;
        
        return;
    }
}
