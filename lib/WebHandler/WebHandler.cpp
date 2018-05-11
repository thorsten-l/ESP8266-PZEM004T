#include <App.hpp>
#include <DefaultAppConfig.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WifiHandler.hpp>
#include <Pzem004Tnb.hpp>
#include "WebHandler.hpp"
#include "pure-min-css-gz.h"
#include "layout-css-gz.h"
#include "template-html.h"

WebHandler webHandler;

static AsyncWebServer server(80);

String jsonStatus()
{
  String message = "{\"voltage\":";
  message += pzem.getU();
  message += ",\"current\":";
  message += pzem.getI();
  message += ",\"power\":";
  message += pzem.getP();

  message += ",\"currentTimestamp\":";
  message += app.secTimestamp();
  message += ",\"pzemValidReadTimestamp\":";
  message += pzem.validReadTimestamp;
  message += ",\"pzemReadErrorCounter\":";
  message += pzem.errorCounter;
  message += ",\"pzemLastErrorTimestamp\":";
  message += pzem.lastErrorTimestamp;
  message += ",\"wifiConnectCounter\":";
  message += wifiHandler.getConnectCounter();
  message += ",\"wifiConnectTimestamp\":";
  message += wifiHandler.getConnectTimestamp();

  message += "}\r\n";
  return message;
}

void handlePageNotFound( AsyncWebServerRequest *request )
{
  request->send(404);
}

void prLegend( AsyncResponseStream *response, const char *name )
{
  response->printf( "<legend>%s</legend>", name );
}

void prGroupLabel( AsyncResponseStream *response, int id, const char *label )
{
  response->printf(
    "<div class='pure-control-group'>"
      "<label for='pgid%d'>%s</label>", id, label );
}

void prTextGroup( AsyncResponseStream *response, int id, const char *label,
   const char *name, const char *value )
{
  prGroupLabel( response, id, label );
  response->printf(
      "<input id='pgid%d' type='text' name='%s' maxlength='64' value='%s'>"
    "</div>", id, name, value );
}

void prTextGroupReadOnly( AsyncResponseStream *response, int id, const char *label,
  const char *value )
{
  prGroupLabel( response, id, label );
  response->printf(
      "<input id='pgid%d' type='text' maxlength='64' readonly value='%s'>"
    "</div>", id, value );
}

void prTextGroup( AsyncResponseStream *response, int id, const char *label,
   const char *name, int value )
{
  prGroupLabel( response, id, label );
  response->printf(
      "<input id='pgid%d' type='text' name='%s' maxlength='64' value='%d'>"
    "</div>", id, name, value );
}

void prCheckBoxGroup( AsyncResponseStream *response, int id, const char *label,
   const char *name, bool value )
{
  prGroupLabel( response, id, label );
  response->printf(
      "<input id='pgid%d' type='checkbox' name='%s' value='true' %s>"
    "</div>", id, name, value ? "checked" : "" );
}

void prSelectStart( AsyncResponseStream *response, int id, const char *label, const char *name )
{
  prGroupLabel( response, id, label );
  response->printf("<select id='pgid%d' name='%s'>", id, name );
}

void prSelectEnd( AsyncResponseStream *response )
{
  response->print("</select></div>" );
}

void prOption( AsyncResponseStream *response, int value, const char *name, bool selected )
{
  response->printf("<option %s value='%d'>%s</option>", selected ? "selected" : "", value, name );
}

