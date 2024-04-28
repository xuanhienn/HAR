/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "cJSON.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "MPU6050.h"

static const char *TAG = "MQTT_EXAMPLE";
const TickType_t xDelay = 50 / portTICK_PERIOD_MS;
static esp_adc_cal_characteristics_t adc1_chars;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

float data[] = {0.61204249,0.43907278,0.71082642,0.50046237,0.49654502,0.571198,
0.61639821,0.43601846,0.71252042,0.5005689,0.49640485,0.57090007,
0.6181526,0.43931233,0.71118941,0.50042686,0.4972178,0.57110862,
0.61561176,0.43871344,0.71354889,0.49939711,0.49783451,0.57140655,
0.61276845,0.43368281,0.71342789,0.4995569,0.4983391,0.57104903,
0.61089306,0.43625802,0.7121574,0.49961016,0.49766632,0.57092986,
0.6104091,0.43518002,0.71264141,0.49984097,0.4972178,0.57027442,
0.61282894,0.43500036,0.71264141,0.5001783,0.49702157,0.57021483,
0.61307093,0.43751567,0.71252042,0.50010729,0.49755419,0.57039359,
0.61125604,0.43541958,0.7132464,0.49968118,0.49763829,0.57092986,
0.61071158,0.43559924,0.71112891,0.49962797,0.49721778,0.57072139,
0.6134944,0.43709645,0.7138514,0.49964573,0.49721778,0.5706618,
0.61186101,0.43709645,0.71288341,0.49920187,0.49777843,0.57128745,
0.61143753,0.43589868,0.71463791,0.49870475,0.4979186,0.57140662,
0.61071158,0.43440147,0.71082642,0.4981011,0.49763827,0.57140662,
0.61089306,0.42925106,0.71064491,0.49936166,0.49738598,0.57060222,
0.6134339,0.43206582,0.71003991,0.50003633,0.49713369,0.57033408,
0.61174001,0.43218559,0.71179441,0.49966348,0.49741401,0.57090014,
0.6087757,0.43254492,0.71112891,0.49975225,0.49749811,0.57063201,
0.60925967,0.43050872,0.7112499,0.50044468,0.49693746,0.57006594,
0.61288944,0.43404214,0.7141539,0.50053345,0.496545,0.57003615,
0.61415985,0.43242515,0.7124599,0.50003625,0.49718977,0.57051273,
0.61288944,0.43500036,0.71191542,0.49989421,0.49727387,0.57090004,
0.61258696,0.43074827,0.71173391,0.49966341,0.4974701,0.57081066,
0.6119215,0.43416192,0.71082642,0.49984095,0.49766633,0.57110859,
0.61125604,0.43272459,0.71137089,0.50007176,0.4972178,0.57078087,
0.61222398,0.43314381,0.71167342,0.4998232,0.4974701,0.57098942,
0.61125604,0.43140704,0.71058441,0.49976993,0.49775042,0.57104901,
0.61046959,0.43086805,0.71034241,0.49998299,0.4973019,0.57072128,
0.6119215,0.43218559,0.71106842,0.500054,0.49707764,0.57057232,
0.61174001,0.43332348,0.7107659,0.50003625,0.49713371,0.57060211,
0.61337341,0.43140704,0.71173391,0.4998232,0.4974701,0.5706617,
0.61131653,0.43224548,0.7104029,0.50005402,0.49766632,0.5707809,
0.61077207,0.43200593,0.7127624,0.49991199,0.49732993,0.57063193,
0.6103486,0.43458114,0.71233891,0.49962792,0.49749812,0.57104903,
0.61077207,0.43050872,0.71252042,0.49950364,0.49755419,0.57104903,
0.60938066,0.43242515,0.71064491,0.49982321,0.49752616,0.57098945,
0.61161902,0.4321257,0.71233891,0.50000076,0.49710567,0.57081069,
0.61167952,0.43176637,0.71203641,0.49992974,0.4972178,0.57122779,
0.61155853,0.43128727,0.71118941,0.49996525,0.49780648,0.57098945,
0.61016711,0.43002961,0.71149191,0.49992974,0.49744206,0.57084048,
0.61004611,0.43248504,0.7123994,0.50032034,0.49724583,0.57063193,
0.61204249,0.43092793,0.71264141,0.50001851,0.49707764,0.57066173,
0.61053008,0.43248504,0.7120969,0.50005414,0.4973299,0.57078105,
0.61119554,0.43284437,0.71149191,0.50014291,0.49738596,0.57069167,
0.61077207,0.43140704,0.71070543,0.49994761,0.49735793,0.57090022,
0.61053008,0.43278448,0.71252042,0.50019617,0.49718974,0.57075125,
0.61107455,0.43254492,0.71227841,0.4999121,0.49735793,0.57069167,
0.6102881,0.43242515,0.71179441,0.4997168,0.49704957,0.57119815,
0.61046959,0.43308393,0.71366989,0.50003638,0.4971617,0.57104918,
0.61204249,0.43086805,0.71197589,0.49978782,0.49738596,0.57066188,
0.61053008,0.43278448,0.71112891,0.49989435,0.49766629,0.57078105,
0.61137703,0.42979005,0.71227841,0.50000087,0.49735793,0.57024477,
0.60986463,0.43272459,0.71179441,0.49982333,0.49730187,0.57072146,
0.6119215,0.43062849,0.71366989,0.49975218,0.49758223,0.5706319,
0.61113504,0.42996972,0.71106842,0.49959239,0.4975542,0.57113838,
0.61089306,0.43080816,0.71197589,0.4998232,0.49749813,0.57101921,
0.6088967,0.42877195,0.71197589,0.49984095,0.49732993,0.57101921,
0.60895719,0.43152682,0.71221789,0.50023155,0.49707764,0.5706617,
0.61101406,0.43038894,0.7124599,0.49989421,0.49744206,0.57072128};

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
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    /*int k = 0;
    char string[20];*/
    int msg_id;
    
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        //ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        
        msg_id = esp_mqtt_client_subscribe(client, "Test", 1);
        //ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        // ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
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

