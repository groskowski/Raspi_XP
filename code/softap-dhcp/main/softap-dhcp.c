#include <stdio.h>
#include <string.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_http_server.h>
#include <esp_log.h>

#include "esp_mac.h"

#include "nvs_flash.h"
#include "esp_netif.h"

#define EXAMPLE_ESP_WIFI_SSID "ESP32_AP"
#define EXAMPLE_ESP_WIFI_PASS ""
#define EXAMPLE_MAX_STA_CONN 10
#define REDIRECT_HOST "192.168.4.100" // IP address of the Raspberry Pi

#define STATIC_IP_DEVICE_MAC "d8:3a:dd:b2:64:2a" // Replace with the MAC address of the device
#define STATIC_IP_ADDRESS "192.168.4.100" // Replace with the desired static IP address

static const char *TAG = "softap_dhcp";

static esp_err_t redirect_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Received request: %s", req->uri);

    if (strstr(req->uri, "xprize.iit") != NULL) {
        // Redirect to the Raspberry Pi's IP address
        ESP_LOGI(TAG, "Redirecting to: http://%s", REDIRECT_HOST);
        httpd_resp_set_status(req, "302 Found");
        httpd_resp_set_hdr(req, "Location", "http://" REDIRECT_HOST);
        httpd_resp_send(req, NULL, 0);
    } else {
        // Handle other requests as needed
        ESP_LOGI(TAG, "Sending 404 Not Found");
        httpd_resp_send_404(req);
    }
    return ESP_OK;
}

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG, "Starting web server");
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t redirect_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = redirect_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &redirect_uri);
    }

    return server;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "Station " MACSTR " connected, AID=%d", MAC2STR(event->mac), event->aid);

        // Check if the connected device has the specific MAC address
        if (memcmp(event->mac, STATIC_IP_DEVICE_MAC, 6) == 0) {
            ESP_LOGI(TAG, "Device with MAC address " MACSTR " connected", MAC2STR(event->mac));

            // Assign the static IP address to the device
            esp_netif_ip_info_t ip_info;
            memset(&ip_info, 0, sizeof(esp_netif_ip_info_t));
            ip_info.ip.addr = esp_ip4addr_aton(STATIC_IP_ADDRESS);
            ip_info.netmask.addr = esp_ip4addr_aton("255.255.255.0");
            ip_info.gw.addr = esp_ip4addr_aton("192.168.1.1");

            esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
            if (netif != NULL) {
                esp_err_t err = esp_netif_dhcps_stop(netif);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to stop DHCP server: %s", esp_err_to_name(err));
                }
                err = esp_netif_set_ip_info(netif, &ip_info);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to set IP info: %s", esp_err_to_name(err));
                }
                err = esp_netif_dhcps_start(netif);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to start DHCP server: %s", esp_err_to_name(err));
                }
                ESP_LOGI(TAG, "Static IP address assigned to " MACSTR, MAC2STR(event->mac));
            } else {
                ESP_LOGE(TAG, "Failed to get netif handle");
            }
        } else {
            ESP_LOGI(TAG, "Device with MAC address " MACSTR " does not match the specified MAC address", MAC2STR(event->mac));
        }
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "Station " MACSTR " disconnected, AID=%d", MAC2STR(event->mac), event->aid);
    }
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

    // Initialize Wi-Fi
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_OPEN
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "ESP32 SoftAP started with SSID: %s", EXAMPLE_ESP_WIFI_SSID);

    // Start the HTTP server
    start_webserver();
}