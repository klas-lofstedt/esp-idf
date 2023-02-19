// C
// FreeRTOS
// ESP32
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
//#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
//#include "esp_netif.h"
// #include "protocol_examples_common.h"
// #include "protocol_examples_common.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

//#include "esp_log.h"
//#include "mqtt_client.h"
//#include "esp_tls.h"
//#include "esp_ota_ops.h"
#include <sys/param.h>
#include <cJSON.h>
// App
#include "mqtt.h"
#include "misc.h"
//#include "gpio.h"


#define PIN_LED 5


#define TOPIC_SUB_LEN (DEVICE_ID_LEN + 9)
#define TOPIC_PUB_LEN (DEVICE_ID_LEN + 9)

static char *create_json_mqtt_message(void);
static void parse_json_mqtt_message(char *message_mqtt);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
//static esp_mqtt_client_handle_t mqtt_app_start(void);
static void mqtt_app_start(void);


static const char *TAG = "MQTT";


extern const uint8_t ca_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t ca_cert_pem_end[] asm("_binary_ca_cert_pem_end");
extern const uint8_t client_cert_pem_start[] asm("_binary_aws_client_cert_pem_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_aws_client_cert_pem_end");
extern const uint8_t client_key_pem_start[] asm("_binary_aws_client_key_pem_start");
extern const uint8_t client_key_pem_end[] asm("_binary_aws_client_key_pem_end");

typedef struct DeviceStatus {
    bool status;
    bool ping;
} DeviceStatus;

DeviceStatus device_status = { .status = false, .ping = true };

esp_mqtt_client_handle_t client;
/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    //ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;

    char device_id[DEVICE_ID_LEN];
    get_device_id(device_id);
    char topic_sub[TOPIC_SUB_LEN];
    snprintf(topic_sub, TOPIC_SUB_LEN, "%s%s", "esp32sub/", device_id);

    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, topic_sub, 0);
            //ESP_LOGI(TAG, "Subscription to topic %s successful, msg_id=%d", topic_sub, msg_id);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            {
                ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
                device_status.status = false;
                mqtt_publish_status(client);
            }
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            {
                //ESP_LOGI(TAG, "MQTT_EVENT_DATA");
                printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
                printf("DATA=%.*s\r\n", event->data_len, event->data);
                parse_json_mqtt_message(event->data);

                mqtt_publish_status(client);
            }
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                //ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
                //ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
                //ESP_LOGI(TAG, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                //         strerror(event->error_handle->esp_transport_sock_errno));
            } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
                //ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
            } else {
                //ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
            }
            break;

        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

static void mqtt_app_start(void)
{
  const esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = "mqtts://a38deg5j7utjz-ats.iot.ap-southeast-2.amazonaws.com:8883",
    .broker.verification.certificate = (const char *)ca_cert_pem_start,
    .credentials = {
      .authentication = {
        .certificate = (const char *)client_cert_pem_start,
        .key = (const char *)client_key_pem_start,
      },
    }
  };

    //ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void parse_json_mqtt_message(char *message_mqtt)
{
    cJSON *message_root = NULL;
    message_root = cJSON_Parse(message_mqtt);
    if(message_root == NULL){
        //ESP_LOGI(TAG, "MQTT sub parse fail");
        return;
    }

    char *status = cJSON_GetObjectItem(message_root, "status") ->valuestring;
    char *ping = cJSON_GetObjectItem(message_root, "ping") ->valuestring;
    // Update global device struct
    if (!strcmp(ping, "online")){
        device_status.ping = true;
    }
    if (!strcmp(ping, "offline")){
        device_status.ping = false;
    }
    if (!strcmp(status, "on")){
        device_status.status = true;
        //gpio_set_level(PIN_LED, true);
    }
    if (!strcmp(status, "off")){
        device_status.status = false;
        //gpio_set_level(PIN_LED, false);
    }
    cJSON_Delete(message_root);

    //ESP_LOGI(TAG, "MQTT Receive: status %d, ping %d", device_status.status, device_status.ping);
}

char *create_json_mqtt_message(void)
{
    char *json_str = NULL;

    cJSON *message_root = cJSON_CreateObject();
    cJSON *message_sub = cJSON_CreateObject();

    char device_id[DEVICE_ID_LEN];
    get_device_id(device_id);

    cJSON_AddStringToObject(message_root, "id", device_id);
    cJSON_AddStringToObject(message_root, "type", DEVICE_TYPE);
    cJSON_AddStringToObject(message_sub, "ping", "online");
    if (device_status.status){
        cJSON_AddStringToObject(message_sub, "status", "on");
    } else {
        cJSON_AddStringToObject(message_sub, "status", "off");
    }
    cJSON_AddItemToObject(message_root, "settings", message_sub);


    json_str = cJSON_Print(message_root);

    cJSON_Delete(message_root);

    ESP_LOGI(TAG, "Create JSON struct of size: %d, %s", strlen(json_str), json_str);

    return json_str;
}

void mqtt_publish_status(esp_mqtt_client_handle_t client)
{
    // Publish MQTT message on subscribed
    char *data = create_json_mqtt_message();
    //ESP_LOGI(TAG, "DataString: %d", strlen(data));

    char device_id[DEVICE_ID_LEN];
    get_device_id(device_id);
    char topic_pub[TOPIC_PUB_LEN];
    snprintf(topic_pub, TOPIC_PUB_LEN, "%s%s", "esp32pub/", device_id);

    int msg_id = esp_mqtt_client_publish(client, topic_pub, data, strlen(data), 0, 0);
    ESP_LOGI(TAG, "Published msg_id: %d to %s", msg_id, topic_pub);
}


void mqtt_task(void *arg)
{
    //ESP_LOGI(TAG, "[APP] Startup..");
    //ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    //ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    //esp_mqtt_client_handle_t client = mqtt_app_start();
    mqtt_app_start();


    while (true) {
        vTaskDelay(100000 / portTICK_PERIOD_MS);
        // bool led_status;
        // if(xQueueReceive(gpio_evt_queue, &led_status, portMAX_DELAY)) {
        //     device_status.status = led_status;
        //     mqtt_publish_status(client);
        //     // char *data = create_json_mqtt_message();
        //     // ESP_LOGI(TAG, "DataString: %d", strlen(data));

        //     // char device_id[DEVICE_ID_LEN];
        //     // get_device_id(device_id);
        //     // char topic_pub[TOPIC_PUB_LEN];
        //     // snprintf(topic_pub, TOPIC_PUB_LEN, "%s%s", "esp32pub/", device_id);

        //     // int msg_id = esp_mqtt_client_publish(client, topic_pub, data, strlen(data), 0, 0);
        //     // ESP_LOGI(TAG, "Published msg_id: %d to %s", msg_id, topic_pub);




        //     //gpio_set_level(PIN_LED, led_status);
        //     //ESP_LOGI(TAG, "LED status %d ", led_status);
        //     //led_status = !led_status;

        //     //vTaskDelay(1000 / portTICK_PERIOD_MS);
        // }
    }
}