void MQTT_sendJSON(esp_mqtt_client_handle_t client, const char *topic, cJSON *json_data) 
{
    int msg_id;
    char *json_str = cJSON_Print(json_data);
    msg_id = esp_mqtt_client_publish(client, topic, json_str, strlen(json_str), 1, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
    cJSON_free(json_str);
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://mqtt.eclipseprojects.io",    // "mqtt://username:password@broker.hivemq.com"
        //.broker.address.uri = "mqtts://admin:P683LO1W3Ucb47UTWlPL1RGOTvsoFkV5@45kg21.stackhero-network.com:1884",
   

    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    MPU_Init();
int16_t ax, ay, az, gx, gy, gz;
    int msg_id;
    int time = 0;
    char string[5];
    char JSON2[16] = ", \"ay\": ";
    char JSON3[14] = ", \"az\": ";
    char JSON4[16] = ", \"gx\": ";
    char JSON5[14] = ", \"gy\": ";
    char JSON6[16] = ", \"gz\": ";
    char JSON7[14] = ", \"hb\": ";
    char JSON8[12] = ", \"Time\": ";
    char JSON9[5] = " }";
    char JSON10[16] = ", \"ay\": ";
    while(1)
        {
            char JSON1[200] = "{ \"ax\": ";
            //MPU
            // max30102_update(&max30102, &result);
            // printf("BPM: %f | SpO2: %f%%\n", result.heart_bpm, result.spO2);
            MPU_Get_Accelerometer(&ax, &ay, &az);
            MPU_Get_Gyroscope(&gx, &gy, &gz);
            //printf("Accel: x=%d, y=%d, z=%d\n", ax, ay, az);

            strcat(JSON1, gcvt(ax/16384.0,9,string));
            strcat(JSON1, JSON10);
            strcat(JSON1, gcvt(ay/16384.0,9,string));
            strcat(JSON1, JSON3);
            strcat(JSON1, gcvt(az/16384.0,9,string));
            strcat(JSON1, JSON4);
            strcat(JSON1, gcvt(gx/65.5,9,string));
            strcat(JSON1, JSON5);
            strcat(JSON1, gcvt(gy/65.5,9,string));
            strcat(JSON1, JSON6);
            strcat(JSON1, gcvt(gz/65.5,9,string));
            // strcat(JSON1, JSON7);
            // strcat(JSON1, gcvt(result.heart_bpm,9,string));
            strcat(JSON1, JSON8);
            strcat(JSON1, itoa(time,string,10));
            strcat(JSON1, JSON9);
            time += 1;
            msg_id = esp_mqtt_client_publish(client, "Test", JSON1, strlen(JSON1), 1, 0);
            //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            //printf("Accel: %s", JSON1);
            vTaskDelay(xDelay);
        }
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());
    // gpio_config_t GPIO_Config = {};
    // GPIO_Config.pin_bit_mask = (1 << 18);          /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
    // GPIO_Config.mode = GPIO_MODE_OUTPUT;               /*!< GPIO mode: set input/output mode                     */
    // GPIO_Config.pull_up_en = GPIO_PULLUP_DISABLE;       /*!< GPIO pull-up                                         */
    // GPIO_Config.pull_down_en = GPIO_PULLDOWN_DISABLE;   /*!< GPIO pull-down                                       */
    // GPIO_Config.intr_type = GPIO_INTR_DISABLE;
    // gpio_config(&GPIO_Config);
    // while(1)
    // {
    //     uint8_t data = gpio_get_level(18);
    //     printf("x = %d \n", data);
    //     vTaskDelay(xDelay);
    // }
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);

    adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);

    mqtt_app_start();
}
