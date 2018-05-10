#include <App.hpp>
#include "Pzem004Tnb.hpp"

#define PZEM_VOLTAGE (uint8_t)0xB0
#define RESP_VOLTAGE (uint8_t)0xA0

#define PZEM_CURRENT (uint8_t)0xB1
#define RESP_CURRENT (uint8_t)0xA1

#define PZEM_POWER   (uint8_t)0xB2
#define RESP_POWER   (uint8_t)0xA2

#define PZEM_ENERGY  (uint8_t)0xB3
#define RESP_ENERGY  (uint8_t)0xA3

#define PZEM_SET_ADDRESS (uint8_t)0xB4
#define RESP_SET_ADDRESS (uint8_t)0xA4

#define PZEM_POWER_ALARM (uint8_t)0xB5
#define RESP_POWER_ALARM (uint8_t)0xA5

#define PZEM_BAUD_RATE 9600
#define PZEM_DEFAULT_READ_TIMEOUT 3000

#define PZEM_ERROR_VALUE -1.0

IPAddress ip(192,168,1,1);
Pzem004Tnb pzem( ip, D5, D6 );  // (RX,TX) connect to TX,RX of PZEM

Pzem004Tnb::Pzem004Tnb(const IPAddress &ipAddr, const uint8_t receivePin, const uint8_t transmitPin)
{
  SoftwareSerial *port = new SoftwareSerial(receivePin, transmitPin);
  port->begin(PZEM_BAUD_RATE);
  this->serial = port;

  ipAddress = ipAddr;
  this->readTimeOut = PZEM_DEFAULT_READ_TIMEOUT;
  this->serialIsSoft = true;
  handlerState = SEND_U;
}

bool Pzem004Tnb::initialize()
{
  errorCounter = lastErrorTimestamp = validReadTimestamp = 0;
  send(PZEM_SET_ADDRESS);
  receive(RESP_SET_ADDRESS);
  // don't know why but if i send it twice the initialization is allways correct
  // otherwise it is sometimes false
  send(PZEM_SET_ADDRESS);
  return receive(RESP_SET_ADDRESS);
}

bool Pzem004Tnb::nbReceive()
{
  if(serial->available() > 0)
  {
    uint8_t c = (uint8_t)serial->read();

    if(!c && !bufferIndex)
      return false;         // skip 0 at startup

    buffer[bufferIndex++] = c;
    handlerTimeStamp = millis();
    // Serial.printf( "--> %d %d %02x\n", bufferIndex, RESPONSE_SIZE, c );
  }

  return bufferIndex == RESPONSE_SIZE;
}

bool Pzem004Tnb::checkData(uint8_t resp)
{
  if(bufferIndex != RESPONSE_SIZE)
  {
    LOG0( "*** PZEM004 ERROR incorrect response size\n" );
    handlerState = SEND_U;
    errorCounter++;
    lastErrorTimestamp = app.secTimestamp();
    return false;
  }

  if(buffer[6] != crc(buffer, bufferIndex - 1))
  {
    LOG0( "*** PZEM004 ERROR crc failure\n" );
    errorCounter++;
    lastErrorTimestamp = app.secTimestamp();
    handlerState = SEND_U;
    return false;
  }

  if(buffer[0] != resp)
  {
    LOG0( "*** PZEM004 ERROR incorrect respose\n" );
    errorCounter++;
    lastErrorTimestamp = app.secTimestamp();
    handlerState = SEND_U;
    return false;
  }

  if(data)
  {
    for(unsigned int i=0; i<RESPONSE_DATA_SIZE; i++)
      data[i] = buffer[1 + i];
  }

  return true;
}

void Pzem004Tnb::handler()
{

  switch( handlerState )
  {
  case SEND_U:
    // LOG0("SEND_U\n");
    handlerTimeStamp = millis();
    send(PZEM_VOLTAGE);
    handlerState = RECEIVE_U;
    break;

  case RECEIVE_U:

    if ( nbReceive() && checkData(RESP_VOLTAGE) )
    {
      // LOG0("RECEIVE_U\n");
      pzemU = (data[0] << 8) + data[1] + (data[2] / 10.0);
      handlerState = SEND_I;
      handlerTimeStamp = millis();
    }

    break;

  case SEND_I:
    // LOG0("SEND_I\n");
    handlerTimeStamp = millis();
    send(PZEM_CURRENT);
    handlerState = RECEIVE_I;
    break;

  case RECEIVE_I:

    if ( nbReceive() && checkData(RESP_CURRENT) )
    {
      // LOG0("RECEIVE_I\n");
      pzemI = (data[0] << 8) + data[1] + (data[2] / 100.0);
      handlerState = ALLREAD_EVENT;
      eventTimeStamp = handlerTimeStamp = millis();
    }

    break;

  case ALLREAD_EVENT:
    if ( abs(millis() - eventTimeStamp) > 1000 )
    {
      LOG0( "PZEM004 INFO - all values read - event\n" );
      handlerTimeStamp = millis();
      validReadTimestamp = app.secTimestamp();
      handlerState = SEND_U;
    }

    break;
  }

  if( abs(millis() - handlerTimeStamp) > readTimeOut )
  {
    LOG0( "*** PZEM004 ERROR - read timeout\n" );
    errorCounter++;
    lastErrorTimestamp = app.secTimestamp();
    handlerState = SEND_U;
  }
}

float Pzem004Tnb::getU()
{
  return pzemU;
}

float Pzem004Tnb::getI()
{
  return pzemI;
}

float Pzem004Tnb::getP()
{
  return pzemU * pzemI;
}

void Pzem004Tnb::send( uint8_t cmd, uint8_t data )
{
  pzemCommand pzem;

  pzem.command = cmd;

  for(unsigned int i=0; i<sizeof(pzem.addr); i++)
    pzem.addr[i] = ipAddress[i];

  pzem.data = data;

  uint8_t *bytes = (uint8_t*)&pzem;
  pzem.crc = crc(bytes, sizeof(pzem) - 1);

  while(serial->available())
    serial->read();

  serial->write(bytes, sizeof(pzem));
  bufferIndex = 0;
}

bool Pzem004Tnb::receive(uint8_t resp, uint8_t *data)
{
  if(serialIsSoft)
    ((SoftwareSerial *)serial)->listen();

  unsigned long startTime = millis();
  uint8_t len = 0;

  while((len < RESPONSE_SIZE) && (millis() - startTime < readTimeOut))
  {
    if(serial->available() > 0)
    {
      uint8_t c = (uint8_t)serial->read();

      if(!c && !len)
        continue;         // skip 0 at startup

      buffer[len++] = c;
    }

    yield();            // do background netw tasks while blocked for IO (prevents ESP watchdog trigger)
  }

  if(len != RESPONSE_SIZE)
    return false;

  if(buffer[6] != crc(buffer, len - 1))
    return false;

  if(buffer[0] != resp)
    return false;

  if(data)
  {
    for(unsigned int i=0; i<RESPONSE_DATA_SIZE; i++)
      data[i] = buffer[1 + i];
  }

  return true;
}

uint8_t Pzem004Tnb::crc(uint8_t *data, uint8_t sz)
{
  uint16_t crc = 0;

  for(uint8_t i=0; i<sz; i++)
    crc += *data++;

  return (uint8_t)(crc & 0xFF);
}
