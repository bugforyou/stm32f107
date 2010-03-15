#ifndef __LED_H__
#define __LED_H__
#include <rtthread.h>

void rt_hw_led_init(void);
void rt_hw_led_on(rt_uint32_t led);
void rt_hw_led_off(rt_uint32_t led);

#endif