void handleSetupPage( AsyncWebServerRequest *request )
{
  int id = 0;

  if(!request->authenticate("admin", appcfg.admin_password))
  {
    return request->requestAuthentication();
  }

  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->printf( TEMPLATE_HEADER, "", APP_NAME " - Setup");
  response->println("<form class=\"pure-form pure-form-aligned\" action='/savecfg' method=POST><fieldset>");

  // Setup

  prLegend( response, "Setup 'admin' user" );
  prTextGroup( response, id++, "Password", "admin_password", appcfg.admin_password );

  // WiFi
  prLegend( response, "WiFi Network Scan" );
  response->printf("<pre>%s</pre>\n", wifiHandler.getScannedNetworks() );

  prLegend( response, "WiFi" );

  prSelectStart( response, id++, "Mode", "wifi_mode" );
  prOption( response, WIFI_AP, "Access Point", appcfg.wifi_mode == WIFI_AP );
  prOption( response, WIFI_STA, "Station", appcfg.wifi_mode == WIFI_STA );
  prSelectEnd( response );

  prTextGroup( response, id++, "SSID", "wifi_ssid", appcfg.wifi_ssid );
  prTextGroup( response, id++, "Password", "wifi_password", appcfg.wifi_password );

  // OTA (Over The Air - firmware update)
  prLegend( response, "Over The Air - firmware update (OTA)");
  prTextGroup( response, id++, "Hostname", "ota_hostname", appcfg.ota_hostname );
  prTextGroup( response, id++, "Password", "ota_password", appcfg.ota_password );

  // OpenHAB
  prLegend( response, "OpenHAB");
  prCheckBoxGroup( response, id++, "Callback Enabled", "ohab_enabled", appcfg.ohab_enabled );

  prSelectStart( response, id++, "OpenHAB Version", "ohab_version" );
  prOption( response, 1, "1.8", appcfg.ohab_version == 1 );
  prOption( response, 2, "&gt;=2.0", appcfg.ohab_version == 2 );
  prSelectEnd( response );

  response->print( "<p style='padding-left: 6em' class='pure-control-group'><em>To disable an item start value with a '-' sign.</em></p>" );

  prTextGroup( response, id++, "Item Voltage", "ohab_item_voltage", appcfg.ohab_item_voltage );
  prTextGroup( response, id++, "Item Current", "ohab_item_current", appcfg.ohab_item_current );
  prTextGroup( response, id++, "Item Power", "ohab_item_power", appcfg.ohab_item_power );
  prTextGroup( response, id++, "Host", "ohab_host", appcfg.ohab_host );
  prTextGroup( response, id++, "Port", "ohab_port", appcfg.ohab_port );
  prCheckBoxGroup( response, id++, "Use Authentication", "ohab_useauth", appcfg.ohab_useauth );
  prTextGroup( response, id++, "User", "ohab_user", appcfg.ohab_user );
  prTextGroup( response, id++, "Password", "ohab_password", appcfg.ohab_password );
  prTextGroup( response, id++, "Sending Interval", "ohab_sending_interval", appcfg.ohab_sending_interval );


  // MQTT
  prLegend( response, "MQTT");
  prCheckBoxGroup( response, id++, "Enabled", "mqtt_enabled", appcfg.mqtt_enabled );
  prTextGroup( response, id++, "Client ID", "mqtt_clientid", appcfg.mqtt_clientid );
  prTextGroup( response, id++, "Host", "mqtt_host", appcfg.mqtt_host );
  prTextGroup( response, id++, "Port", "mqtt_port", appcfg.mqtt_port );
  prCheckBoxGroup( response, id++, "Use Authentication", "mqtt_useauth", appcfg.mqtt_useauth );
  prTextGroup( response, id++, "User", "mqtt_user", appcfg.mqtt_user );
  prTextGroup( response, id++, "Password", "mqtt_password", appcfg.mqtt_password );

  response->print( "<p style='padding-left: 6em' class='pure-control-group'><em>To disable a topic start value with a '-' sign.</em></p>" );

  prTextGroup( response, id++, "Topic Voltage", "mqtt_topic_voltage", appcfg.mqtt_topic_voltage );
  prTextGroup( response, id++, "Topic Current", "mqtt_topic_current", appcfg.mqtt_topic_current );
  prTextGroup( response, id++, "Topic Power", "mqtt_topic_power", appcfg.mqtt_topic_power );
  prTextGroup( response, id++, "Topic JSON", "mqtt_topic_json", appcfg.mqtt_topic_json );
  prTextGroup( response, id++, "Sending Interval", "mqtt_sending_interval", appcfg.mqtt_sending_interval );

  response->println("<p><input class='pure-button pure-button-primary' type='submit' value='Save Configuration'></p>");
  response->println("</fieldset></form>");
  response->print( TEMPLATE_FOOTER );
  request->send(response);
}

void paramChars( AsyncWebServerRequest *request, char *dest,
   const char* paramName, const char* defaultValue )
{
  const char *value = defaultValue;

  if(request->hasParam( paramName, true ))
  {
    AsyncWebParameter* p = request->getParam(paramName, true);
    value = p->value().c_str();
    if ( value == 0 || strlen( value ) == 0 )
    {
      value = defaultValue;
    }
  }

  strncpy( dest, value, 63 );
  dest[63] = 0;
}

