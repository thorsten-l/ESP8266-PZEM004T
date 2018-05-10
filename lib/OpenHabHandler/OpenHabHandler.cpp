#include <Arduino.h>
#include <App.hpp>
#include <WifiHandler.hpp>
#include <ESP8266HTTPClient.h>
#include <Pzem004Tnb.hpp>
#include "OpenHabHandler.hpp"

OpenHabHandler openHabHandler;


OpenHabHandler::OpenHabHandler()
{
  lastSendTimestamp = 0;
}

void OpenHabHandler::sendValueV1( const char* itemname, const float value )
{
  HTTPClient http;
  char urlBuffer[256];

  sprintf( urlBuffer, "http://%s:%d/CMD?%s=%0.2f", appcfg.ohab_host,
           appcfg.ohab_port, itemname, value );

  LOG1("URL=%s\n",urlBuffer);

  http.begin( urlBuffer );

  if ( appcfg.ohab_useauth )
  {
    http.setAuthorization(appcfg.ohab_user, appcfg.ohab_password);
  }

  int httpCode = http.GET();

  if (httpCode > 0)
  {
    LOG1("[HTTP] GET... code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK)
    {
      http.getString();
    }
  }
  else
  {
    LOG1("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

void OpenHabHandler::sendValueV2( const char* itemname, const float value )
{
  HTTPClient http;
  char urlBuffer[256];
  char buffer[32];

  sprintf( urlBuffer, "http://%s:%d/rest/items/%s", appcfg.ohab_host,
           appcfg.ohab_port, itemname );

  sprintf( buffer, "%0.2f", value );

  LOG1("URL=%s\n",urlBuffer);

  http.begin( urlBuffer );
  http.addHeader("Cache-Control", "no-cache");
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "text/plain");

  if ( appcfg.ohab_useauth )
  {
    http.setAuthorization(appcfg.ohab_user, appcfg.ohab_password);
  }

  int httpCode = http.POST(buffer);

  if (httpCode > 0)
  {
    LOG1("[HTTP] POST... code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK)
    {
      http.getString();
    }
  }
  else
  {
    LOG1("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}


void OpenHabHandler::sendValue( const char* itemname, const float value )
{
  if( appcfg.ohab_enabled && wifiHandler.isReady() )
  {
    if ( itemname != NULL && strlen(itemname) > 0 && itemname[0] != '-' )
    {
      if( appcfg.ohab_version == 1 )
      {
        sendValueV1( itemname, value );
      }
      else
      {
        sendValueV2( itemname, value );
      }
    }
  }
}


void OpenHabHandler::handle( time_t now )
{
  if ( appcfg.ohab_enabled && wifiHandler.isReady())
  {
    if (( now - lastSendTimestamp ) > (appcfg.ohab_sending_interval*1000))
    {
      LOG1( "OpenHAB sending values. (%lu)\n", now );
      sendValue( appcfg.ohab_item_voltage, pzem.getU() );
      sendValue( appcfg.ohab_item_current, pzem.getI() );
      sendValue( appcfg.ohab_item_power, pzem.getP() );
      lastSendTimestamp = now;
    }
  }
}
