#include <App.hpp>
#include <DefaultAppConfig.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "WifiHandler.hpp"

WifiHandler wifiHandler;

static time_t lastTimestamp;

static void wifiOff()
{
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  WiFi.setOutputPower(0.0f);
  WiFi.hostname(appcfg.ota_hostname);
}

static void wifiInitStationMode()
{
  LOG0("Starting Wifi in Station Mode");
  WiFi.begin( appcfg.wifi_ssid, appcfg.wifi_password );
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
}

void WifiHandler::setup()
{
  LOG0("WiFi Setup started...\n");
  connected = false;
  lastTimestamp = 0;
  connectCounter = 0;
  connectTimestamp = 0;

  wifiOff();

  scanNetworks();

  wifiOff();

  if ( isInStationMode() )
  {
    wifiInitStationMode();
  }
  else
  {
    LOG0("Starting Wifi Access Point Mode\n");
    char buffer[64];
    sprintf( buffer, DEFAULT_WIFI_SSID, ESP.getChipId());
    strcpy( appcfg.wifi_ssid, buffer );
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig( IPAddress( 192, 168, 192, 1 ),
                       IPAddress( 192, 168, 192, 1 ),
                       IPAddress( 255, 255, 255, 0 ));
    WiFi.softAP( appcfg.wifi_ssid, appcfg.wifi_password );
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    digitalWrite( WIFI_LED, true );
    connected = true;
    connectCounter++;
    connectTimestamp = app.secTimestamp();
  }
}

const bool WifiHandler::handle( time_t timestamp )
{
  if ( abs(timestamp - lastTimestamp) >= 500 && isInStationMode())
  {
    lastTimestamp = timestamp;
    int status = WiFi.status();

    if ( connected )
    {
      if ( status == WL_CONNECTED )
      {
        return true;
      }
      else
      {
        LOG0( "WARNING: WiFi connection lost!\n" );

        wifiOff();
        wifiInitStationMode();

        connected = false;
      }
    }
    else
    {
      if ( status == WL_CONNECTED )
      {
        Serial.println("\n");
        Serial.printf("WiFi connected to %s\n", appcfg.wifi_ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.println();
        digitalWrite( WIFI_LED, true );
        connected = true;
        connectCounter++;
        connectTimestamp = app.secTimestamp();
      }
      else
      {
        Serial.print( "." );
        digitalWrite(WIFI_LED, digitalRead(WIFI_LED) ^ true);
      }
    }
  }
  return connected;
}

const bool WifiHandler::isInStationMode()
{
  return (appcfg.wifi_mode == WIFI_STA);
}

const bool WifiHandler::isConnected()
{
  return connected;
}

const bool WifiHandler::isReady()
{
  return isConnected() && isInStationMode();
}

const char* WifiHandler::scanNetworks()
{
  networkBuffer[0] = 0;

  Serial.println( "\nScanning WiFi networks...");
  int n = WiFi.scanNetworks(false, false);
  Serial.println( "done.");

  if (n == 0)
  {
    Serial.println("no networks found");
    strcpy( networkBuffer, "no networks found" );
  }
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    int l = 0;

    for (int i = 0; i < n; ++i)
    {
      l += sprintf( networkBuffer+l, "%2d: %s (%d)%s\n", i+1,
                    WiFi.SSID(i).c_str(), WiFi.RSSI(i),
                    (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay( 5 );
    }
  }

  Serial.println("----------------------------------------------");
  Serial.print(networkBuffer);
  Serial.println("----------------------------------------------");

  return networkBuffer;
}

const char* WifiHandler::getScannedNetworks()
{
  return networkBuffer;
}

unsigned long WifiHandler::getConnectCounter()
{
  return connectCounter;
}

unsigned long WifiHandler::getConnectTimestamp()
{
  return connectTimestamp;
}
