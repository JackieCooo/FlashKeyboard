/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description:  PMU DRIVER
 *
 * Create:  2023-11-06
 */

#include "non_os.h"
#include "chip_io.h"
#include "tcxo.h"
#if defined(BUILD_APPLICATION_STANDARD)
#include "pm_pmu.h"
#endif
#include "pmu.h"

#ifdef BUILD_FLASHBOOT
static void clock_protocol_rst(void)
{
    writel(0x570000B8, 0x0); // B_SOC_SUB初始化
    writel(0x570000B8, 0x1);
    writel(0x570000B8, 0x3);
}

void pmu_init(void)
{
    // buck & micldo升压
    writew(0x5702C080, 0x114);
    // cldo升压
    writew(0x5702C088, 0x559);

    /* SFC提频 */
    reg32_setbits(0x52000540, 0, 1, 1);
    writel(0x52000574, 0x5);
    /* CPU提频 */
    writew(0x57000100, 0x0);
    writew(0x570000FC, 0x5);
    writew(0x57000100, 0x1);

    // buck效率优化配置
    writew(0x5702C410, 0x10);   // BUCK_CFG_REG_4
    writew(0x5702C414, 0xB0);   // BUCK_CFG_REG_5
    writew(0x5702C404, 0x321);  // BUCK_CFG_REG_1
    writew(0x5702C40C, 0x180);  // BUCK_CFG_REG_3

    writel(0x57008410, 0x40);   // XOCLKOUTDIE(S_MGPIO25): 寄存器控制关闭
    clock_protocol_rst();
}
#else /* IF BUILD_APPLICATION_STANDARD */
void pmu_init(void)
{
    if (non_os_is_driver_initialised(DRIVER_INIT_PMU) == true) {
        return;
    }

#ifdef CONFIG_POWER_SUPPLY_BY_LDO
    // 关闭buck
    reg16_clrbit(0x5702C270, 0x0);
    reg16_setbit(0x5702C26C, 0x0);
    uapi_pmu_ldo_set_voltage(PMU_LDO_ID_MICLDO, PMU_MICLDO_VSET_1V3);
#else
    uapi_pmu_ldo_set_voltage(PMU_LDO_ID_BUCK_1P1, PMU_BUCK_1P1_VSET_1V2);
    // 关闭micldo
    uapi_pmu_control(PMU_CONTROL_MICLDO_POWER, PMU_CONTROL_POWER_OFF);
#endif
    uapi_pmu_ldo_set_voltage(PMU_LDO_ID_CLDO, PMU_CLDO_VSET_1V1);

    // 由各个模块打开
    reg16_setbits(0x5702C230, 0x8, 0x3, 0x0);

#if defined(CONFIG_SUPPORT_NFC_SERVICE)
    uapi_pmu_ldo_set_voltage(PMU_LDO_ID_NFCLDO, PMU_NFCLDO_VSET_1V2);
    uapi_pmu_control(PMU_CONTROL_NFCLDO_POWER, PMU_CONTROL_POWER_ON);
#else
    writew(0x5702C234, 0);
    // 高阻
    reg32_clrbit(0x5702C230, 0x9);
    writew(0x5702C974, 0x3);    // NFC_LIMITER_SENSE_ENB_CFG：手动打开
#endif

    return;
}
#endif /* BUILD_APPLICATION_STANDARD END */
