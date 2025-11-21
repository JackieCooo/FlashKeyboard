#include <stdbool.h>

#include "oled.h"

#include "spi.h"
#include "pinctrl.h"
#include "gpio.h"
#include "dma.h"

#define LOG_TAG "oled"
#include "debug.h"

#define SPI_BUS SPI_BUS_1

#define CS_PIN(x) uapi_gpio_set_val(11, x)
#define DC_PIN(x) uapi_gpio_set_val(12, x)

typedef struct {
    uint8_t p1;  // 起始页
    uint8_t p2;  // 结尾页
    uint8_t y1_off;  // y距页起始的偏移
    uint8_t y2_off;  // y距页结尾的偏移
} window_t;


static uint8_t g_gram[(CONFIG_SCREEN_HEIGHT >> 3) * (CONFIG_SCREEN_WIDTH)] = {0};


static int oled_bsp_init(void)
{
    int ret = 0;

    /* SCLK */
    uapi_pin_set_mode(CONFIG_OLED_SCLK_PIN, HAL_PIO_SPI1_CLK);

    /* MOSI */
    uapi_pin_set_mode(CONFIG_OLED_MOSI_PIN, HAL_PIO_SPI1_TXD);

    /* RST */
    if (CONFIG_OLED_RES_PIN > 0) {
        uapi_pin_set_mode(CONFIG_OLED_RES_PIN, HAL_PIO_FUNC_GPIO);
        uapi_gpio_set_dir(CONFIG_OLED_RES_PIN, GPIO_DIRECTION_OUTPUT);
        uapi_gpio_set_val(CONFIG_OLED_RES_PIN, GPIO_LEVEL_HIGH);
    }

    /* DC */
    uapi_pin_set_mode(CONFIG_OLED_DC_PIN, HAL_PIO_FUNC_GPIO);
    uapi_gpio_set_dir(CONFIG_OLED_DC_PIN, GPIO_DIRECTION_OUTPUT);

    /* CS */
    uapi_pin_set_mode(CONFIG_OLED_CS_PIN, HAL_PIO_FUNC_GPIO);
    uapi_gpio_set_dir(CONFIG_OLED_CS_PIN, GPIO_DIRECTION_OUTPUT);
    uapi_gpio_set_val(CONFIG_OLED_CS_PIN, GPIO_LEVEL_HIGH);

    /* 初始化SPI */
    spi_attr_t config = { 0 };

    config.freq_mhz = 8;                                        /* 工作频率 8MHz */
    config.is_slave = false;                                    /* 主机模式 */
    config.frame_size = HAL_SPI_FRAME_SIZE_8;                   /* 帧大小，8位 */
    config.slave_num = 1;                                       /* 1个从机 */
    config.spi_frame_format = HAL_SPI_FRAME_FORMAT_STANDARD;    /* 传输模式：标准 */
    config.bus_clk = SPI_CLK_FREQ;                              /* 总线速率 32MHz */
    config.frame_format = SPI_CFG_FRAME_FORMAT_MOTOROLA_SPI;    /* 协议格式：摩托罗拉SPI协议格式 */
    config.tmod = HAL_SPI_TRANS_MODE_TXRX;                      /* 传输模式：收发模式 */
    config.clk_phase = SPI_CFG_CLK_CPHA_0;                      /* 相位：空闲低电平 */
    config.clk_polarity = SPI_CFG_CLK_CPOL_0;                   /* 极性：第一个时钟沿采集 */

    spi_extra_attr_t ext_config = { 0 };

    ext_config.sspi_param.wait_cycles = 0x10;                   /* 等待周期 */

    ret = uapi_spi_init(SPI_BUS, &config, &ext_config);
    if (ret) {
        return ret;
    }

    /* 初始化DMA */
    uapi_dma_init();
    uapi_dma_open();

    return 0;
}

static int oled_write_bytes(bool is_data, uint8_t* bytes, size_t len)
{
    spi_xfer_data_t data = {
        .tx_buff = bytes,
        .tx_bytes = len,
    };

    DC_PIN(is_data);
    CS_PIN(0);
    int ret = uapi_spi_master_write(SPI_BUS, &data, CONFIG_SPI_MAX_TIMEOUT);
    CS_PIN(1);

    return ret;
}

static int oled_write_cmd(uint8_t cmd)
{
    return oled_write_bytes(false, &cmd, 1);
}

