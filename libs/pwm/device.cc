#include "afro_esc.hh"

/*****************
 * Local structs *
 *****************/

typedef struct {
    uint32_t ti;
    uint32_t sa;
    uint32_t da;
    
    uint32_t size;
    uint32_t stride;
    uint32_t next;
    
    uint32_t reserved[2];
} dma_s;

typedef struct {
    uint8_t* va;
    uint32_t pa;
} pm_s;

struct channel_s {
    uint8_t*  vb;
    uint32_t* sample;
    
    dma_s*    dma;
    pm_s*     pm;
    
    volatile uint32_t* dma_a;
    
    uint32_t subcycle_time_us;
    uint32_t num_samples;
    uint32_t num_dmas;
    uint32_t num_pms;
    uint32_t num_max;
};

/*******************
 * Local variables *
 *******************/

static struct channel_s channels[PWM_DEVICE_DMA_CHANNELS_NUMBER];

static volatile uint32_t*  pwm_region;
static volatile uint32_t*  pcm_region;
static volatile uint32_t*  clk_region;
static volatile uint32_t* gpio_region;

static int channels_setup = 0;
static int pins_setup     = 0;

/*******************
 * Local functions *
 *******************/

static void udelay(int us);
static void error(const char* status);
static void* map_peripheral(uint32_t start, uint32_t size);
static void init_pin(int pin_id);
static void uninit_pin(int pin_id);
static void set_pin_mode(int pin_id, uint32_t mode);
static void set_pin_level(int pin_id, int level);
static void init_channel(int channel_id);
static void init_channel_vb(int channel_id);
static void init_channel_pm(int channel_id);
static void init_channel_dma(int channel_id);
static void uninit_channel(int channel_id);
static void set_channel(int channel_id, int pin_id, int width);
static uint8_t* get_channel_dma(int channel_id);
static uint32_t get_channel_v_to_p(int channel_id, void *v);
static void init(void);
static void uninit(void);

/************
 * Auxiliary
 */

static void udelay(int us) {
    fflush(stdout);
    
    struct timespec ts = {0, us*1000};
    nanosleep(&ts, NULL);
}

static void error(const char* status) {
    uninit();

    printf(status);
    printf(".\n");
    printf("Unable to handle error (PWM). Terminating.\n");

    exit(EXIT_FAILURE);
}

static void* map_peripheral(uint32_t start, uint32_t size) {
    void* va;
    int   fd;

    fd = open("/dev/mem", O_RDWR);
    if(fd < 0) {
        error("Failed to read /dev/mem mapping peripheral");

        return NULL;
    }

    va = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, start);
    if(va == MAP_FAILED) {
        error("Failed to mmap mmaping peripheral");

        return NULL;
    }

    close(fd);

    return va;
}

/***************
 * GPIO pins
 */

static void init_pin(int pin_id) {
    if((pins_setup & 1 << pin_id) != 0)
        error("Failed to init initialized pin");

    set_pin_mode (pin_id, PWM_DEVICE_GPIO_MODE_OUT);
    set_pin_level(pin_id, 0);
    
    pins_setup |= 1 << pin_id;
}

static void uninit_pin(int pin_id) {
    if((pins_setup & 1 << pin_id) == 0)
        error("Failed to uninit not initialized pin");

    set_pin_mode (pin_id, PWM_DEVICE_GPIO_MODE_OUT);
    set_pin_level(pin_id, 0);
    
    pins_setup -= 1 << pin_id;
}

static void set_pin_mode(int pin_id, uint32_t mode) {
    uint32_t fsel = gpio_region[PWM_DEVICE_GPIO_FSEL0 + pin_id/10];

    fsel &= ~(7 << ((pin_id % 10) * 3));
    fsel |= mode << ((pin_id % 10) * 3);

    gpio_region[PWM_DEVICE_GPIO_FSEL0 + pin_id/10] = fsel;
}

static void set_pin_level(int pin_id, int level) {
    if(level)
        gpio_region[PWM_DEVICE_GPIO_SET0] = 1 << pin_id;
    else
        gpio_region[PWM_DEVICE_GPIO_CLR0] = 1 << pin_id;
}

/*******************
 * DMA channels
 */

