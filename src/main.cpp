#include <Arduino.h>

#include <App.hpp>
#include <WifiHandler.hpp>
#include <OtaHandler.hpp>
#include <WebHandler.hpp>
#include <OpenHabHandler.hpp>
#include <MqttHandler.hpp>
#include <Pzem004Tnb.hpp>
#include <ESP8266WiFi.h>

#include <Wire.h>
#include <SSD1306Wire.h>
#include <SansSerif_plain_13.h>

SSD1306Wire oled(0x3c, D2, D1, GEOMETRY_64_48 ); // WEMOS OLED Shield

static unsigned long refreshTimestamp;
static unsigned long lifeTicker;
static unsigned long currentTimestamp;

void oledLog( const char* text )
{
  oled.clear();
  oled.drawString( 0, 0, text );
  oled.display();
}

void setup()
{
  delay(500);

  oled.init();
  oled.flipScreenVertically();
  oled.clear();

  oled.setTextAlignment(TEXT_ALIGN_LEFT);
  oled.setFont(SansSerif_plain_13);

  oledLog( "STARTUP\nVersion\n" APP_VERSION  );

  app.setup();
  app.writeConfig();
  app.printConfig();

  oledLog( "SETUP\nPZEM004" );
  Serial.print( "PZEM004T initialized = " );
  Serial.println( pzem.initialize() ? "true" : "false" );

  oledLog( "SETUP\nWiFi" );
  wifiHandler.setup();

  oledLog( "SETUP\ndone." );
  refreshTimestamp = millis();
}

void loop()
{
  currentTimestamp = millis();

  if (abs( currentTimestamp - lifeTicker ) >= 30000)
  {
    LOG1( "wifi is connected %d\n", wifiHandler.isConnected());
    lifeTicker = currentTimestamp;
  }

  if ( wifiHandler.handle( currentTimestamp ))
  {
    otaHandler.handle();
    webHandler.handle();
    openHabHandler.handle(currentTimestamp);
    mqttHandler.handle(currentTimestamp);

    if ( digitalRead(POWER_BUTTON) == false )
    {
      oled.clear();
      oled.drawString( 0, 0, "IP-Addr.");

      if ( wifiHandler.isInStationMode() )
      {
        IPAddress a = WiFi.localIP();

        char buffer[32];
        oled.drawStringf( 0, 16, buffer, "%d.%d.\n%d.%d", a[0], a[1], a[2], a[3] );
      }
      else
      {
        oled.drawString( 0, 16, "192.168.\n192.1" );
      }

      oled.display();

      refreshTimestamp = currentTimestamp;
    }

    if ( abs(currentTimestamp - refreshTimestamp) >= 2000 )
    {
      char buffer[32];
      LOG1( "%0.1fV;", pzem.getU() );
      Serial.printf( " %0.2fA; %0.1fW\n", pzem.getI(), pzem.getP() );

      oled.clear();
      oled.drawStringf( 0, 0, buffer, "%0.1fV\n", pzem.getU() );
      oled.drawStringf( 0, 16, buffer, "%0.2fA\n", pzem.getI() );
      oled.drawStringf( 0, 32, buffer, "%0.1fW\n", pzem.getP() );
      oled.display();

      refreshTimestamp = currentTimestamp;
    }

    pzem.handler();
  }

  app.handle();
}