static void oled_reg_init(void)
{
    /* 进入睡眠 */
    oled_write_cmd(0xAE);
    /* 起始行地址 */
    oled_write_cmd(0x40);
    /* 对比度 */
    oled_write_cmd(0x81);
    oled_write_cmd(0xCF);  // A[7:0]
    /* 段重映射 */
    oled_write_cmd(0xA1);  // 0xA0：左右反置，0xA1：正常
    /* COM输出扫描方向 */
    oled_write_cmd(0xC8);  // 0xC0：上下反置，0xC8：正常
    /* Multiplex Ratio */
    oled_write_cmd(0xA8);
    oled_write_cmd(0x3f);  // 1/64 duty
    /* 屏幕偏移 */
    oled_write_cmd(0xD3);
    oled_write_cmd(0x00);  // A[5:0]
    /* 时钟频率 */
    oled_write_cmd(0xD5);
    oled_write_cmd(0x80);  // A[3:0]+1：分频系数；A[7:4]：晶振频率
    /* 预充电周期 */
    oled_write_cmd(0xD9);
    oled_write_cmd(0xF1);  // A[3:0]：第一阶段；A[7:4]：第二阶段
    /* COM配置 */
    oled_write_cmd(0xDA);
    oled_write_cmd(0x12);  // A[4]=0：序列；A[4]=1：另类；A[5]：COM左右重映射
    /* Vcomh */
    oled_write_cmd(0xDB);
    oled_write_cmd(0x40);  // A[6:4]
    /* 内存取址模式 */
    oled_write_cmd(0x20);
    oled_write_cmd(0x00);  // A[1:0]：0x00：行取址；0x01：列取址；0x02：页取址

    oled_write_cmd(0x8D);//--set Charge Pump enable/disable
    oled_write_cmd(0x14);//--set(0x10) disable
    /* 屏幕使能 */
    oled_write_cmd(0xA4);  // 0xA4：使用RAM内容；0xA5：忽略RAM内容
    /* 屏幕反转 */
    oled_write_cmd(0xA6);  // 0xA6：正常；0xA7：反转
    /* 唤醒屏幕 */
    oled_write_cmd(0xAF);
    /* 清屏 */
    oled_clear();
}

static int oled_open_window(uint8_t x, uint8_t y, uint8_t w, uint8_t h, window_t* window)
{
    if (window == NULL ||
        x >= CONFIG_SCREEN_WIDTH ||
        y >= CONFIG_SCREEN_HEIGHT ||
        w + x > CONFIG_SCREEN_WIDTH ||
        h + y > CONFIG_SCREEN_HEIGHT ||
        w == 0 || h == 0
    ) {
        LOG("invalid param");
        return -1;
    }

    window->y1_off = y % 8;
    window->y2_off = (y + h + window->y1_off) % 8 ? 8 - ((y + h + window->y1_off) % 8) : 0;
    window->p1 = y >> 3;
    window->p2 = ((y + h + window->y1_off + window->y2_off) >> 3) - 1;

    /* 设置列区间 */
    oled_write_cmd(0x21);
    oled_write_cmd(x);
    oled_write_cmd(x + w - 1);
    /* 设置页区间 */
    oled_write_cmd(0x22);
    oled_write_cmd(window->p1);
    oled_write_cmd(window->p2);

    return 0;
}

int oled_init(void)
{
    int ret = 0;

    /* 初始化板级驱动 */
    ret = oled_bsp_init();
    if (ret) {
        LOG("oled bsp init failed, err: %d", ret);
        return ret;
    }

    /* 初始化寄存器 */
    oled_reg_init();

    return 0;
}

int oled_set_data(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t* data)
{
    int ret = 0;

    /* 开窗 */
    window_t window = {0};
    ret = oled_open_window(x, y, w, h, &window);
    if (ret) {
        LOG("open window failed");
        return ret;
    }
    // LOG("p1=%d, p2=%d, y1_off=%d, y2_off=%d", window.p1, window.p2, window.y1_off, window.y2_off);

    /* 缓存数据 */
    for (uint8_t i = window.p1, *ptr = data; i <= window.p2; i++, ptr += w) {
        memcpy(g_gram + i * CONFIG_SCREEN_WIDTH + x, ptr, w);
    }

    /* 写数据 */
    return oled_write_bytes(true, g_gram + window.p1 * CONFIG_SCREEN_WIDTH + x, (window.p2 - window.p1 + 1) * w);
}

void oled_clear(void)
{
    memset(g_gram, 0, sizeof(g_gram));

    /* 设置列区间 */
    oled_write_cmd(0x21);
    oled_write_cmd(0);
    oled_write_cmd(CONFIG_SCREEN_WIDTH - 1);

    /* 设置页区间 */
    oled_write_cmd(0x22);
    oled_write_cmd(0);
    oled_write_cmd((CONFIG_SCREEN_HEIGHT >> 3) - 1);

    /* 写数据 */
    oled_write_bytes(true, g_gram, sizeof(g_gram));
}
