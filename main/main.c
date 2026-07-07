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
#include "mqtt_client.h"

#define ESP_WIFI_SSID_STA "WIFI2658"
#define ESP_WIFI_PASS_STA "KO9604AL"

#define MQTT_ADDRESS "mqtt://192.168.2.113:1883"

QueueHandle_t networkStateQueueHandler;

esp_mqtt_client_handle_t mqtt_client = NULL;

static void networkStatusTask(void *pvParameters) {
    bool networkState = false, lastNetworkState = false;
    const char *TAG = "networkStatus";

    while(1) {
        xQueuePeek(networkStateQueueHandler, &networkState, 0);
        if (networkState != lastNetworkState) {
            ESP_LOGI(TAG, "Nuevo estado de conexion wifi: %s", networkState ? "Conectado" : "Desconectado");
            lastNetworkState = networkState;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void MqttInit(void)
{
    const char *TAG = "mqttInit";
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_ADDRESS,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

    ESP_ERROR_CHECK(esp_mqtt_client_start(mqtt_client));

    esp_mqtt_client_publish(
        mqtt_client,
        "termosmart/test",
        "Hola desde ESP32",
        0,
        1,
        0
    );

    ESP_LOGI(TAG, "MQTT initialized");
}

void app_main(void)
{

    networkStateQueueHandler = xQueueCreate(1, sizeof(bool));
    initWifi(ESP_WIFI_SSID_STA, ESP_WIFI_PASS_STA, WIFI_MODE_STA, networkStateQueueHandler);
    Flash_Searching();
    RGB_Init();
    RGB_Example();
    SD_Init();                              // SD must be initialized behind the LCD
    LCD_Init();
    BK_Light(50);
    LVGL_Init();                            // returns the screen object

/********************* Demo *********************/
    dashboardInit();

    MqttInit();

    // lv_demo_widgets();
    // lv_demo_keypad_encoder();
    // lv_demo_benchmark();
    // lv_demo_stress();
    // lv_demo_music();

    xTaskCreate(networkStatusTask,"network status tag", 4096, NULL, 4,NULL);;

    while (1) {
        // raise the task priority of LVGL and/or reduce the handler period can improve the performance
        vTaskDelay(pdMS_TO_TICKS(10));
        // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
        lv_timer_handler();
    }
}
