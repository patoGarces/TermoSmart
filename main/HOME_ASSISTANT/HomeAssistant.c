#include "HomeAssistant.h"
#include "mqttClient.h"
#include "stdio.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "string.h"
#include "secrets.h"

#define TEMP_TOPIC_CONFIG "homeassistant/sensor/termosmart_temperature/config"
#define TEMP_TOPIC_VALUE  "termosmart/temperature"

#define POWER_TOPIC_CONFIG "homeassistant/switch/termosmart_power/config"
#define POWER_TOPIC_STATE  "termosmart/power/state"
#define POWER_TOPIC_SET    "termosmart/power/set"

static const char *TAG = "HomeAssistant";

static QueueHandle_t actualTermoStateQueue;

static const char *temperature_discovery =
        "{"
        "\"name\":\"Temperatura\","
        "\"unique_id\":\"termosmart_temperature\","
        "\"state_topic\":\"termosmart/temperature\","
        "\"unit_of_measurement\":\"°C\","
        "\"device_class\":\"temperature\","
        "\"state_class\":\"measurement\","
        "\"device\":{"
            "\"identifiers\":[\"termosmart\"],"
            "\"name\":\"TermoSmart\","
            "\"manufacturer\":\"patoGarces\","
            "\"model\":\"ESP32-C6\""
        "}"
        "}";

static const char *switch_discovery =
    "{"
    "\"name\":\"Encendido\","
    "\"unique_id\":\"termosmart_power\","
    "\"state_topic\":\"termosmart/power/state\","
    "\"command_topic\":\"termosmart/power/set\","
    "\"retain\":\"true\","
    "\"payload_on\":\"ON\","
    "\"payload_off\":\"OFF\","
    "\"device\":{"
        "\"identifiers\":[\"termosmart\"],"
        "\"name\":\"TermoSmart\","
        "\"manufacturer\":\"Patricio\","
        "\"model\":\"ESP32-C6\""
    "}"
    "}";


void updateHATemp(float actualTemp) {

    char actualTempChar[6];
    snprintf(actualTempChar, sizeof(actualTempChar), "%.2f", actualTemp);

    mqttPublishTopic(
        TEMP_TOPIC_VALUE,
        actualTempChar,
        false
    );
}

void updateHAStatePower(bool isPowerOn) {    
    mqttPublishTopic(
        POWER_TOPIC_STATE,
        isPowerOn ? "ON" : "OFF",
        true
    );
}

static void homeAssistantUpdate(void *pvParameters) {
    bool isReleOn = false;
    bool isMqttConnectionState = false, lastMqttConnectionState = false;

    mqtt_context_t *ctx = (mqtt_context_t *)pvParameters;
    mqtt_message_t newMessage;

    while(1) {

        if (xQueuePeek(ctx->connectionQueue, &isMqttConnectionState, 0)) {
            if (isMqttConnectionState != lastMqttConnectionState) {
                lastMqttConnectionState = isMqttConnectionState;
                 ESP_LOGI(TAG, "Estado de conexion MQTT: %s", isMqttConnectionState ? "Conectado" : "Desconectado");

                if (isMqttConnectionState) {
                        mqttPublishTopic(
                        TEMP_TOPIC_CONFIG,
                        temperature_discovery,
                        true
                    );

                    mqttPublishTopic(
                        POWER_TOPIC_CONFIG,
                        switch_discovery,
                        true
                    );
                    mqttSuscribeTopic(POWER_TOPIC_SET, 1);
                }
            }
        }
        if (xQueueReceive(ctx->mqttMessageQueue, &newMessage, 0)) {
            ESP_LOGI(TAG, "Nuevo mensaje del topic: %s, con data: %s", newMessage.topic, newMessage.payload);

            if (strcmp(newMessage.topic, POWER_TOPIC_SET) == 0) {
                isReleOn = strcmp(newMessage.payload, "ON") == 0;
                ESP_LOGI(TAG, "TENGO QUE CAMBIAR EL ESTADO DEL RELE A: %d", isReleOn);

                xQueueSend(actualTermoStateQueue, &isReleOn, 0);
            }
        }
        updateHAStatePower(isReleOn);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void homeAssistantInit(QueueHandle_t updateTermoStateQueue, QueueHandle_t mqttConnectionState) {

    actualTermoStateQueue = updateTermoStateQueue;
    static mqtt_context_t mqttContext;
    mqttContext.connectionQueue = mqttConnectionState;
    mqttContext.mqttMessageQueue = xQueueCreate(10, sizeof(mqtt_message_t));

    mqttInit(MQTT_ADDRESS, &mqttContext);

    xTaskCreate(homeAssistantUpdate, "home assistant update", 4096, &mqttContext, 5, NULL);
}