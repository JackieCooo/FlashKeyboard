#ifndef __SERVICE_CTRL_DEFS_H__
#define __SERVICE_CTRL_DEFS_H__

#include <stdbool.h>
#include <stdint.h>


typedef void(*service_ctrl_receive_cb_t)(const void*, uint32_t);

typedef struct {
    int (*init)(void);
    int (*deinit)(void);
    bool (*connected)(void);
    int (*send)(const void*, uint32_t);
    void (*set_receive_callback)(service_ctrl_receive_cb_t);
} service_ctrl_func_t;

#endif