static void init_channel(int channel_id) {    
    if((channels_setup & 1 << channel_id) != 0)
        error("Failed to initialize initialized channel");

    if(channels[channel_id].vb)
        error("Invalid vb initiating channel");

    channels[channel_id].subcycle_time_us = PWM_DEVICE_SUBCYCLE_TIME_US;
    channels[channel_id].num_samples      = PWM_DEVICE_SUBCYCLE_TIME_US/PWM_DEVICE_PULSE_WIDTH_INCREMENT_US;
    channels[channel_id].num_max          = channels[channel_id].num_samples - 1;
    channels[channel_id].num_dmas         = channels[channel_id].num_samples * 2;
    channels[channel_id].num_pms          = (
        (channels[channel_id].num_dmas*32 + channels[channel_id].num_samples*4 + PWM_DEVICE_DMA_CHANNEL_PAGE_SIZE - 1)
        >> PWM_DEVICE_DMA_CHANNEL_PAGE_SHIFT
    );

    init_channel_vb (channel_id);
    init_channel_pm (channel_id);
    init_channel_dma(channel_id);

    channels_setup |= 1 << channel_id;
}


static void init_channel_vb(int channel_id) {
    channels[channel_id].vb = (uint8_t*) mmap(
        NULL,
        channels[channel_id].num_pms * PWM_DEVICE_DMA_CHANNEL_PAGE_SIZE,
        PROT_READ|PROT_WRITE,
        MAP_SHARED|MAP_ANONYMOUS|MAP_NORESERVE|MAP_LOCKED,
        -1,
         0
    );

    if(channels[channel_id].vb == MAP_FAILED)
        error("Failed to mmap initiating channel vb");
    
    if((unsigned long) channels[channel_id].vb & (PWM_DEVICE_DMA_CHANNEL_PAGE_SIZE - 1))
        error("Invalid vb after mmap initiating channel vb");
}


static void init_channel_pm(int channel_id) {
    int  fd, memfd;
    char pagemap_fn[64];
    size_t i;

    channels[channel_id].pm = (pm_s*) malloc(
        channels[channel_id].num_pms * sizeof(*channels[channel_id].pm)
    );

    if(channels[channel_id].pm == 0)
        error("Invalid pm making channel pagemap");

    memfd = open("/dev/mem", O_RDWR);
    if(memfd < 0)
        error("Invalid memfd making channel pagemap");
    
    sprintf(pagemap_fn, "/proc/%d/pagemap", getpid());

    fd = open(pagemap_fn, O_RDONLY);
    if(fd < 0)
        error("Invalid fd making channel pagemap");
    
    if(lseek(fd, (uint32_t) channels[channel_id].vb >> 9, SEEK_SET) != 
                 (uint32_t) channels[channel_id].vb >> 9)
        error("Invalid vb making channel pagemap");
    
    for(i = 0; i < channels[channel_id].num_pms; ++i) {
        uint64_t pfn;

        channels[channel_id].pm[i].va = channels[channel_id].vb +
                                     i*PWM_DEVICE_DMA_CHANNEL_PAGE_SIZE;
        channels[channel_id].pm[i].va[0] = 0;
        
        if(read(fd, &pfn, sizeof(pfn)) != sizeof(pfn))
            error("Invalid size of pfn making pagemap");
        
        if(((pfn >> 55) & 0x1bf) != 0x10c)
            error("Invalid pfn making pagemap");
        
        channels[channel_id].pm[i].pa = (uint32_t) pfn << 
            PWM_DEVICE_DMA_CHANNEL_PAGE_SHIFT | 0x40000000;
    }

    close(fd);
    close(memfd);
}

