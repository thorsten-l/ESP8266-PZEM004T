#ifndef __WIFI_HANDLER_H__
#define __WIFI_HANDLER_H__

class WifiHandler
{
private:
  unsigned long connectTimestamp;
  unsigned long connectCounter;
  bool connected;
  char networkBuffer[1024];
  const char* scanNetworks();

public:
  void setup();
  const bool isReady();
  const bool isConnected();
  const bool isInStationMode();
  const char* getScannedNetworks();
  const bool handle( time_t timestamp );
  unsigned long getConnectTimestamp();
  unsigned long getConnectCounter();
};

extern WifiHandler wifiHandler;

#endif
