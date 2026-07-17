#include "TemperatureTermo.h"
#include "ds18x20.h"
#include "esp_log.h"

static const char *TAG = "TemperatureTermo";

static gpio_num_t gpioSensor;


float getTermoTemp() {
    float actualTemp = 0.00;

    esp_err_t err = ds18x20_measure(gpioSensor, DS18X20_ANY, true);

    if (err != ESP_OK) {
        ESP_LOGW(TAG, "DS18B20 no detectado");
        return -1.00;
    } 

    ds18b20_read_temperature(gpioSensor, DS18X20_ANY, &actualTemp);

    ESP_LOGI(TAG, "Temperatura: %.2f °C", actualTemp);

    return actualTemp; 
}

void temperatureTermoInit(gpio_num_t gpioTemp) {

    gpioSensor = gpioTemp;

    esp_err_t err = ds18x20_measure(gpioTemp, DS18X20_ANY, true);

    if (err != ESP_OK) {
        ESP_LOGW(TAG, "DS18B20 no detectado");
        return;
    } 
}