static void init_channel_dma(int channel_id) {
    dma_s    *dma    = (dma_s*) get_channel_dma(channel_id);
    uint32_t *sample = (uint32_t*) channels[channel_id].vb;

    uint32_t phys_fifo_addr;
    size_t   i;

    channels[channel_id].dma_a = 
        (uint32_t*) map_peripheral(PWM_DEVICE_DMA_ADDRESS, PWM_DEVICE_DMA_CHANNEL_SIZE) +
        (PWM_DEVICE_DMA_CHANNEL_INCREASE_SHIFT * channel_id);
    
    if(channels[channel_id].dma_a == NULL)
        error("Invalid dma_a initiating channel dma");

    phys_fifo_addr = (PWM_DEVICE_PWM_ADDRESS | 0x7e000000) + 0x18;

    memset(sample, 0, sizeof(channels[channel_id].num_samples * sizeof(uint32_t)));

    for(i = 0; i < channels[channel_id].num_samples; ++i) {
        dma->ti     = PWM_DEVICE_DMA_NO_WIDE_BURSTS | PWM_DEVICE_DMA_WAIT_RESP;
        dma->sa     = get_channel_v_to_p(channel_id, sample + i);
        dma->da     = PWM_DEVICE_PHYS_GPCLR0;
        dma->size   = 4;
        dma->stride = 0;
        dma->next   = get_channel_v_to_p(channel_id, dma + 1);
        ++dma;

        dma->ti =
            PWM_DEVICE_DMA_NO_WIDE_BURSTS |
            PWM_DEVICE_DMA_WAIT_RESP      |
            PWM_DEVICE_DMA_D_DREQ         |
            PWM_DEVICE_DMA_PER_MAP(5);

        dma->sa     = get_channel_v_to_p(channel_id, sample);
        dma->da     = phys_fifo_addr;
        dma->size   = 4;
        dma->stride = 0;
        dma->next   = get_channel_v_to_p(channel_id, dma + 1);
        ++dma;
    }

    --dma;
    dma->next = get_channel_v_to_p(channel_id, get_channel_dma(channel_id));

    channels[channel_id].dma_a[PWM_DEVICE_DMA_CS] = PWM_DEVICE_DMA_RESET;
    udelay(10);

    channels[channel_id].dma_a[PWM_DEVICE_DMA_CS]        = PWM_DEVICE_DMA_INT | PWM_DEVICE_DMA_END;
    channels[channel_id].dma_a[PWM_DEVICE_DMA_CONBLK_AD] = get_channel_v_to_p(channel_id, get_channel_dma(channel_id));
    channels[channel_id].dma_a[PWM_DEVICE_DMA_DEBUG]     = 7;
    channels[channel_id].dma_a[PWM_DEVICE_DMA_CS]        = 0x10880001;
}


static void uninit_channel(int channel_id) {
    dma_s*    dma = (dma_s*) get_channel_dma(channel_id);
    uint32_t* vb  = (uint32_t*) channels[channel_id].vb;
    int i;

    if(!channels[channel_id].vb)
        error("Invalid VB clearing channel");

    for(i = 0; i < channels[channel_id].num_samples; ++i) {
        dma->da = PWM_DEVICE_PHYS_GPCLR0;
        dma    += 2;
    }
    udelay(PWM_DEVICE_SUBCYCLE_TIME_US);

    for(i = 0; i < channels[channel_id].num_samples; ++i) {
        *(vb + i) = 0;
    }
    udelay(PWM_DEVICE_SUBCYCLE_TIME_US);
            
    channels[channel_id].dma_a[PWM_DEVICE_DMA_CS] = PWM_DEVICE_DMA_RESET;
    udelay(10);

    channels_setup -= 1 << channel_id;
}

static void clear_channel(int channel_id, int pin_id) {
    uint32_t* vb = (uint32_t*) channels[channel_id].vb;
    int i;

    if (!channels[channel_id].vb)
        error("Invalid vb clearing before setting channel");
    
    if ((pins_setup & 1 << pin_id) == 0)
        error("Invalid pin_id");

    for (i = 0; i < channels[channel_id].num_samples; ++i) {
        *(vb + i) &= ~(1 << pin_id);
    }

    set_pin_level(pin_id, 0);
}

