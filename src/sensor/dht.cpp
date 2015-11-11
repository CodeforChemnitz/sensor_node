#include "../sensor_node.h"

SensorDHT::SensorDHT(uint8_t *options)
{
  this->dht = new DHT(options[0], DHT22);
}

void SensorDHT::finish(SensorNode *sensor)
{
  float value;
  value = this->dht->readTemperature();
  sensor->writeValue(
    SENSOR_VALUE_TEMPERATURE,
    value
  );

  value = this->dht->readHumidity();
  sensor->writeValue(
    SENSOR_VALUE_HUMIDITY,
    value
  );
}

void SensorDHT::start(SensorNode *sensor)
{
  this->dht->readTemperature();
  this->dht->readHumidity();
}

