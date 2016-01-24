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

int8_t SensorNode::getSensorConfig(uint8_t sensor_id, uint8_t *result, uint8_t max_length)
{
  uint8_t i;
  uint8_t payload_length;
  uint16_t eeprom_pos;
  if (sensor_id >= NODE_MAX_SENSOR_COUNT) {
    return -1;
  }

  eeprom_pos = sensor_id;
  eeprom_pos *= (NODE_EEPROM_SENSOR_TYPE_SIZE + NODE_EEPROM_SENSOR_CONFIG_SIZE);
  eeprom_pos += NODE_EEPROM_BASIC_SENSOR_OFFSET;
  eeprom_pos += NODE_EEPROM_SENSOR_TYPE_SIZE;

  payload_length = EEPROM.read(eeprom_pos);
  eeprom_pos++;
  if (payload_length > NODE_EEPROM_SENSOR_CONFIG_PAYLOAD_SIZE) {
    payload_length = NODE_EEPROM_SENSOR_CONFIG_PAYLOAD_SIZE;
  }

  if (payload_length > max_length) {
    return -2;
  }

  for(i=0; i < payload_length; i++) {
    result[i] = EEPROM.read(eeprom_pos);
    eeprom_pos++;
  }
  return payload_length;
}

uint16_t SensorNode::getSensorType(uint8_t sensor_id)
{
  uint16_t eeprom_pos;
  uint16_t sensor_type;

  if (sensor_id >= NODE_MAX_SENSOR_COUNT) {
    return 1;
  }

  eeprom_pos = sensor_id;
  eeprom_pos *= (NODE_EEPROM_SENSOR_TYPE_SIZE + NODE_EEPROM_SENSOR_CONFIG_SIZE);
  eeprom_pos += NODE_EEPROM_BASIC_SENSOR_OFFSET;

  EEPROM.get(eeprom_pos, sensor_id);
  return sensor_id;
}

uint8_t SensorNode::getKey(char *key, uint8_t max_length)
{
  uint8_t i, length;
  uint16_t eeprom_pos;

  eeprom_pos = NODE_EEPROM_KEY_OFFSET;
  length = EEPROM.read(eeprom_pos);
  eeprom_pos++;

  if(length > max_length) {
    length = max_length;
  }

  for(i = 0; i < length; i++) {
    key[i] = EEPROM.read(eeprom_pos);
    eeprom_pos++;
  }
  return length;
}

uint8_t SensorNode::getUUID(char *uuid, uint8_t max_length)
{
  uint8_t i, length;
  uint16_t eeprom_pos;

  eeprom_pos = NODE_EEPROM_UUID_OFFSET;
  length = EEPROM.read(eeprom_pos);
  eeprom_pos++;

  if(length > max_length) {
    length = max_length;
  }

  for(i = 0; i < length; i++) {
    uuid[i] = EEPROM.read(eeprom_pos);
    eeprom_pos++;
  }
  return length;
}

void SensorNode::loadConfig()
{
  uint8_t i, j;
  uint16_t sensor_type;
  int pos = NODE_EEPROM_BASIC_SENSOR_OFFSET;
  BaseSensor *sensor = NULL;
  uint8_t config_payload[NODE_EEPROM_SENSOR_CONFIG_PAYLOAD_SIZE];
  int8_t payload_length;
  for(i = 0; i < NODE_MAX_SENSOR_COUNT; i++) {
    NODE_DEBUG_PRINT("Load sensor ");
    NODE_DEBUG_PRINTLN(i);
    sensor_type = this->getSensorType(i);
    NODE_DEBUG_PRINT("Sensor type ");
    NODE_DEBUG_PRINTLN(sensor_type);
    if(sensor_type == 0) {
      this->sensors[i] = NULL;
      continue;
    }

    payload_length = this->getSensorConfig(i, &config_payload[0], sizeof(config_payload));
    if(payload_length < 0) {
      continue;
    }

    sensor = this->setupSensor(sensor_type, config_payload);
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
  NODE_DEBUG_PRINTLN("start");
  sensor_remote->start();
  NODE_DEBUG_PRINTLN("getStatus");
  status = 0;
  for(i = 0; i < 10; i++) {
    status = sensor_remote->getStatus();
    NODE_DEBUG_PRINT("Status ");
    NODE_DEBUG_PRINTLN(status);
    if(status == 2) {
      break;
    }
    delay(1000);
  }
  if(status != 2) {
    return;
  }

  for(i = 0; i < this->value_count; i++) {
      NODE_DEBUG_PRINT("submitValue ");
      NODE_DEBUG_PRINTLN(i);
      sensor_remote->submitValue(&this->values[i * NODE_CACHE_VALUE_SIZE], NODE_CACHE_VALUE_SIZE);
  }
  NODE_DEBUG_PRINTLN("finish");
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

