#include "wifi_manager.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "sdkconfig.h"
#include <string.h>

static const char *TAG = "wifi_manager";

static esp_err_t nvs_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

#ifdef CONFIG_FLIPDOT_WIFI_STA_MODE

#define MAX_RETRY 5
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static EventGroupHandle_t s_wifi_event_group;
static int s_retry = 0;

static void sta_event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry < MAX_RETRY) {
            esp_wifi_connect();
            s_retry++;
            ESP_LOGW(TAG, "Retrying WiFi connection (%d/%d)", s_retry, MAX_RETRY);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *e = (ip_event_got_ip_t *)data;
        ESP_LOGI(TAG, "Connected — IP: " IPSTR " — browse to http://" IPSTR,
                 IP2STR(&e->ip_info.ip), IP2STR(&e->ip_info.ip));
        s_retry = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

#endif // CONFIG_FLIPDOT_WIFI_STA_MODE

esp_err_t wifi_manager_init(void)
{
    ESP_ERROR_CHECK(nvs_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

#ifdef CONFIG_FLIPDOT_WIFI_AP_MODE

    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = strlen(CONFIG_FLIPDOT_WIFI_SSID),
            .max_connection = 4,
            .authmode = strlen(CONFIG_FLIPDOT_WIFI_PASSWORD) >= 8
                        ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN,
        },
    };
    strlcpy((char *)wifi_config.ap.ssid, CONFIG_FLIPDOT_WIFI_SSID, sizeof(wifi_config.ap.ssid));
    strlcpy((char *)wifi_config.ap.password, CONFIG_FLIPDOT_WIFI_PASSWORD, sizeof(wifi_config.ap.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "AP mode — SSID: \"%s\" — connect and browse to http://192.168.4.1",
             CONFIG_FLIPDOT_WIFI_SSID);

#elif defined(CONFIG_FLIPDOT_WIFI_STA_MODE)

    s_wifi_event_group = xEventGroupCreate();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &sta_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &sta_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {.sta = {}};
    strlcpy((char *)wifi_config.sta.ssid, CONFIG_FLIPDOT_WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strlcpy((char *)wifi_config.sta.password, CONFIG_FLIPDOT_WIFI_PASSWORD, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                            pdFALSE, pdFALSE, portMAX_DELAY);
    if (!(bits & WIFI_CONNECTED_BIT)) {
        ESP_LOGE(TAG, "Failed to connect to SSID: \"%s\"", CONFIG_FLIPDOT_WIFI_SSID);
        return ESP_FAIL;
    }

#endif

    return ESP_OK;
}
