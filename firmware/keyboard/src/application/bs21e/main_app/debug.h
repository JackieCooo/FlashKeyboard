#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "soc_osal.h"


#define LOG(fmt, ...) osal_printk("[" LOG_TAG "] " fmt "\r\n", ##__VA_ARGS__)

#endif
