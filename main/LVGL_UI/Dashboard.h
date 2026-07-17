#pragma once

#include "lvgl.h"
#include "LVGL_Driver.h"
#include "SD_SPI.h"


#define TEMP_COLOR_COLD       lv_color_hex(0x1565C0)  // Azul
#define TEMP_COLOR_WARM       lv_color_hex(0xFB8C00)  // Naranja
#define TEMP_COLOR_HOT        lv_color_hex(0xC62828)  // Rojo


void dashboardInit(void);
void updateDashboardTemp(float newTemp);
void updateDashboardWifiStatus(bool connected);
void updateDashboardMqttStatus(bool connected);
void updateDashboardRelayStatus(bool on);
void updateDashboardTempColor(lv_color_t newColor);