int paramInt( AsyncWebServerRequest *request, const char* paramName, int defaultValue )
{
  int value = defaultValue;

  if(request->hasParam( paramName, true ))
  {
    AsyncWebParameter* p = request->getParam(paramName, true);
    const char *pv = p->value().c_str();
    if ( pv != 0 && strlen( pv ) > 0 )
    {
      value = atoi( pv );
    }
  }

  return value;
}

bool paramBool( AsyncWebServerRequest *request, const char* paramName )
{
  bool value = false;

  if(request->hasParam( paramName, true ))
  {
    AsyncWebParameter* p = request->getParam(paramName, true);
    const char *pv = p->value().c_str();
    if ( pv != 0 && strlen( pv ) > 0 )
    {
      value = strcmp( "true", pv ) == 0;
    }
  }
  return value;
}

void handleSavePage( AsyncWebServerRequest *request )
{
  if(!request->authenticate("admin", appcfg.admin_password))
  {
    return request->requestAuthentication();
  }

  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->printf( TEMPLATE_HEADER, "", APP_NAME " - Save Configuration");
  response->print("<pre>");

  int params = request->params();

  for(int i=0;i<params;i++)
  {
    AsyncWebParameter* p = request->getParam(i);
    response->printf("%s = '%s'\n", p->name().c_str(), p->value().c_str());
  }

  // Security
  paramChars( request, appcfg.admin_password, "admin_password", DEFAULT_ADMIN_PASSWORD );

  // WIFI
  appcfg.wifi_mode = paramInt( request, "wifi_mode", DEFAULT_WIFI_MODE);
  paramChars( request, appcfg.wifi_ssid, "wifi_ssid", DEFAULT_WIFI_SSID );
  paramChars( request, appcfg.wifi_password, "wifi_password", DEFAULT_WIFI_PASSWORD );

  // OTA
  paramChars( request, appcfg.ota_hostname, "ota_hostname", DEFAULT_OTA_HOSTNAME );
  paramChars( request, appcfg.ota_password, "ota_password", DEFAULT_OTA_PASSWORD );

  // OpenHAB
  appcfg.ohab_enabled = paramBool( request, "ohab_enabled" );
  appcfg.ohab_version = paramInt( request, "ohab_version", DEFAULT_OHAB_VERSION );
  paramChars( request, appcfg.ohab_item_voltage, "ohab_item_voltage", DEFAULT_OHAB_ITEM_VOLTAGE );
  paramChars( request, appcfg.ohab_item_current, "ohab_item_current", DEFAULT_OHAB_ITEM_CURRENT );
  paramChars( request, appcfg.ohab_item_power, "ohab_item_power", DEFAULT_OHAB_ITEM_POWER );
  paramChars( request, appcfg.ohab_host, "ohab_host", DEFAULT_OHAB_HOST );
  appcfg.ohab_port = paramInt( request, "ohab_port", DEFAULT_OHAB_PORT );
  appcfg.ohab_useauth = paramBool( request, "ohab_useauth" );
  paramChars( request, appcfg.ohab_user, "ohab_user", DEFAULT_OHAB_USER );
  paramChars( request, appcfg.ohab_password, "ohab_password", DEFAULT_OHAB_PASSWORD );
  appcfg.ohab_sending_interval = paramInt( request, "ohab_sending_interval", DEFAULT_OHAB_SENDING_INTERVAL );

  // MQTT
  appcfg.mqtt_enabled = paramBool( request, "mqtt_enabled" );
  paramChars( request, appcfg.mqtt_clientid, "mqtt_clientid", DEFAULT_MQTT_CLIENTID );
  paramChars( request, appcfg.mqtt_host, "mqtt_host", DEFAULT_MQTT_HOST );
  appcfg.mqtt_port = paramInt( request, "mqtt_port", DEFAULT_MQTT_PORT );
  appcfg.mqtt_useauth = paramBool( request, "mqtt_useauth" );
  paramChars( request, appcfg.mqtt_user, "mqtt_user", DEFAULT_MQTT_USER );
  paramChars( request, appcfg.mqtt_password, "mqtt_password", DEFAULT_MQTT_PASSWORD );
  paramChars( request, appcfg.mqtt_topic_voltage, "mqtt_topic_voltage", DEFAULT_MQTT_TOPIC_VOLTAGE );
  paramChars( request, appcfg.mqtt_topic_current, "mqtt_topic_current", DEFAULT_MQTT_TOPIC_CURRENT );
  paramChars( request, appcfg.mqtt_topic_power, "mqtt_topic_power", DEFAULT_MQTT_TOPIC_POWER );
  paramChars( request, appcfg.mqtt_topic_json, "mqtt_topic_json", DEFAULT_MQTT_TOPIC_JSON );
  appcfg.mqtt_sending_interval = paramInt( request, "mqtt_sending_interval", DEFAULT_MQTT_SENDING_INTERVAL );

  response->println("</pre>");
  response->println("<h2 style='color: red'>Restarting System in 5sec</h2>");
  response->print( TEMPLATE_FOOTER );
  request->send(response);
  app.delayedSystemRestart();
}


