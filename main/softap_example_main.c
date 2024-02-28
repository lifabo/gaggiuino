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
#define TXD2_PIN (GPIO_NUM_3)
#define RXD2_PIN (GPIO_NUM_2)

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
}

int findHexSequence(const char *data, int dataSize, const uint8_t *hexSequence)
{
    int sequenceLength = strlen((const char *)hexSequence);

    for (int i = 0; i < dataSize - sequenceLength + 1; ++i)
    {
        int match = 1;
        for (int j = 0; j < sequenceLength; ++j)
        {
            if (data[i + j] != hexSequence[j])
            {
                match = 0;
                break;
            }
        }

        if (match)
        {
            return i; // Index des ersten Vorkommens der Hex-Folge (relativ zu den empfangenen Daten)
        }
    }

    return -1; // Hex-Folge nicht gefunden
}

static void checkCurrentTemp(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    // +1 für string null terminierung
    char *data = (char *)malloc(RX_BUF_SIZE + 1);
    const uint8_t dezPattern[] = {99, 117, 114, 114, 101, 110, 116, 84, 101, 109, 112, 61, '\0'}; // dezimale Darstellung von "currentTemp="

    while (1)
    {
        const int transmittedBytesCount = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS);

        if (transmittedBytesCount > 0)
        {
            int index = findHexSequence(data, transmittedBytesCount, dezPattern);

            if (index != -1)
            {
                // ESP_LOGI(RX_TASK_TAG, "Dez-Folge gefunden an Position %d in den Daten!\n", index);
                ESP_LOGI(RX_TASK_TAG, "Wert CurrentTemp: %c%c", (char)data[index + 12], (char)data[index + 13]);
                // ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, transmittedBytesCount, ESP_LOG_INFO);
            }
            else
            {
                // ESP_LOGI(RX_TASK_TAG, "Pattern nicht gefunden");
            }

            // nullterminierung
            data[transmittedBytesCount] = 0;
        }
    }

    free(data);
}
// --------------------------------------------------------------

void app_main(void)
{
    // init UART
    initUART();
    xTaskCreate(checkCurrentTemp, "uart_rx_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}