#ifndef __MQTT_HANDLER_H__
#define __MQTT_HANDLER_H__

class MqttHandler
{
private:
  bool initialized;
  bool reconnect();
  void setup();
  time_t lastPublishTimestamp;

public:
  MqttHandler();
  void handle( time_t now );
  void sendValue( const char* topic, const char* value );
  void sendValue( const char* topic, const float value );
};

extern MqttHandler mqttHandler;

#endif
