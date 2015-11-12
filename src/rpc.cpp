#include "sensor_node.h"

ArduRPC_SensorNode::ArduRPC_SensorNode(ArduRPC &rpc, char *name) : ArduRPCHandler()
{
  this->type = 0xff01;
  this->registerSelf(rpc, name, (void *)this);
}

uint8_t ArduRPC_SensorNode::call(uint8_t cmd_id)
{
  uint8_t sensor_id;
  uint16_t eeprom_pos;
  uint8_t i, data;

  if (cmd_id == 0x09) {
    /* getMaxSensorCount() */
    this->_rpc->writeResult_uint8(NODE_MAX_SENSOR_COUNT);

    return RPC_RETURN_SUCCESS;
  } else if (cmd_id == 0x10) {
    /* getSensorType() */
    sensor_id = this->_rpc->getParam_uint8();
    if (sensor_id >= NODE_MAX_SENSOR_COUNT) {
      return 1;
    }

    eeprom_pos = sensor_id;
    eeprom_pos *= (NODE_EEPROM_SENSOR_TYPE_SIZE + NODE_EEPROM_SENSOR_CONFIG_SIZE);
    eeprom_pos += NODE_EEPROM_BASIC_SENSOR_OFFSET;

    i = NODE_EEPROM_SENSOR_TYPE_SIZE;
    this->_rpc->writeResult(RPC_UINT16);
    while(i--) {
      data = EEPROM.read(eeprom_pos);
      this->_rpc->writeResult(data);
      eeprom_pos++;
    }

    return RPC_RETURN_SUCCESS;
  } else if (cmd_id == 0x11) {
    /* getSensorConfig() */
    sensor_id = this->_rpc->getParam_uint8();
    if (sensor_id >= NODE_MAX_SENSOR_COUNT) {
      return 1;
    }

    eeprom_pos = sensor_id;
    eeprom_pos *= (NODE_EEPROM_SENSOR_TYPE_SIZE + NODE_EEPROM_SENSOR_CONFIG_SIZE);
    eeprom_pos += NODE_EEPROM_BASIC_SENSOR_OFFSET;
    eeprom_pos += NODE_EEPROM_SENSOR_TYPE_SIZE;

    this->_rpc->writeResult(RPC_ARRAY);
    this->_rpc->writeResult(RPC_UINT8);
    this->_rpc->writeResult(NODE_EEPROM_SENSOR_CONFIG_SIZE);

    i = NODE_EEPROM_SENSOR_CONFIG_SIZE;
    while(i--) {
      data = EEPROM.read(eeprom_pos);
      this->_rpc->writeResult(data);
      eeprom_pos++;
    }

    return RPC_RETURN_SUCCESS;
  } else if (cmd_id == 0x12) {
    /* setSensor() */
    sensor_id = this->_rpc->getParam_uint8();
    if (sensor_id >= NODE_MAX_SENSOR_COUNT) {
      return 1;
    }
    eeprom_pos = sensor_id;
    eeprom_pos *= (NODE_EEPROM_SENSOR_TYPE_SIZE + NODE_EEPROM_SENSOR_CONFIG_SIZE);
    eeprom_pos += NODE_EEPROM_BASIC_SENSOR_OFFSET;

    /* Write sensor type */
    i = NODE_EEPROM_SENSOR_TYPE_SIZE;
    while(i--) {
      data = this->_rpc->getParam_uint8();
      // ToDo: remove debug print()
      //EEPROM.update(eeprom_pos, data);
      Serial.print(i);
      Serial.print(" ");
      Serial.print(eeprom_pos);
      Serial.print(" ");
      Serial.println(data, HEX);
      eeprom_pos++;
    }

    /* Write sensor options */
    i = this->_rpc->getParam_uint8();
    if (i > NODE_EEPROM_SENSOR_CONFIG_SIZE) {
      i = NODE_EEPROM_SENSOR_CONFIG_SIZE;
    }
    while(i--) {
      data = this->_rpc->getParam_uint8();
      // ToDo: remove debug print()
      //EEPROM.update(eeprom_pos, data);
      Serial.print(i);
      Serial.print(" ");
      Serial.print(eeprom_pos);
      Serial.print(" ");
      Serial.println(data, HEX);
      eeprom_pos++;
    }
    return RPC_RETURN_SUCCESS;
  }
  return RPC_RETURN_COMMAND_NOT_FOUND;
}

