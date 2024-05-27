#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_netif.h"

#include "lwip/lwip_napt.h"
#include "lwip/sockets.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "WIFI.hpp"
#include "BASES.hpp"

#include "lwip/ip4_addr.h"

#define EXAMPLE_ESP_MAXIMUM_RETRY      5

static const char *TAG_AP = "WiFi SoftAP";
static const char *TAG_STA = "WiFi Sta";

static uint8_t ap_connect = 0;
bool initWIFI = false;

void *m_sta_netif = NULL;
void *m_ap_netif = NULL;

esp_event_handler_instance_t instance_any_id;
esp_event_handler_instance_t instance_got_ip;

config_wifi_t *m_config;

BASE reconnect_timer;

uint32_t my_ap_ip;
uint8_t m_mac[6];
char ip_info[16];

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

esp_netif_t *wifi_init_softap(void);
esp_netif_t *wifi_init_sta(void);

/**
 * @brief boot with settings
 * 
 * @param config WIFI configuration structure
 * @return bool error return
 */
bool WIFI_Start(config_wifi_t *config)
{
    if(config->mode >= WIFI_MODE_MAX) return false;

    m_config = config;

    if(!initWIFI)
    {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());

        //Initialize NVS
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);


        /*Initialize WiFi */
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        ESP_ERROR_CHECK(esp_wifi_set_mode((wifi_mode_t)config->mode));

        /* Register Event handler */
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
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                        ESP_EVENT_ANY_ID,
                        &wifi_event_handler,
                        NULL,
                        NULL));

        if(config->mode == WIFI_MODE_STA || config->mode == WIFI_MODE_APSTA)
        {
            /* Initialize STA */
            ESP_LOGI(TAG_STA, "ESP_WIFI_MODE_STA");
            m_sta_netif = wifi_init_sta();
        }

        if(config->mode == WIFI_MODE_AP || config->mode == WIFI_MODE_APSTA)
        {
            ESP_LOGI(TAG_STA, "ESP_WIFI_MODE_AP");
            m_ap_netif = wifi_init_softap();
        }

        /* Start WiFi */
        ESP_ERROR_CHECK(esp_wifi_start());
 
    }else{
        WIFI_Update(config);
    }

    initWIFI = true;

    return true;
}

/**
 * @brief update wifi settings
 * 
 * @param config WIFI configuration structure
 * @return bool error return
 */
bool WIFI_Update(config_wifi_t *config)
{
    if(config->mode != WIFI_MODE_NULL) m_config->mode           = config->mode          ;
    if(strlen(config->ssid_sta)     != 0 ) m_config->ssid_sta       = config->ssid_sta      ;
    if(strlen(config->pass_sta)     != 0 ) m_config->pass_sta       = config->pass_sta      ;
    if(strlen(config->ssid_ap)      != 0 ) m_config->ssid_ap        = config->ssid_ap       ;
    if(strlen(config->pass_ap)      != 0 ) m_config->pass_ap        = config->pass_ap       ;
    if(config->channel              != 0 ) m_config->channel        = config->channel       ;
    if(config->max_connection       != 0 ) m_config->max_connection = config->max_connection;

    if(m_config->mode == WIFI_MODE_STA || m_config->mode == WIFI_MODE_APSTA)
    {
        /* Initialize STA */
        ESP_LOGI(TAG_STA, "ESP_WIFI_MODE_STA");

        wifi_config_t wifi_sta_config = {};
        memcpy(wifi_sta_config.sta.ssid, m_config->ssid_sta, strlen(m_config->ssid_sta)+1);
        memcpy(wifi_sta_config.sta.password, m_config->pass_sta, strlen(m_config->pass_sta)+1);
        wifi_sta_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
        wifi_sta_config.sta.failure_retry_cnt = EXAMPLE_ESP_MAXIMUM_RETRY;

        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));
    }

    if(m_config->mode == WIFI_MODE_AP || m_config->mode == WIFI_MODE_APSTA)
    {
        /* Initialize STA */
        ESP_LOGI(TAG_STA, "ESP_WIFI_MODE_SoftAP");

        wifi_config_t wifi_ap_config = {};
        memcpy(wifi_ap_config.ap.ssid, m_config->ssid_ap, strlen(m_config->ssid_ap)+1);
        memcpy(wifi_ap_config.ap.password, m_config->pass_ap, strlen(m_config->pass_ap)+1);
        wifi_ap_config.ap.channel = m_config->channel;
        wifi_ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
        wifi_ap_config.ap.max_connection = m_config->max_connection;
        wifi_ap_config.ap.pmf_cfg.required = false;

        if (strlen(m_config->ssid_ap) == 0) {
            wifi_ap_config.ap.authmode = WIFI_AUTH_OPEN;
        }

        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));
    }

    ap_connect = 0;

    return true;
}

/**
 * @brief change wifi mode
 * 
 * @param mode WIFI mode type
 * @return bool error return
 */
bool WIFI_MODE(WIFI_mode_t mode)
{
    if(!initWIFI) return false;
    if(mode == m_config->mode) return true;
    if(mode >= WIFI_MODE_MAX) return false;

    ESP_ERROR_CHECK(esp_wifi_set_mode((wifi_mode_t)mode));
    if((mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA) && m_config->mode != WIFI_MODE_STA && m_config->mode != WIFI_MODE_APSTA)
    {
        /* Initialize STA */
        ESP_LOGI(TAG_STA, "ESP_WIFI_MODE_STA");
        m_sta_netif = wifi_init_sta();
    }
    if((mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA) && m_config->mode != WIFI_MODE_AP && m_config->mode != WIFI_MODE_APSTA)
    {
        /* Initialize STA */
        ESP_LOGI(TAG_STA, "ESP_WIFI_MODE_SoftAP");
        m_ap_netif = wifi_init_softap();
    }
    m_config->mode = mode;
    return true;
}

