#ifndef _SENSOR_NODE_H
#define _SENSOR_NODE_H

#include <avr/pgmspace.h>

#include "ArduRPC.h"

#include "DHT.h"

#if (ARDUINO >= 100)
 #include <Arduino.h>
 #include <EEPROM.h>
#else
#endif

#define NODE_EEPROM_UUID_OFFSET 0
#define NODE_EEPROM_API_KEY_OFFSET 32
#define NODE_EEPROM_BASIC_SENSOR_OFFSET 64

#define NODE_MAX_SENSOR_COUNT  32
#define NODE_CACHE_VALUE_SIZE  9

#define SENSOR_VALUE_TEMPERATURE 1
#define SENSOR_VALUE_HUMIDITY    2

class SensorNode;

class BaseSensor
{
  public:
    BaseSensor();
    virtual void
      finish(SensorNode *),
      run(SensorNode *),
      start(SensorNode *);
  private:
};

class SensorWifiModuleRemote : public ArduRPCRequestHandler
{
  public:
    SensorWifiModuleRemote(ArduRPCRequest &rpc, uint8_t handler_id);

    uint8_t
      finish(),
      submitValue(uint8_t *, uint8_t),
      start();
    uint8_t
      getStatus();
  private:
};

class SensorNode
{
  public:
    SensorNode();
    void
      loadConfig(),
      run(),
      submitValues(SensorWifiModuleRemote *),
      writeValue(uint8_t, float);
    BaseSensor
      *setupSensor(uint16_t, uint8_t *);
  private:
    uint8_t *writeValuePrefix(uint8_t);
    void resetValues();
    BaseSensor *sensors[NODE_MAX_SENSOR_COUNT];
    uint16_t current_sensor_type;
    uint8_t current_sensor_id;
    uint8_t values[NODE_MAX_SENSOR_COUNT * 9];
    uint8_t value_count;
};


class SensorDHT : public BaseSensor
{
  public:
    SensorDHT(uint8_t *);
    void
      finish(SensorNode *),
      run(SensorNode *),
      start(SensorNode *);
  private:
    DHT *dht;
};

class ArduRPC_SensorNode : public ArduRPCHandler
{
  public:
    ArduRPC_SensorNode(ArduRPC &rpc, char *name);
    uint8_t
      call(uint8_t);
};

#endif
