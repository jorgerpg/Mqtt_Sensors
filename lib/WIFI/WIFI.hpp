#ifndef WIFI_HPP
#define WIFI_HPP

#define wifi_reconnect_timeout 5000

/** @attention Inlcudes must be changed according to the platform used **/
#include <stdint.h>
#include <stdbool.h>

extern void *m_sta_netif;
extern void *m_ap_netif;

typedef enum{

  WIFI_MODE_NULL = 0,  /**< null mode */
  WIFI_MODE_STA,       /**< WiFi station mode */
  WIFI_MODE_AP,        /**< WiFi soft-AP mode */
  WIFI_MODE_APSTA,     /**< WiFi station + soft-AP mode */
  WIFI_MODE_MAX
  
}WIFI_mode_t;

typedef struct 
{
  WIFI_mode_t mode;
  char* ssid_sta;
  char* pass_sta;
  char* ssid_ap;
  char* pass_ap;
  uint8_t channel;
  uint8_t max_connection;
}config_wifi_t;


  /* Function Prototypes */

bool WIFI_Start(config_wifi_t *config); // inicializar com as configurações
bool WIFI_Update(config_wifi_t *config); // configurar com as configurações
bool WIFI_MODE(WIFI_mode_t mode);
void *       WIFI_Netif(WIFI_mode_t mode);
bool WIFI_connect(bool enable); // conectar ou desconectar da rede
uint8_t      WIFI_connected(); // retorna o status da rede
void         WIFI_get_IP(char * ip, void * netif = m_sta_netif); // Endereço IPv4

#endif /* WIFI_HPP */