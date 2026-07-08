#pragma once
#include "stdio.h"
#include "stdbool.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef struct {
    QueueHandle_t connectionQueue;
    QueueHandle_t mqttMessageQueue;
} mqtt_context_t;

typedef struct {
    char topic[128];
    char payload[256];
} mqtt_message_t;

void mqttInit(char *hostAddress, mqtt_context_t *mqttContext);
void mqttPublishTopic(char *topic, char *message, bool isRetained);
uint32_t mqttSuscribeTopic(char *topic, int8_t qos);