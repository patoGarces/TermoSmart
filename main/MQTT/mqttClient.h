#pragma once
#include "stdio.h"
#include "stdbool.h"


void mqttInit(char *hostAddress);
void mqttPublishTopic(char *topic, char *message, bool isRetained);
uint32_t mqttSuscribeTopic(char *topic, int8_t qos);