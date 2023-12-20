/*  WiFi softAP Example

 This example code is in the Public Domain (or CC0 licensed, at your option.)

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "string.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include <string.h>
#include "esp_http_server.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "esp_spiffs.h"

static const char *TAG = "gag";
static bool LED_STATE = false;
static const int RX_BUF_SIZE = 1024;

/* The examples use WiFi configuration that you can set via project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_WIFI_CHANNEL CONFIG_ESP_WIFI_CHANNEL
#define EXAMPLE_MAX_STA_CONN CONFIG_ESP_MAX_STA_CONN
#define LED_PIN 11

#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)
#define KNECSMOM (GPIO_NUM_3)
#define KNECSDAD (GPIO_NUM_2)

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
#ifdef CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT
            .authmode = WIFI_AUTH_WPA3_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
#else /* CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT */
            .authmode = WIFI_AUTH_WPA2_PSK,
#endif
            .pmf_cfg = {
                .required = true,
            },
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}

// UART
void initUART(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_set_pin(UART_NUM_0, KNECSMOM, KNECSDAD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

int sendData(const char *logName, const uint8_t *data, int UART_PORT)
{
    // const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_PORT, data, 8);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}
/*
static void writeUARTNUM1_DISPLAY(void *arg)
{
    static const char *TX_TASK_TAG = "TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    while (1)
    {
        sendData(TX_TASK_TAG, "Hello world", UART_NUM_1);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static void writeUARTNUM0_STM(void *arg)
{
    static const char *TX_TASK_TAG = "TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    while (1)
    {
        sendData(TX_TASK_TAG, "Hello world", UART_NUM_0);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}*/

static void readUARTNUM1_DISPLAY(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t *data = (uint8_t *)malloc(RX_BUF_SIZE + 1);
    while (1)
    {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS);
        if (rxBytes > 0)
        {
            data[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);

            sendData("TX_TSAK_TAG", *data, UART_NUM_0);
        }
    }
    free(data);
}
static void readUARTNUM0_STM(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t *data = (uint8_t *)malloc(RX_BUF_SIZE + 1);
    while (1)
    {
        const int rxBytes = uart_read_bytes(UART_NUM_0, data, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS);
        if (rxBytes > 0)
        {
            data[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);

            sendData("TX_TSAK_TAG", data, UART_NUM_1);
        }
    }
    free(data);
}
// --------------------------------------------------------------

// LED
static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(LED_PIN);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
}

static void blink_led(void)
{
    gpio_set_level(LED_PIN, LED_STATE);
}
// --------------------------------------------------------------

// WEBSERVER
esp_err_t hello_get_handler(httpd_req_t *req)
{
    LED_STATE = !LED_STATE;
    blink_led();

    char *response;

    // FILE CHECK
    FILE *f = fopen("/storage/index.html", "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }

    char line[64];
    size_t response_size = 0;
    response = malloc(1); // Initialisierung des response-Zeigers

    while (fgets(line, sizeof(line), f) != NULL)
    {
        // printf("%s", line);
        size_t line_len = strlen(line);
        response = realloc(response, response_size + line_len + 1);
        if (response == NULL)
        {
            // Fehler bei der Speicherzuweisung
            ESP_LOGE(TAG, "Failed to allocate memory");
            fclose(f);
            return ESP_FAIL;
        }

        // Zeichenkette anhängen
        strcpy(response + response_size, line);
        response_size += line_len;
    }

    fclose(f);

    httpd_resp_send(req, response, strlen(response));

    // Vergessen Sie nicht, den Speicher freizugeben, wenn Sie fertig sind
    // free(response);
    return ESP_OK;
}

void start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_uri_t hello = {
            .uri = "/hello",
            .method = HTTP_GET,
            .handler = hello_get_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(server, &hello);
    }
}
// --------------------------------------------------------------

void app_main(void)
{

    // init UART
    initUART();
    // xTaskCreate(tx_task, "uart_tx_task", 1024 * 2, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(readUARTNUM1_DISPLAY, "uart_rx_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(readUARTNUM0_STM, "uart_rx_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);

    // init led
    configure_led();

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();

    start_webserver();

    // spiffs
    esp_vfs_spiffs_conf_t config = {
        .base_path = "/storage",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};

    esp_err_t result = esp_vfs_spiffs_register(&config);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to init SPIFFS.");
        return;
    }

    size_t total = 0, used = 0;
    result = esp_spiffs_info(config.partition_label, &total, &used);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get partition info.");
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}
