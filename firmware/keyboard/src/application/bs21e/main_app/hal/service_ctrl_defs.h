#ifndef __SERVICE_CTRL_DEFS_H__
#define __SERVICE_CTRL_DEFS_H__

#include <stdbool.h>
#include <stdint.h>


typedef struct {
    int (*init)(void);
    int (*deinit)(void);
    bool (*connected)(void);
    int (*send)(const void*, uint32_t len);
} service_ctrl_func_t;

#endif
