#ifndef __PZEM004TNB_HPP__
#define __PZEM004TNB_HPP__

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <IPAddress.h>

enum pzemState {
  SEND_U,
  RECEIVE_U,
  SEND_I,
  RECEIVE_I,
  ALLREAD_EVENT
};

struct pzemCommand
{
  uint8_t command;
  uint8_t addr[4];
  uint8_t data;
  uint8_t crc;
};

#define RESPONSE_SIZE sizeof(pzemCommand)
#define RESPONSE_DATA_SIZE RESPONSE_SIZE - 2


class Pzem004Tnb
{
private:
  float pzemU = 0.0f;
  float pzemI = 0.0f;

  IPAddress ipAddress;

  Stream *serial;
  unsigned long readTimeOut;
  bool serialIsSoft;

  enum pzemState handlerState;
  unsigned long handlerTimeStamp;
  unsigned long eventTimeStamp;

  uint8_t data[RESPONSE_DATA_SIZE];
  uint8_t buffer[RESPONSE_SIZE];
  int bufferIndex = 0;

  void send(uint8_t cmd, uint8_t data = 0);
  bool receive(uint8_t resp, uint8_t *data = 0);
  uint8_t crc(uint8_t *data, uint8_t sz);
  bool nbReceive();
  bool checkData(uint8_t resp);

public:
  unsigned long errorCounter;
  unsigned long lastErrorTimestamp;
  unsigned long validReadTimestamp;

  Pzem004Tnb(const IPAddress &ipAddr, const uint8_t receivePin, const uint8_t transmitPin);
  bool initialize();
  void handler();
  float getU();
  float getI();
  float getP();
};

extern Pzem004Tnb pzem;

#endif
