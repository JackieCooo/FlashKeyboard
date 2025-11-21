#ifndef __OLED_H__
#define __OLED_H__

#include <stdint.h>


int oled_init(void);
int oled_set_data(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t* data);
void oled_clear(void);

#endif
