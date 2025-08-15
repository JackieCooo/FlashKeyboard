#ifndef __SERVICE_CONTROLLER_H__
#define __SERVICE_CONTROLLER_H__

#include <stdint.h>


void service_ctrl_init(void);
void service_ctrl_deinit(void);
void service_ctrl_detect(void);
int service_ctrl_send_key(uint32_t key);

#endif
