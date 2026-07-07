#include "HomeAssistant.h"
#include "mqttClient.h"
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MQTT_ADDRESS "mqtt://192.168.2.113:1883"

#define TEMP_TOPIC_CONFIG "homeassistant/sensor/termosmart_temperature/config"
#define TEMP_TOPIC_VALUE  "termosmart/temperature"

#define POWER_TOPIC_CONFIG "homeassistant/switch/termosmart_power/config"
#define POWER_TOPIC_STATE  "termosmart/power/state"
#define POWER_TOPIC_SET    "termosmart/power/set"

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
    "\"payload_on\":\"ON\","
    "\"payload_off\":\"OFF\","
    "\"device\":{"
        "\"identifiers\":[\"termosmart\"],"
        "\"name\":\"TermoSmart\","
        "\"manufacturer\":\"Patricio\","
        "\"model\":\"ESP32-C6\""
    "}"
    "}";


static void sentUpdateTemp(float actualTemp) {

    char actualTempChar[6];
    snprintf(actualTempChar, sizeof(actualTempChar), "%.2f", actualTemp);

    mqttPublishTopic(
        TEMP_TOPIC_VALUE,
        actualTempChar,
        false
    );
}

static void homeAssistantUpdate(void *pvParameters) {

    float actualTemp = 10.0;

    while(1) {

        sentUpdateTemp(actualTemp);
        actualTemp += 0.1;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void homeAssistantInit() {

    mqttInit(MQTT_ADDRESS);
    
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


    sentUpdateTemp(12.5);

    mqttSuscribeTopic(POWER_TOPIC_SET, 1);

    xTaskCreate(homeAssistantUpdate, "home assistant update", 2048, NULL, 5, NULL);
}