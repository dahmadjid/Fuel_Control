/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "esp_netif.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      "Wameedh_Wifi"
#define EXAMPLE_ESP_WIFI_PASS      "WameedhC001A"
#define EXAMPLE_ESP_MAXIMUM_RETRY  3
#define SOCK_TAG "SOCKET"
#define SERVER_IP "172.20.11.243"
#define PORT 6205
/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi station";

static int s_retry_num = 0;
int sock = -1;

sockaddr_in dest_addr = {
    0,
    AF_INET,
    htons(PORT),
    {inet_addr(SERVER_IP)}
    };

// struct sockaddr_in {
//   u8_t            sin_len;
//   sa_family_t     sin_family;
//   in_port_t       sin_port;
//   struct in_addr  sin_addr;
// #define SIN_ZERO_LEN 8
//   char            sin_zero[SIN_ZERO_LEN];
// };





static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    
    uint8_t ssid[32];      /**< SSID of target AP. Null terminated string. */
    uint8_t password[64];  /**< Password of target AP. Null terminated string.*/
    wifi_scan_method_t scan_method;    /**< do all channel scan or fast scan */
    bool bssid_set;        /**< whether set MAC address of target AP or not. Generally, station_config.bssid_set needs to be 0; and it needs to be 1 only when users need to check the MAC address of the AP.*/
    uint8_t bssid[6];     /**< MAC address of target AP*/
    uint8_t channel;       /**< channel of target AP. Set to 1~13 to scan starting from the specified channel before connecting to AP. If the channel of AP is unknown, set it to 0.*/
    uint16_t listen_interval;   /**< Listen interval for ESP32 station to receive beacon when WIFI_PS_MAX_MODEM is set. Units: AP beacon intervals. Defaults to 3 if set to 0. */
    wifi_sort_method_t sort_method;    /**< sort the connect AP in the list by rssi or security mode */
    wifi_scan_threshold_t  threshold;     /**< When sort_method is set, only APs which have an auth mode that is more secure than the selected auth mode and a signal stronger than the minimum RSSI will be used. */
    wifi_pmf_config_t pmf_cfg;    /**< Configuration for Protected Management Frame. Will be advertized in RSN Capabilities in RSN IE. */
    

    wifi_config_t wifi_config = { .sta =  {
        /* ssid            */ EXAMPLE_ESP_WIFI_SSID,
        /* password        */ EXAMPLE_ESP_WIFI_PASS,
        /* scan_method     */ {},
        /* bssid_set       */ {},
        /* bssid           */ {},
        /* channel         */ {},
        /* listen_interval */ {},
        /* sort_method     */ {},
        /* threshold       */ {
            /* rssi            */ {},
            /* authmode        */ WIFI_AUTH_WPA2_PSK},
        /* pmf_cfg         */ {
            /* capable         */ true,
            /* required        */ false},
    }};
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGW(TAG, "connected to ap SSID: %s || password: %s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed to connect to SSID: %s || password: %s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}
int createSocket(int* s)
{           

    *s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (*s < 0) 
    {
        ESP_LOGE(SOCK_TAG, "Unable to create socket: errno %s", strerror(errno));
        return errno;    
    }

    ESP_LOGI(SOCK_TAG, "Successfully created socket %d", *s);
    return 0;
}

int connectToServer(const sockaddr_in& dest)
{   
    int err = connect(sock, (sockaddr*)&dest, sizeof(dest));
    if (err != 0) 
    {
        ESP_LOGE(SOCK_TAG, "Socket unable to connect: errno %s", strerror(errno));
        return errno;
    }
    ESP_LOGI(SOCK_TAG, "Successfully connected");
    return 0;
}


int sendTag(const Uid& tag, char* buf)
{
    
    int err;
    err = createSocket(&sock);
    if (err != 0) 
        return err;
    err = connectToServer(dest_addr);
    if (err != 0) 
        return err;
    err = send(sock, &tag.uidByte, tag.size, 0);
    if (err < 0) 
    {
        ESP_LOGE(SOCK_TAG, "Error occurred during sending tag: errno %s", strerror(errno));
        if (errno == ENOTCONN || errno == ECONNRESET )
        {
            closesocket(sock);
            err = createSocket(&sock);
            if (err != 0) 
                return err;
            err = connectToServer(dest_addr);
            if (err != 0) 
                return err;
            err = send(sock, &tag.uidByte, tag.size, 0);
            if (err < 0) 
            {
                ESP_LOGE(SOCK_TAG, "Error occurred during sending tag: errno %s", strerror(errno));
                return errno;
            }
            else
            {
                ESP_LOGI(SOCK_TAG, "Successfully sent tag");
                return 0;
            }
        }
        return errno;
    }

    ESP_LOGI(SOCK_TAG, "Successfully sent tag, waiting for response");
    memset(buf, 0, 64);
    int len = recv(sock, buf, 64, 0);

    if (len < 0)
    {
        ESP_LOGE(SOCK_TAG, "Error occured while recieving data errno: %s", strerror(errno));
        return errno;
    }
    
    ESP_LOGI(SOCK_TAG, "response from server: %s", buf);
    closesocket(sock);
    return 0;
    
}



void wifiInit(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
}
