#ifndef __APP_H__
#define __APP_H__

#include <Arduino.h>

#define LOG0( format ) Serial.printf( "(%ld) " format, millis())
#define LOG1( format, x) Serial.printf( "(%ld) " format, millis(), x )

// #define LOG0( format )
// #define LOG1( format, x)

#define APP_NAME "PowerMeter"
#define APP_VERSION "1.0.1"
#define APP_AUTHOR "Dr. Thorsten Ludewig <t.ludewig@gmail.com>"
#define APP_CONFIG_FILE "/config.bin"

#define WIFI_LED D0
#define POWER_BUTTON D7

typedef struct appconfig
{
  char wifi_ssid[64];
  char wifi_password[64];
  int  wifi_mode;

  char ota_hostname[64];
  char ota_password[64];

  char admin_password[64];

  bool ohab_enabled;
  int  ohab_version;
  char ohab_host[64];
  int  ohab_port;
  bool ohab_useauth;
  char ohab_user[64];
  char ohab_password[64];
  char ohab_item_voltage[64];
  char ohab_item_current[64];
  char ohab_item_power[64];
  time_t ohab_sending_interval;

  bool mqtt_enabled;
  char mqtt_clientid[64];
  char mqtt_host[64];
  int  mqtt_port;
  bool mqtt_useauth;
  char mqtt_user[64];
  char mqtt_password[64];
  char mqtt_topic_voltage[64];
  char mqtt_topic_current[64];
  char mqtt_topic_power[64];
  char mqtt_topic_json[64];
  time_t mqtt_sending_interval;

} AppConfig;

class App
{
private:
  bool initialized = false;
  bool doSystemRestart;
  time_t systemRestartTimestamp;

  void loadConfig();
  void restartSystem();

public:
  App();

  void setup();
  void writeConfig();
  void printConfig();
  void delayedSystemRestart();
  void handle();
  unsigned long secTimestamp();
};

extern App app;
extern AppConfig appcfg;

#endif
