#include "App.hpp"
#include "DefaultAppConfig.h"
#include <FS.h>
#include <SSD1306Wire.h>
#include <Ticker.h>

App app;
AppConfig appcfg;

extern SSD1306Wire oled;

Ticker appTicker;
volatile unsigned long secCounter;

void appTickerCallback()
{
  secCounter++;
}

App::App()
{
  initialized = true;
  secCounter = 0;

  strncpy( appcfg.wifi_ssid, DEFAULT_WIFI_SSID, 63 );
  strncpy( appcfg.wifi_password, DEFAULT_WIFI_PASSWORD, 63 );
  appcfg.wifi_mode = DEFAULT_WIFI_MODE;

  strncpy( appcfg.ota_hostname, DEFAULT_OTA_HOSTNAME, 63 );
  strncpy( appcfg.ota_password, DEFAULT_OTA_PASSWORD, 63 );

  strncpy( appcfg.admin_password, DEFAULT_ADMIN_PASSWORD, 63 );

  appcfg.ohab_enabled = DEFAULT_OHAB_ENABLED;
  appcfg.ohab_version = DEFAULT_OHAB_VERSION;
  strncpy( appcfg.ohab_host, DEFAULT_OHAB_HOST, 63 );
  appcfg.ohab_port = DEFAULT_OHAB_PORT;
  appcfg.ohab_useauth = DEFAULT_OHAB_USEAUTH;
  strncpy( appcfg.ohab_user, DEFAULT_OHAB_USER, 63 );
  strncpy( appcfg.ohab_password, DEFAULT_OHAB_PASSWORD, 63 );
  strncpy( appcfg.ohab_item_voltage, DEFAULT_OHAB_ITEM_VOLTAGE, 63 );
  strncpy( appcfg.ohab_item_current, DEFAULT_OHAB_ITEM_CURRENT, 63 );
  strncpy( appcfg.ohab_item_power, DEFAULT_OHAB_ITEM_POWER, 63 );
  appcfg.ohab_sending_interval = DEFAULT_OHAB_SENDING_INTERVAL;

  appcfg.mqtt_enabled = DEFAULT_MQTT_ENABLED;
  strncpy( appcfg.mqtt_clientid, DEFAULT_MQTT_CLIENTID, 63 );
  strncpy( appcfg.mqtt_host, DEFAULT_MQTT_HOST, 63 );
  appcfg.mqtt_port = DEFAULT_MQTT_PORT;
  appcfg.mqtt_useauth = DEFAULT_MQTT_USEAUTH;
  strncpy( appcfg.mqtt_user, DEFAULT_MQTT_USER, 63 );
  strncpy( appcfg.mqtt_password, DEFAULT_MQTT_PASSWORD, 63 );
  strncpy( appcfg.mqtt_topic_voltage, DEFAULT_MQTT_TOPIC_VOLTAGE, 63 );
  strncpy( appcfg.mqtt_topic_current, DEFAULT_MQTT_TOPIC_CURRENT, 63 );
  strncpy( appcfg.mqtt_topic_power, DEFAULT_MQTT_TOPIC_POWER, 63 );
  strncpy( appcfg.mqtt_topic_json, DEFAULT_MQTT_TOPIC_JSON, 63 );
  appcfg.mqtt_sending_interval = DEFAULT_MQTT_SENDING_INTERVAL;
}

void App::restartSystem()
{
  // watchdogTicker.detach();
  ESP.eraseConfig();
  LOG0( "*** restarting system ***\n" );
  delay( 2000 );
  ESP.restart();
  delay( 2000 );
  ESP.reset();
}

void App::setup()
{
  Serial.begin(115200);

  doSystemRestart = false;
  appTicker.attach( 1.0, appTickerCallback );

  pinMode( WIFI_LED, OUTPUT);
  digitalWrite( WIFI_LED, 0 );
  pinMode( POWER_BUTTON, INPUT_PULLUP);

  for( int i=0; i<5; i++)
  {
    digitalWrite( WIFI_LED, 1 );
    delay(500);
    digitalWrite( WIFI_LED, 0 );
    delay(500);
  }

  Serial.println();
  Serial.println("\n\n");
  Serial.println("\n\n");
  Serial.println(F(APP_NAME ", Version " APP_VERSION ", by " APP_AUTHOR));
  Serial.println( "Build date: " __DATE__ " " __TIME__  );
  Serial.printf("appcfg file size: %d bytes\n\n", sizeof( appcfg ));

  if ( digitalRead(POWER_BUTTON) == false )
  {
    Serial.println();
    LOG0("*** RESET appcfguration ***\n");
    Serial.println();

    oled.clear();
    oled.drawString( 0, 0, "Firmware\nReset" );
    oled.display();

    for( int i=0; i<15; i++)
    {
      digitalWrite( WIFI_LED, 1 );
      delay( 100 );
      digitalWrite( WIFI_LED, 0 );
      delay( 100 );
    }

    digitalWrite( WIFI_LED, 1 );

    ESP.eraseConfig();

    if (SPIFFS.begin())
    {
      LOG0("File system format started...\n");
      SPIFFS.format();
      LOG0("File system format finished.\n");
      SPIFFS.end();
    }

    digitalWrite( WIFI_LED, 0 );
    restartSystem();
  }

  ESP.eraseConfig();
  loadConfig();
}


