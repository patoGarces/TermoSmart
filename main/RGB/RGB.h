#pragma once

#include "driver/gpio.h"
#include "led_strip.h"

#define COLOR_RED       (rgb_color_t){64,0,0}
#define COLOR_GREEN     (rgb_color_t){0,64,0}
#define COLOR_BLUE      (rgb_color_t){0,0,64}
#define COLOR_YELLOW    (rgb_color_t){64,64,0}
#define COLOR_WHITE     (rgb_color_t){64,64,64}
#define COLOR_BLACK     (rgb_color_t){0,0,0}

#define BLINK_GPIO 8

void RGB_Init(void);
void Set_RGB( uint8_t red_val, uint8_t green_val, uint8_t blue_val);
void RGB_Example(void);