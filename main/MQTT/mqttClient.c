#include "mqttClient.h"
#include "mqtt_client.h"
#include "esp_log.h"

esp_mqtt_client_handle_t mqtt_client = NULL;
const char *TAG = "MqttClient";

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqttEventHandler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%ld", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    // esp_mqtt_client_handle_t client = event->client;

    mqtt_context_t *ctx = (mqtt_context_t *)handler_args;
    bool isConnected = false;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        isConnected = true;
        xQueueOverwrite(ctx->connectionQueue, &isConnected);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        isConnected = false,
        xQueueOverwrite(ctx->connectionQueue, &isConnected);
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        // ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        ESP_LOGI(TAG, "NEW MESSAGE -> Topic=%.*s\tData=%.*s", event->topic_len, event->topic, event->data_len, event->data);
        mqtt_message_t msg = {0};
        snprintf(msg.topic, sizeof(msg.topic),
                "%.*s", event->topic_len, event->topic);

        snprintf(msg.payload, sizeof(msg.payload),
                "%.*s", event->data_len, event->data);

        xQueueSend(ctx->mqttMessageQueue, &msg, 0);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqttPublishTopic(char *topic, char *message, bool isRetained) {
    esp_mqtt_client_publish(mqtt_client, topic, message, 0, 1, isRetained);
}


uint32_t mqttSuscribeTopic(char *topic, int8_t qos) {
    return esp_mqtt_client_subscribe(mqtt_client, topic, qos);
}


void mqttInit(char *hostAddress, mqtt_context_t *mqttContext) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = hostAddress,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

    ESP_ERROR_CHECK(esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqttEventHandler, mqttContext));
    ESP_ERROR_CHECK(esp_mqtt_client_start(mqtt_client));

    ESP_LOGI(TAG, "MQTT initialized");
}