#pragma once

#include "lvgl.h"
#include "demos/lv_demos.h"

#include "LVGL_Driver.h"
#include "SD_SPI.h"

#define DASHBOARD_LVGL_TICK_PERIOD_MS  1000


void dashboardInit(void);