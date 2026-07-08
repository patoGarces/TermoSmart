#pragma once
#include "stdbool.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

void homeAssistantInit(QueueHandle_t updateTermoStateQueue);
void updateHAStatePower(bool isPowerOn);
void updateHATemp(float actualTemp);
