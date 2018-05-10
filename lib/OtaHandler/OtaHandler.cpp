#include <App.hpp>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <SSD1306Wire.h>
#include "OtaHandler.hpp"

OtaHandler otaHandler;

extern SSD1306Wire oled;

#define OLED_WIDTH 64
#define OLED_HEIGHT 48

#define SHOW_OLED_PROGRESS

void OtaHandler::setup()
{
  LOG0("OTA Setup started...\n");

  ArduinoOTA.setHostname(appcfg.ota_hostname);
  ArduinoOTA.setPassword(appcfg.ota_password);

  ArduinoOTA.onStart([]()
  {
    Serial.println("\nOTA Start");
    oled.clear();
    oled.setFont(ArialMT_Plain_16);
    oled.drawString( 0, 0, "Upload" );
    oled.display();
  });

  ArduinoOTA.onEnd([]()
  {
    digitalWrite(WIFI_LED,0);
    Serial.println("\nOTA End\n");
    oled.clear();
    oled.setFont(ArialMT_Plain_10);
    oled.drawString( 0, 0, "*REBOOT*" );
    oled.display();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
  {
    unsigned int percent =  (progress / (total / 100));

    Serial.printf("\rOTA Progress: %u%%", percent );

#ifdef SHOW_OLED_PROGRESS
    char buffer[32];

    oled.clear();
    oled.drawString( 0, 0, "Upload" );
    oled.drawStringf( 0, 20, buffer, "%d%%", percent );
    oled.fillRect( 0, 43, OLED_WIDTH * progress / total, 4 );
    oled.display();
#endif

  });

  ArduinoOTA.onError([](ota_error_t error)
  {
    Serial.printf("OTA Error[%u]: ", error);

    if (error == OTA_AUTH_ERROR)
      Serial.println("OTA Auth Failed");
    else
    if (error == OTA_BEGIN_ERROR)
      Serial.println("OTA Begin Failed");
    else
    if (error == OTA_CONNECT_ERROR)
      Serial.println("OTA Connect Failed");
    else
    if (error == OTA_RECEIVE_ERROR)
      Serial.println("OTA Receive Failed");
    else
    if (error == OTA_END_ERROR)
      Serial.println("OTA End Failed");
  });

  ArduinoOTA.begin();
  MDNS.addServiceTxt("arduino", "tcp", "fw_name", APP_NAME );
  MDNS.addServiceTxt("arduino", "tcp", "fw_version", APP_VERSION );
  initialized = true;
}

void OtaHandler::handle()
{
  if( !initialized )
  {
    setup();
  }

  ArduinoOTA.handle();
}
