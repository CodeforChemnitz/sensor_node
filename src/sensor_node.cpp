#include "sensor_node.h"

uint8_t EEPROM_read(int pos)
{
  pos = pos - NODE_EEPROM_BASIC_SENSOR_OFFSET;
  if (pos == 0)
    return 0;
  if (pos == 1)
    return 1;
  if (pos == 2)
    return 6;
  return 0;
}

SensorNode::SensorNode()
{
}

BaseSensor *SensorNode::setupSensor(uint16_t type, uint8_t *options)
{
  if(type == 0x01) {
    return (BaseSensor *)new SensorDHT(options);
  }
  return NULL;
}

void SensorNode::loadConfig()
{
  uint8_t i, j;
  uint16_t sensor_type;
  int pos = NODE_EEPROM_BASIC_SENSOR_OFFSET;
  BaseSensor *sensor = NULL;
  uint8_t sensor_options[8];

  for(i = 0; i < NODE_MAX_SENSOR_COUNT; i++) {
    sensor_type = 0;
    sensor_type = (uint8_t)EEPROM_read(pos);
    pos++;
    sensor_type <<= 8;
    sensor_type |= (uint8_t)EEPROM_read(pos);
    pos++;
    if(sensor_type == 0) {
      pos += 8;
      continue;
    }
    
    for(j = 0; j < 8; j++) {
      sensor_options[j] = EEPROM_read(pos);
      pos++;
    }
    sensor = this->setupSensor(sensor_type, sensor_options);
    if (sensor != NULL) {
      this->sensors[i] = sensor;
    }
  }
}

void SensorNode::resetValues()
{
  this->value_count = 0;
}

void SensorNode::run()
{
  uint8_t i;
  uint16_t sensor_type;
  BaseSensor *sensor;

  for(i = 0; i < NODE_MAX_SENSOR_COUNT; i++) {
    sensor = this->sensors[i];
    if (sensor != NULL) {
      sensor->start(this);
    }
  }

  for(i = 0; i < NODE_MAX_SENSOR_COUNT; i++) {
    sensor = this->sensors[i];
    if (sensor != NULL) {
      sensor->run(this);
    }
  }

  int pos = NODE_EEPROM_BASIC_SENSOR_OFFSET;
  
  this->resetValues();
  for(i = 0; i < NODE_MAX_SENSOR_COUNT; i++) {
    sensor = this->sensors[i];
    if (sensor == NULL) {
      pos += 10;
    } else {
      sensor_type = 0;
      sensor_type = (uint8_t)EEPROM_read(pos);
      pos++;
      sensor_type <<= 8;
      sensor_type |= (uint8_t)EEPROM_read(pos);
      pos += 9;

      this->current_sensor_id = i;
      this->current_sensor_type = sensor_type;
      sensor->finish(this);
    }
  }
}

void SensorNode::submitValues(SensorWifiModuleRemote *sensor_remote)
{
  uint8_t i, status;
  Serial.println("start");
  sensor_remote->start();
  Serial.println("getStatus");
  status = 0;
  for(i = 0; i < 10; i++) {
    status = sensor_remote->getStatus();
    Serial.print("Status ");
    Serial.println(status);
    if(status == 2) {
      break;
    }
    delay(1000);
  }
  if(status != 2) {
    return;
  }

  for(i = 0; i < this->value_count; i++) {
      Serial.print("submitValue");
      Serial.println(i);
      sensor_remote->submitValue(&this->values[i * NODE_CACHE_VALUE_SIZE], NODE_CACHE_VALUE_SIZE);
  }
  Serial.println("finish");
  sensor_remote->finish();
  // ToDo: get status
  delay(3000);
}

uint8_t *SensorNode::writeValuePrefix(uint8_t type)
{
  uint8_t *data = &this->values[this->value_count * NODE_CACHE_VALUE_SIZE];
  
  data[0] = this->current_sensor_id;
  data[1] = (((this->current_sensor_type) >> 8) & 0xff);
  data[2] = ((this->current_sensor_type) & 0xff);
  data[3] = type;
  data = data + 4;
  return data;
}

void SensorNode::writeValue(uint8_t type, float value)
{
  uint8_t *data;
  uint8_t *v = (uint8_t *)&value;
  data = this->writeValuePrefix(type);

  data[0] = RPC_FLOAT;
  data[1] = v[3];
  data[2] = v[2];
  data[3] = v[1];
  data[4] = v[0];

  this->value_count++;
}

BaseSensor::BaseSensor()
{
}

SensorDHT::SensorDHT(uint8_t *options)
{
  this->dht = new DHT(options[0], DHT22);
}


void SensorDHT::start(SensorNode *sensor)
{
  this->dht->readTemperature();
  this->dht->readHumidity();
}

void SensorDHT::run(SensorNode *sensor)
{
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

SensorWifiModuleRemote::SensorWifiModuleRemote(ArduRPCRequest &rpc, uint8_t handler_id) : ArduRPCRequestHandler()
{
  this->_rpc = &rpc;
  this->_handler_id = handler_id;
}

uint8_t SensorWifiModuleRemote::getStatus()
{
  uint8_t status = 0;

  this->_rpc->reset();
  Serial.println("call");
  //this->_rpc->writeRequest_uint8(i);
  this->_rpc->call(this->_handler_id, 0x12);
  
  status = this->_rpc->readResult_uint8();
  Serial.println("end");
  return status;
}

uint8_t SensorWifiModuleRemote::finish()
{
  this->_rpc->reset();
  Serial.println("call");
  //this->_rpc->writeRequest_uint8(i);
  this->_rpc->call(this->_handler_id, 0x11);
  Serial.println("end");
  return 0;
}

uint8_t SensorWifiModuleRemote::start()
{
  this->_rpc->reset();
  Serial.println("call");
  //this->_rpc->writeRequest_uint8(i);
  this->_rpc->call(this->_handler_id, 0x10);
  Serial.println("end");
  return 0;
}

uint8_t SensorWifiModuleRemote::submitValue(uint8_t *data, uint8_t length)
{
  uint8_t i;
  this->_rpc->reset();
  Serial.println("call");
  for(i = 0; i < length; i++) {
    this->_rpc->writeRequest_uint8(data[i]);
  }
  this->_rpc->call(this->_handler_id, 0x13);
  Serial.println("end");
  return 0;
}

