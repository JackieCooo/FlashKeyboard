#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "soc_osal.h"


#define LOG(fmt, ...) osal_printk("[" LOG_TAG "] " fmt "\r\n", ##__VA_ARGS__)

#define LOG_BUF(msg, buf, len) \
{ \
    osal_printk("[" LOG_TAG "] " msg ": "); \
    for (size_t i = 0; i < (len); i++) { \
        osal_printk("%02X ", ((uint8_t*)(buf))[i]); \
    } \
    osal_printk("\r\n"); \
}

#endif
