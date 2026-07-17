#pragma once
#include "stdbool.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

void homeAssistantInit(QueueHandle_t updateTermoStateQueue, QueueHandle_t mqttConnectionState);
void updateHAStatePower(bool isPowerOn);
void updateHATemp(float actualTemp);
