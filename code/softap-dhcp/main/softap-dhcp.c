#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/dns.h"
#include "esp_netif.h"

#define EXAMPLE_ESP_WIFI_SSID "ESP32_AP"
#define EXAMPLE_ESP_WIFI_PASS ""
#define EXAMPLE_ESP_WIFI_CHANNEL 1
#define EXAMPLE_MAX_STA_CONN 10

#define RASPBERRY_PI_MAC_ADDR {0xd8, 0x3a, 0xdd, 0xb2, 0x64, 0x2a}
#define RASPBERRY_PI_IP_ADDR "192.168.4.100"

static const char *TAG = "wifi softAP";

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_OPEN
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);

    uint8_t raspberry_pi_mac[6] = RASPBERRY_PI_MAC_ADDR;
    ip4_addr_t raspberry_pi_ip;
    IP4_ADDR(&raspberry_pi_ip, 192, 168, 4, 100);

    //dhcps_register_mac_static_ip(raspberry_pi_mac, &raspberry_pi_ip);
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();

    // Set up the DNS server
    ip_addr_t dns_server;
    dns_server.type = IPADDR_TYPE_V4;
    dns_server.u_addr.ip4.addr = esp_ip4addr_aton("192.168.4.1");
    dns_setserver(0, &dns_server);

    // Add a DNS record for "xprize.iit"
    const char* hostname = "xprize";
    const char* instance = "iit";
    ip_addr_t raspberry_pi_ip;
    raspberry_pi_ip.type = IPADDR_TYPE_V4;
    raspberry_pi_ip.u_addr.ip4.addr = esp_ip4addr_aton(RASPBERRY_PI_IP_ADDR);

    // Create the full domain name with the instance name
    size_t full_name_len = strlen(hostname) + strlen(instance) + 2;
    char* full_name = (char*)malloc(full_name_len);
    snprintf(full_name, full_name_len, "%s.%s", hostname, instance);

    dns_gethostbyname(full_name, &raspberry_pi_ip, NULL, NULL);
    free(full_name);

    // Start the DNS server
    dns_init();
}