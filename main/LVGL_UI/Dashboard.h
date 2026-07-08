#pragma once

#include "lvgl.h"
#include "demos/lv_demos.h"

#include "LVGL_Driver.h"
#include "SD_SPI.h"

#define DASHBOARD_LVGL_TICK_PERIOD_MS  1000


void dashboardInit(void);
void updateDashboardTemp(float newTemp);
void updateDashboardWifiStatus(bool connected);
void updateDashboardMqttStatus(bool connected);
void updateDashboardRelayStatus(bool on);