void handleRootPage( AsyncWebServerRequest *request )
{
  char titleBuffer[100];
  sprintf( titleBuffer, APP_NAME " - %s", appcfg.ota_hostname );

  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->printf( TEMPLATE_HEADER, "", titleBuffer );

  response->println("<form class=\"pure-form pure-form-aligned\"><fieldset>");
  prLegend( response, "Status");

  int rid=0;

  char valueBuffer[32];

  sprintf( valueBuffer, "%0.1fV", pzem.getU() );
  prTextGroupReadOnly( response, rid++, "Voltage", valueBuffer );
  sprintf( valueBuffer, "%0.2fA", pzem.getI() );
  prTextGroupReadOnly( response, rid++, "Current", valueBuffer );
  sprintf( valueBuffer, "%0.1fW", pzem.getP() );
  prTextGroupReadOnly( response, rid++, "Power", valueBuffer );

  response->println("</fieldset></form>");

  response->print( "<script>function getPowerState(){var e=document.getElementById('pgid0'),t=document.getElementById('pgid1'),n=document.getElementById('pgid2');fetch('/state').then(resp=>resp.json()).then(function(o){e.value=o.voltage.toFixed(1)+'V',t.value=o.current.toFixed(2)+'A',n.value=o.power.toFixed(1)+'W'})}setInterval(getPowerState,5e3);</script>" );

  response->print( TEMPLATE_FOOTER );
  request->send(response);
}


WebHandler::WebHandler()
{
  initialized = false;
}

void WebHandler::setup()
{
  LOG0("HTTP server setup...\n");

  server.on( "/", HTTP_GET, handleRootPage );
  server.on( "/setup.html", HTTP_GET, handleSetupPage );
  server.on( "/savecfg", HTTP_POST, handleSavePage );
  server.onNotFound( handlePageNotFound );

  server.on("/state", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonStatus() );
    response->addHeader( "Access-Control-Allow-Origin", "*" );
    request->send(response);
  });

  server.on("/pure-min.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css",
            PURE_MIN_CSS_GZ, PURE_MIN_CSS_GZ_LEN);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/layout.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css",
            LAYOUT_CSS_GZ, LAYOUT_CSS_GZ_LEN);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/info.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->printf( TEMPLATE_HEADER, "", APP_NAME " - Info");

    response->print("<form class='pure-form'>");

    prLegend( response, "Application");

    response->print(
      "<p>Name: " APP_NAME "</p>"
      "<p>Version: " APP_VERSION "</p>"
      "<p>Author: Dr. Thorsten Ludewig &lt;t.ludewig@gmail.com></p>");

    prLegend( response, "RESTful API");
      char ipAddress[16];
      strcpy( ipAddress, WiFi.localIP().toString().c_str());

    response->printf(
      "<p><a href='http://%s/state'>http://%s/state</a> - PowerMeter JSON status</p>",
      ipAddress, ipAddress );

      prLegend( response, "Build");
      response->print(
        "<p>Date: " __DATE__ "</p>"
        "<p>Time: " __TIME__ "</p>");

    prLegend( response, "Services");
    response->printf( "<p>OpenHAB Callback Enabled: %s</p>", (appcfg.ohab_enabled) ? "true" : "false" );
    response->printf( "<p>MQTT Enabled: %s</p>", (appcfg.mqtt_enabled) ? "true" : "false" );

    response->print("</form>");
    response->print( TEMPLATE_FOOTER );
    request->send( response );
  });

  MDNS.addService("http", "tcp", 80);
  MDNS.addServiceTxt( "http", "tcp", "path", "/" );
  MDNS.addServiceTxt( "http", "tcp", "fw_name", APP_NAME );
  MDNS.addServiceTxt( "http", "tcp", "fw_version", APP_VERSION );

  server.begin();

  LOG0("HTTP server started\n");
  initialized = true;
}

void WebHandler::handle()
{
  if( ! initialized )
  {
    setup();
  }
}