/**
 * @brief Return to Netif
 * 
 * @param mode WIFI mode type
 * @return void* netif pointer
 */
void * WIFI_Netif(WIFI_mode_t mode)
{
    if(!initWIFI) return NULL;
    if(mode != WIFI_MODE_STA && mode != WIFI_MODE_AP) return NULL;
    if(mode == WIFI_MODE_STA) return m_sta_netif;
    return m_ap_netif;
}

/**
 * @brief connect or disconnect from the network
 * 
 * @param enable connect or disconnect
 * @return bool error return
 */
bool WIFI_connect(bool enable)
{
    if(!initWIFI) return false;
    if(m_config->mode != WIFI_MODE_STA && m_config->mode != WIFI_MODE_APSTA) return false;

    wifi_ap_record_t ap_info;
    esp_wifi_sta_get_ap_info(&ap_info);

    // desconectado e ligar
    if (!ap_info.ssid[0] && enable) 
    {
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
        esp_wifi_connect();

    //conectado e desligar
    } else if(ap_info.ssid[0] && !enable){
        esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id);
        esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip);
        ESP_ERROR_CHECK(esp_wifi_disconnect());
    }

    return true;
}

/**
 * @brief Checking the connection
 * 
 * @return 
 *  0 - Trying to connect
 *  1 - Connection failed
 *  2 - Connected
 */
uint8_t WIFI_connected()
{
    return ap_connect;
}


/**
 * @brief IPV4 address return
 * 
 * @param ip IPV4 address
 * @param netif netif
 */
void WIFI_get_IP(char * ip, void * netif)
{
    esp_netif_ip_info_t m_ip;

    if(esp_netif_get_ip_info((esp_netif_t *)netif, &m_ip) != ESP_OK){
        printf("Falha ao obter o endereço IP do STA\n");
    }

    esp_ip4addr_ntoa(&m_ip.ip, ip_info, 16);

    strcpy(ip, ip_info);
}

void WIFI_SoftAP_set_IP(esp_netif_t * netif)
{
    esp_netif_ip_info_t ipInfo_ap;
    esp_read_mac(m_mac, ESP_MAC_WIFI_SOFTAP);
    esp_netif_set_ip4_addr(&ipInfo_ap.ip, m_mac[3],m_mac[4],m_mac[5],1);
    esp_netif_set_ip4_addr(&ipInfo_ap.gw, m_mac[3],m_mac[4],m_mac[5],1);
    esp_netif_set_ip4_addr(&ipInfo_ap.netmask, 255,255,255,0);
    esp_netif_dhcps_stop(netif); // stop before setting ip WifiAP
    esp_netif_set_ip_info(netif, &ipInfo_ap);
    esp_netif_dhcps_start(netif);
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        esp_wifi_connect();
        ESP_LOGI(TAG_STA, "Station started");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG_STA,"disconnected - retry to connect to the AP");
        ap_connect = 1;
        esp_wifi_connect();
        ESP_LOGI(TAG_STA, "retry to connect to the AP");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG_STA, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        ap_connect = 2;
    }
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) 
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *) event_data;
        ESP_LOGI(TAG_AP, "Station " MACSTR " joined, AID=%d", MAC2STR(event->mac), event->aid);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) 
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *) event_data;
        ESP_LOGI(TAG_AP, "Station " MACSTR " left, AID=%d", MAC2STR(event->mac), event->aid);
    }
}

/* Initialize soft AP */
esp_netif_t *wifi_init_softap(void)
{
    esp_netif_t *esp_netif_ap = esp_netif_create_default_wifi_ap();

    WIFI_SoftAP_set_IP(esp_netif_ap);

    wifi_config_t wifi_ap_config = {};
    memcpy(wifi_ap_config.ap.ssid, m_config->ssid_ap, strlen(m_config->ssid_ap)+1);
    memcpy(wifi_ap_config.ap.password, m_config->pass_ap, strlen(m_config->pass_ap)+1);
    wifi_ap_config.ap.channel = m_config->channel;
    wifi_ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_ap_config.ap.max_connection = m_config->max_connection;
    wifi_ap_config.ap.pmf_cfg.required = false;

    if (strlen(m_config->ssid_ap) == 0) {
        wifi_ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));

    ESP_LOGI(TAG_AP, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             m_config->ssid_ap, m_config->pass_ap, m_config->channel);

    /* Enable napt on the AP netif */
    if (esp_netif_napt_enable(esp_netif_ap) != ESP_OK) {
        ESP_LOGE(TAG_AP, "NAPT not enabled on the netif: %p", esp_netif_ap);
    }

    return esp_netif_ap;
}

/* Initialize wifi station */
esp_netif_t *wifi_init_sta(void)
{
    esp_netif_t *esp_netif_sta = esp_netif_create_default_wifi_sta();

    wifi_config_t wifi_sta_config = {};
    memcpy(wifi_sta_config.sta.ssid, m_config->ssid_sta, strlen(m_config->ssid_sta)+1);
    memcpy(wifi_sta_config.sta.password, m_config->pass_sta, strlen(m_config->pass_sta)+1);
    wifi_sta_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    wifi_sta_config.sta.failure_retry_cnt = EXAMPLE_ESP_MAXIMUM_RETRY;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));

    ESP_LOGI(TAG_STA, "wifi_init_sta finished.");

    /* Set sta as the default interface */
    esp_netif_set_default_netif(esp_netif_sta);

    return esp_netif_sta;
}