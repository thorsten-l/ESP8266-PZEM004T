#include <Arduino.h>
#include <App.hpp>
#include <WifiHandler.hpp>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Pzem004Tnb.hpp>
#include "MqttHandler.hpp"

MqttHandler mqttHandler;

static WiFiClient wifiClient;
static PubSubClient client(wifiClient);
static long lastReconnectAttempt = 0;

static void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }

  Serial.println();
}

MqttHandler::MqttHandler()
{
  initialized = false;
  lastPublishTimestamp = 0;
}

bool MqttHandler::reconnect()
{
  if (( appcfg.mqtt_useauth && client.connect(appcfg.mqtt_clientid, appcfg.mqtt_user, appcfg.mqtt_password ))
      || ( !appcfg.mqtt_useauth && client.connect(appcfg.mqtt_clientid)))
  {
    LOG0( "mqtt broker connected\n" );
  }

  return client.connected();
}

////////

void MqttHandler::setup()
{
  LOG0("MQTT Setup...\n");
  client.setServer( appcfg.mqtt_host, appcfg.mqtt_port );
  client.setCallback( callback );
  initialized = true;
}

void MqttHandler::handle( time_t now )
{
  if ( appcfg.mqtt_enabled && wifiHandler.isReady())
  {
    if ( initialized == false )
    {
      setup();
    }

    if (!client.connected())
    {
      if (now - lastReconnectAttempt > 5000)
      {
        lastReconnectAttempt = now;

        if (reconnect())
        {
          lastReconnectAttempt = 0;
        }
      }
    }
    else
    {
      client.loop();

      if (( now - lastPublishTimestamp ) > (appcfg.mqtt_sending_interval*1000))
      {
        char buffer[128];
        lastPublishTimestamp = now;
        sendValue( appcfg.mqtt_topic_voltage, pzem.getU());
        sendValue( appcfg.mqtt_topic_current, pzem.getI());
        sendValue( appcfg.mqtt_topic_power, pzem.getP());
        sprintf( buffer, "{\"voltage\":%0.1f,\"current\":%0.2f,\"power\":%0.1f,\"currentTimestamp\":%lu}", pzem.getU(), pzem.getI(), pzem.getP(), app.secTimestamp());
        sendValue( appcfg.mqtt_topic_json, buffer );
      }
    }
  }
}

void MqttHandler::sendValue( const char* topic, const char *value )
{
  if( appcfg.mqtt_enabled && wifiHandler.isReady() && client.connected())
  {
    if ( topic != NULL && value != NULL && strlen(topic) > 0 && topic[0] != '-' )
    {
      client.publish( topic, value );
    }
  }
}

void MqttHandler::sendValue( const char* topic, const float value )
{
  char buffer[32];
  sprintf( buffer, "%0.2f", value );
  sendValue( topic, buffer );
}