void App::loadConfig()
{
  if (!SPIFFS.begin())
  {
    LOG0("ERROR: Failed to mount file system");
  }
  else
  {
    if( SPIFFS.exists(APP_CONFIG_FILE))
    {
      File configFile = SPIFFS.open( APP_CONFIG_FILE, "r");

      if (!configFile)
      {
        LOG1("ERROR: file %s not found.\n", APP_CONFIG_FILE );
      }
      else
      {
        LOG1("Loading appcfguration from %s file...\n", APP_CONFIG_FILE );

        if( configFile.size() != sizeof( appcfg ))
        {
          Serial.printf( "ERROR: %s file size not match appcfg structure %d != %d bytes.\n", APP_CONFIG_FILE, configFile.size(), sizeof( appcfg ));
        }
        else
        {
          int bytesRead = configFile.readBytes((char *)&appcfg, sizeof( appcfg ));
          LOG1( "%d bytes read from appcfg file.\n", bytesRead );
          configFile.close();
        }
      }
    }
    else
    {
      LOG0( "WARNING: appcfg file " APP_CONFIG_FILE " does not exist. Using default appcfg.\n" );
    }
    SPIFFS.end();
  }
}

void App::writeConfig()
{
  if (!SPIFFS.begin())
  {
    LOG0("ERROR: Failed to mount file system");
  }
  else
  {
    LOG1("writing file %s.\n", APP_CONFIG_FILE );

    File configFile = SPIFFS.open( APP_CONFIG_FILE, "w");

    if (!configFile)
    {
      LOG1("ERROR: Failed to open appcfg file %s for writing.\n", APP_CONFIG_FILE );
      return;
    }

    int length = configFile.write((const uint8_t *)&appcfg, sizeof( appcfg ));
    LOG1("%d bytes written to appcfg file.\n", length );

    configFile.close();

    FSInfo fs_info;
    SPIFFS.info(fs_info);

    Serial.printf( "\n--- SPIFFS Info ---\ntotal bytes = %d\n", fs_info.totalBytes );
    Serial.printf( "used bytes = %d\n", fs_info.usedBytes );
    Serial.printf( "block size = %d\n", fs_info.blockSize );
    Serial.printf( "page size = %d\n", fs_info.pageSize );
    Serial.printf( "max open files = %d\n", fs_info.maxOpenFiles );
    Serial.printf( "max path length = %d\n", fs_info.maxPathLength );

    SPIFFS.end();
  }
}

void App::printConfig()
{
  Serial.println();
  Serial.println( "--- App appcfguration -----------------------------------" );
  Serial.println( "  Security:" );
  Serial.printf(  "    Admin password: %s\n", appcfg.admin_password );
  Serial.println( "\n  WiFi:" );
  Serial.printf(  "    SSID: %s\n", appcfg.wifi_ssid );
  Serial.printf(  "    Password: %s\n", appcfg.wifi_password );
  Serial.printf(  "    Mode: %s\n", ( appcfg.wifi_mode == 1 ) ? "Station" : "Access Point" );
  Serial.println( "\n  OTA:" );
  Serial.printf(  "    Hostname: %s\n", appcfg.ota_hostname );
  Serial.printf(  "    Password: %s\n", appcfg.ota_password );
  Serial.println( "\n  OpenHAB:" );
  Serial.printf(  "    Enabled: %s\n", (appcfg.ohab_enabled ? "true" : "false" ));
  Serial.printf(  "    Version: %d\n", appcfg.ohab_version );
  Serial.printf(  "    Host: %s\n", appcfg.ohab_host );
  Serial.printf(  "    Port: %d\n", appcfg.ohab_port );
  Serial.printf(  "    Use Auth: %s\n", (appcfg.ohab_useauth ? "true" : "false" ));
  Serial.printf(  "    User: %s\n", appcfg.ohab_user );
  Serial.printf(  "    Password: %s\n", appcfg.ohab_password );
  Serial.printf(  "    Item Voltage: %s\n", appcfg.ohab_item_voltage );
  Serial.printf(  "    Item Current: %s\n", appcfg.ohab_item_current );
  Serial.printf(  "    Item Power: %s\n", appcfg.ohab_item_power );
  Serial.println( "\n  MQTT:" );
  Serial.printf(  "    Enabled: %s\n", (appcfg.mqtt_enabled ? "true" : "false" ));
  Serial.printf(  "    Client ID: %s\n", appcfg.mqtt_clientid );
  Serial.printf(  "    Host: %s\n", appcfg.mqtt_host );
  Serial.printf(  "    Port: %d\n", appcfg.mqtt_port );
  Serial.printf(  "    Use Auth: %s\n", (appcfg.mqtt_useauth ? "true" : "false" ));
  Serial.printf(  "    User: %s\n", appcfg.mqtt_user );
  Serial.printf(  "    Password: %s\n", appcfg.mqtt_password );
  Serial.printf(  "    Topic Voltage: %s\n", appcfg.mqtt_topic_voltage );
  Serial.printf(  "    Topic Current: %s\n", appcfg.mqtt_topic_current );
  Serial.printf(  "    Topic Power: %s\n", appcfg.mqtt_topic_power );
  Serial.println( "---------------------------------------------------------" );
  Serial.println();
}

void App::delayedSystemRestart()
{
  doSystemRestart = true;
  systemRestartTimestamp = millis();
}

void App::handle()
{
  if( doSystemRestart && ( millis() - systemRestartTimestamp ) > 5000 )
  {
    writeConfig();
    restartSystem();
  }
}

unsigned long App::secTimestamp()
{
  return secCounter;
}
