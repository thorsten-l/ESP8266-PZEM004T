#ifndef __DEFAULT_APP_CONFIG_H__
#define __DEFAULT_APP_CONFIG_H__

#include <ESP8266WiFi.h>

#define DEFAULT_WIFI_SSID "PowerMeter-%06x"
#define DEFAULT_WIFI_PASSWORD "12345678"
#define DEFAULT_WIFI_MODE WIFI_AP

#define DEFAULT_OTA_HOSTNAME "power-meter-1"
#define DEFAULT_OTA_PASSWORD "otapass"

#define DEFAULT_ADMIN_PASSWORD "admin"

#define DEFAULT_OHAB_ENABLED false
#define DEFAULT_OHAB_VERSION 1
#define DEFAULT_OHAB_HOST "192.168.1.1"
#define DEFAULT_OHAB_PORT 80
#define DEFAULT_OHAB_USEAUTH true
#define DEFAULT_OHAB_USER "user"
#define DEFAULT_OHAB_PASSWORD "password"
#define DEFAULT_OHAB_ITEM_VOLTAGE "PowerMeter1Voltage"
#define DEFAULT_OHAB_ITEM_CURRENT "PowerMeter1Current"
#define DEFAULT_OHAB_ITEM_POWER "PowerMeter1Power"
#define DEFAULT_OHAB_SENDING_INTERVAL 60

#define DEFAULT_MQTT_ENABLED false
#define DEFAULT_MQTT_CLIENTID "PowerMeter1"
#define DEFAULT_MQTT_HOST "192.168.1.1"
#define DEFAULT_MQTT_PORT 1883
#define DEFAULT_MQTT_USEAUTH true
#define DEFAULT_MQTT_USER "user"
#define DEFAULT_MQTT_PASSWORD "password"
#define DEFAULT_MQTT_TOPIC_VOLTAGE "powermeter1/voltage"
#define DEFAULT_MQTT_TOPIC_CURRENT "powermeter1/current"
#define DEFAULT_MQTT_TOPIC_POWER "powermeter1/power"
#define DEFAULT_MQTT_TOPIC_JSON "powermeter1/json"
#define DEFAULT_MQTT_SENDING_INTERVAL 60

#endif
