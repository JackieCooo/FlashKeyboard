#include "soc_osal.h"
#include "app_os_init.h"
#include "common_def.h"
#include "std_def.h"

#include "debug.h"

#define LOG_TAG "main"


void app_main(void *unused)
{
    UNUSED(unused);

    while (1) {
        osal_msleep(1000);
        LOG("working.");
    }
}
