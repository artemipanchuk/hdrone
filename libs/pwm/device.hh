#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#define PWM_DEVICE_DELAY_VIA_PWM                 0
#define PWM_DEVICE_DELAY_VIA_PCM                 1

#define PWM_DEVICE_PULSE_WIDTH_INCREMENT_US      10
#define PWM_DEVICE_SUBCYCLE_TIME_US              20000

#define PWM_DEVICE_DMA_CHANNELS_NUMBER           15

#define PWM_DEVICE_DMA_CHANNEL_PAGE_SHIFT        12
#define PWM_DEVICE_DMA_CHANNEL_PAGE_SIZE         4096

#define PWM_DEVICE_DMA_ADDRESS                   0x20007000
#define PWM_DEVICE_DMA_CHANNEL_INCREASE_SHIFT    0x100
#define PWM_DEVICE_DMA_CHANNEL_SIZE              0x24

#define PWM_DEVICE_PWM_ADDRESS                   0x2020C000
#define PWM_DEVICE_PWM_SIZE                      0x28

#define PWM_DEVICE_CLK_ADDRESS                   0x20101000
#define PWM_DEVICE_CLK_SIZE                      0xA8

#define PWM_DEVICE_GPIO_ADDRESS                  0x20200000
#define PWM_DEVICE_GPIO_SIZE                     0x100

#define PWM_DEVICE_PCM_ADDRESS                   0x20203000
#define PWM_DEVICE_PCM_SIZE                      0x24

#define PWM_DEVICE_DMA_NO_WIDE_BURSTS           (1<<26)
#define PWM_DEVICE_DMA_WAIT_RESP                (1<<3)
#define PWM_DEVICE_DMA_D_DREQ                   (1<<6)
#define PWM_DEVICE_DMA_PER_MAP(x)               ((x)<<16)
#define PWM_DEVICE_DMA_END                      (1<<1)
#define PWM_DEVICE_DMA_RESET                    (1<<31)
#define PWM_DEVICE_DMA_INT                      (1<<2)

#define PWM_DEVICE_DMA_CS                       (0x00/4)
#define PWM_DEVICE_DMA_CONBLK_AD                (0x04/4)
#define PWM_DEVICE_DMA_DEBUG                    (0x20/4)

#define PWM_DEVICE_GPIO_FSEL0                   (0x00/4)
#define PWM_DEVICE_GPIO_SET0                    (0x1c/4)
#define PWM_DEVICE_GPIO_CLR0                    (0x28/4)
#define PWM_DEVICE_GPIO_LEV0                    (0x34/4)
#define PWM_DEVICE_GPIO_PULLEN                  (0x94/4)
#define PWM_DEVICE_GPIO_PULLCLK                 (0x98/4)

#define PWM_DEVICE_GPIO_MODE_IN                 0
#define PWM_DEVICE_GPIO_MODE_OUT                1

#define PWM_DEVICE_PWM_CTL                      (0x00/4)
#define PWM_DEVICE_PWM_DMAC                     (0x08/4)
#define PWM_DEVICE_PWM_RNG1                     (0x10/4)
#define PWM_DEVICE_PWM_FIFO                     (0x18/4)

#define PWM_DEVICE_PWMCLK_CNTL                  40
#define PWM_DEVICE_PWMCLK_DIV                   41

#define PWM_DEVICE_PWMCTL_MODE1                 (1<<1)
#define PWM_DEVICE_PWMCTL_PWEN1                 (1<<0)
#define PWM_DEVICE_PWMCTL_CLRF                  (1<<6)
#define PWM_DEVICE_PWMCTL_USEF1                 (1<<5)

#define PWM_DEVICE_PWMDMAC_ENAB                 (1<<31)
#define PWM_DEVICE_PWMDMAC_THRSHLD              ((15<<8) | (15<<0))

#define PWM_DEVICE_PCM_CS_A                     (0x00/4)
#define PWM_DEVICE_PCM_FIFO_A                   (0x04/4)
#define PWM_DEVICE_PCM_MODE_A                   (0x08/4)
#define PWM_DEVICE_PCM_RXC_A                    (0x0c/4)
#define PWM_DEVICE_PCM_TXC_A                    (0x10/4)
#define PWM_DEVICE_PCM_DREQ_A                   (0x14/4)
#define PWM_DEVICE_PCM_INTEN_A                  (0x18/4)
#define PWM_DEVICE_PCM_INT_STC_A                (0x1c/4)
#define PWM_DEVICE_PCM_GRAY                     (0x20/4)

#define PWM_DEVICE_PCMCLK_CNTL                  38
#define PWM_DEVICE_PCMCLK_DIV                   39

#define PWM_DEVICE_PHYS_GPCLR0                  (0x7e200000 + 0x28)
#define PWM_DEVICE_PHYS_GPSET0                  (0x7e200000 + 0x1c)

namespace PWM {
    void init(void);
    void uninit(void);

    class Device {
    public:
         Device(int channel_id, int pin_id);
        ~Device();

        void set(int value);
        void clear(void);
        void error(const char* status);

    private:
        int channel_id;
        int pin_id;
        int value;
    };
}
