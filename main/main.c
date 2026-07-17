/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "ST7789.h"
#include "SD_SPI.h"
#include "RGB.h"
#include "Dashboard.h"
#include "WifiHandler.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "HomeAssistant.h"
#include "secrets.h"
#include "driver/gpio.h"
#include "TemperatureTermo.h"

#define GPIO_SWITCH     2

QueueHandle_t networkStateQueue;
QueueHandle_t mqttStateQueue;
QueueHandle_t updateRelayStateQueue;

static void setSwitchStateOutput(bool state) {
    gpio_set_level(GPIO_SWITCH, state);
}

static uint8_t tempToPercent(float temp) {
    const float minTemp = 20.0f;
    const float maxTemp = 60.0f;

    if (temp <= minTemp)
        return 0;

    if (temp >= maxTemp)
        return 100;

    return (uint8_t)(((temp - minTemp) * 100.0f) / (maxTemp - minTemp));
}

static void networkStatusTask(void *pvParameters) {
    bool networkState = false, lastNetworkState = false, mqttState = false, lastMqttState = false;
    const char *TAG = "networkStatus";

    while(1) {
        xQueuePeek(networkStateQueue, &networkState, 0);
        if (networkState != lastNetworkState) {
            ESP_LOGI(TAG, "Nuevo estado de conexion wifi: %s", networkState ? "Conectado" : "Desconectado");

            if (networkState) {
                homeAssistantInit(updateRelayStateQueue, mqttStateQueue);
            }
            updateDashboardWifiStatus(networkState);
            lastNetworkState = networkState;
        }

        xQueuePeek(mqttStateQueue, &mqttState, 0);
        if (mqttState != lastMqttState) {
            ESP_LOGI(TAG, "Nuevo estado de conexion mqtt: %s", mqttState ? "Conectado" : "Desconectado");

            updateDashboardMqttStatus(mqttState);
            lastMqttState = mqttState;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void updateSystemStatus(void *pvParameters) {
    bool termoState = false;
    float actualTemp = 0.00;
    const char *TAG = "updateSystemStatus";
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while(1) {
        if (xQueueReceive(updateRelayStateQueue, &termoState, 0)) {
            ESP_LOGI(TAG, "Nuevo estado termo: %s", termoState ? "ENCENDIDO" : "APAGADO");
            updateDashboardRelayStatus(termoState);

            setSwitchStateOutput(termoState);
        }

        actualTemp = getTermoTemp();
        updateHATemp(actualTemp);
        updateDashboardTemp(actualTemp);

        rgb_t tempColor = convertTempPercentToRgb(tempToPercent(actualTemp));
        setRgb(tempColor.r, tempColor.g, tempColor.b);

        if (actualTemp < 40.0f) {
            updateDashboardTempColor(TEMP_COLOR_COLD);
        }
        else if (actualTemp <= 47.0f) {
            updateDashboardTempColor(TEMP_COLOR_WARM);
        }
        else {
            updateDashboardTempColor(TEMP_COLOR_HOT);
        }
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
    }
}

void app_main(void) {

    gpio_config_t switchGpio = {
        .pin_bit_mask = 1 << GPIO_SWITCH,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = false,
        .pull_down_en = false,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&switchGpio);

    // Queues create
    updateRelayStateQueue = xQueueCreate(1, sizeof(bool));
    networkStateQueue = xQueueCreate(1, sizeof(bool));
    mqttStateQueue = xQueueCreate(1, sizeof(bool));

    initWifi(ESP_WIFI_SSID_STA, ESP_WIFI_PASS_STA, WIFI_MODE_STA, networkStateQueue);
    Flash_Searching();
    initRgb();
    SD_Init();                              // SD must be initialized behind the LCD
    LCD_Init();
    BK_Light(50);
    LVGL_Init();                            // returns the screen object

    dashboardInit();

    temperatureTermoInit(GPIO_NUM_1);

    xTaskCreate(networkStatusTask,"network status task", 4096, NULL, 4,NULL);;
    xTaskCreate(updateSystemStatus,"update system status", 4096, NULL, 4,NULL);;

    while (1) {
        // raise the task priority of LVGL and/or reduce the handler period can improve the performance
        vTaskDelay(pdMS_TO_TICKS(10));
        // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
        lv_timer_handler();
    }
}
