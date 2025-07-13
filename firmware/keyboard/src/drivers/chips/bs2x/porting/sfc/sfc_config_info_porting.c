#include "common_def.h"
#include "sfc.h"
#include "sfc_config_info_porting.h"
#include "sfc_config_info.h"

#if defined(CONFIG_SFC_USE_CUSTOMIZED_DEVICE_INFO)
static const flash_cmd_execute_t g_flash_gd_bus_enable[] = {
    { FLASH_CMD_TYPE_END,         0, { 0x0 }}
};

static const spi_opreation_t g_flash_common_read_cmds[] = {
    {SPI_CMD_SUPPORT, 0x03, 0x0, 0x0},
    {SPI_CMD_SUPPORT, 0x0B, 0x0, 0x1},
    {SPI_CMD_SUPPORT, 0x3B, 0x1, 0x1},
    {SPI_CMD_SUPPORT, 0xBB, 0x2, 0x1},
    {SPI_CMD_SUPPORT, 0x6B, 0x5, 0x1},
    {SPI_CMD_SUPPORT, 0xEB, 0x6, 0x3}
};

static const spi_opreation_t g_flash_common_write_cmds[] = {
    {SPI_CMD_SUPPORT, 0x0, 0x0, 0},
    {SPI_CMD_SUPPORT, 0x02, 0x0, 0},
    SPI_CMD_UNSUPPORT,
    SPI_CMD_UNSUPPORT,
    {SPI_CMD_SUPPORT, 0x32, 0x5, 0},
    SPI_CMD_UNSUPPORT,
};

static const spi_opreation_t g_flash_common_erase_cmds[] = {
    {SPI_CMD_SUPPORT, 0xC7, 0x0, CHIP_SIZE},
    {SPI_CMD_SUPPORT, 0xD8, 0x0, _64K},
    {SPI_CMD_SUPPORT, 0x52, 0x0, _32K},
    {SPI_CMD_SUPPORT, 0x20, 0x0, _4K}
};

static const flash_spi_info_t g_flash_spi_infos[] = {
#ifdef FLASH_1M
    { FLASH_GD25LE80, FLASH_SIZE_1MB, 4,
      (spi_opreation_t *)g_flash_common_read_cmds, (spi_opreation_t *)g_flash_common_write_cmds,
      (spi_opreation_t *)g_flash_common_erase_cmds, (flash_cmd_execute_t *)g_flash_gd_bus_enable },
#else
    { FLASH_GD25LE40, FLASH_SIZE_512KB, 4,
      (spi_opreation_t *)g_flash_common_read_cmds, (spi_opreation_t *)g_flash_common_write_cmds,
      (spi_opreation_t *)g_flash_common_erase_cmds, (flash_cmd_execute_t *)g_flash_gd_bus_enable },
#endif
};
#endif

static const flash_spi_info_t g_flash_spi_unknown_info = {
    FLASH_UNKOWN, FLASH_SIZE_512KB, 3,
    (spi_opreation_t *)g_default_read_cmds, (spi_opreation_t *)g_default_write_cmds,
    (spi_opreation_t *)g_default_erase_cmds, (flash_cmd_execute_t *)g_default_quad_enable
};

flash_spi_info_t *sfc_port_get_flash_spi_infos(void)
{
    return (flash_spi_info_t *)g_flash_spi_infos;
}

uint32_t sfc_port_get_flash_num(void)
{
    return sizeof(g_flash_spi_infos) / sizeof(flash_spi_info_t);
}

flash_spi_info_t *sfc_port_get_unknown_flash_info(void)
{
    return (flash_spi_info_t *)&g_flash_spi_unknown_info;
}