static void set_channel(int channel_id, int pin_id, int width) {
    uint32_t* vb = (uint32_t*) channels[channel_id].vb;
    dma_s*   dma = (dma_s*) get_channel_dma(channel_id);
    int i;

    if(!channels[channel_id].vb)
        error("Invalid vb setting channel");
    
    if(width > channels[channel_id].num_max)
        error("Invalid width setting channel");

    if((pins_setup & 1 << pin_id) == 0)
        error("Invalid pin setting channel");

    *(vb) |= 1 << pin_id;
    dma->da = PWM_DEVICE_PHYS_GPSET0;

    if(width > 0)
        for(i = 1; i < width - 1; ++i) {
            *(vb + i) &= ~(1 << pin_id);
            dma += 2;
        }

    *(vb + width) |= 1 << pin_id;
    dma->da = PWM_DEVICE_PHYS_GPCLR0;
}

static uint8_t* get_channel_dma(int channel_id) {
    return channels[channel_id].vb +
           (sizeof(uint32_t) * channels[channel_id].num_samples);
}

static uint32_t get_channel_v_to_p(int channel_id, void *v) {
    uint32_t offset = (uint8_t*) v - channels[channel_id].vb;
    
    return channels[channel_id].pm[offset >> PWM_DEVICE_DMA_CHANNEL_PAGE_SHIFT].pa +
           (offset % PWM_DEVICE_DMA_CHANNEL_PAGE_SIZE);
}

/******************
 * PWM DMA control
 */

static void init(void) {
    pwm_region  = (uint32_t*) map_peripheral(PWM_DEVICE_PWM_ADDRESS,  PWM_DEVICE_PWM_SIZE);
    pcm_region  = (uint32_t*) map_peripheral(PWM_DEVICE_PCM_ADDRESS,  PWM_DEVICE_PCM_SIZE);
    clk_region  = (uint32_t*) map_peripheral(PWM_DEVICE_CLK_ADDRESS,  PWM_DEVICE_CLK_SIZE);
    gpio_region = (uint32_t*) map_peripheral(PWM_DEVICE_GPIO_ADDRESS, PWM_DEVICE_GPIO_SIZE);

    if(pwm_region == NULL || pcm_region == NULL || clk_region == NULL || gpio_region == NULL)
        error("Failed to mmap initiating");

    pwm_region[PWM_DEVICE_PWM_CTL]     = 0; 
    udelay(10);
    clk_region[PWM_DEVICE_PWMCLK_CNTL] = 0x5A000006;
    udelay(100);
    clk_region[PWM_DEVICE_PWMCLK_DIV]  = 0x5A000000 | (50<<12);
    udelay(100);
    clk_region[PWM_DEVICE_PWMCLK_CNTL] = 0x5A000016;
    udelay(100);
    pwm_region[PWM_DEVICE_PWM_RNG1]    = PWM_DEVICE_PULSE_WIDTH_INCREMENT_US * 10;
    udelay(10);
    pwm_region[PWM_DEVICE_PWM_DMAC]    = PWM_DEVICE_PWMDMAC_ENAB | PWM_DEVICE_PWMDMAC_THRSHLD;
    udelay(10);
    pwm_region[PWM_DEVICE_PWM_CTL]     = PWM_DEVICE_PWMCTL_CLRF;
    udelay(10);
    pwm_region[PWM_DEVICE_PWM_CTL]     = PWM_DEVICE_PWMCTL_USEF1 | PWM_DEVICE_PWMCTL_PWEN1;
    udelay(10);
}

static void uninit(void) {
    size_t i;

    for(i = 0; i < PWM_DEVICE_DMA_CHANNELS_NUMBER; ++i) {
        if(channels[i].dma_a && channels[i].vb) {
            uninit_channel(i);
        }
    }
}

/******************
 * PWM namespace
 */

void PWM::init(void) {
    ::init();
}

void PWM::uninit(void) {
    ::uninit();
}

PWM::Device::Device(int channel_id, int pin_id) {
    this->channel_id = channel_id;
    this->pin_id     = pin_id;

    init_pin(pin_id);
    init_channel(channel_id);
}

PWM::Device::~Device(void) {
    uninit_channel(this->channel_id);
    uninit_pin(this->pin_id);
}

void PWM::Device::set(int value) {
    this->value = value;

    clear_channel(this->channel_id, this->pin_id);
    set_channel(this->channel_id, this->pin_id, value);

    printf("%d %d %d\n", this->channel_id, this->pin_id, this->value);
}

void PWM::Device::clear(void) {
    clear_channel(this->channel_id, this->pin_id);
}
