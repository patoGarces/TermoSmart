#pragma once

#include "driver/gpio.h"

void temperatureTermoInit(gpio_num_t gpioTemp);
float getTermoTemp();