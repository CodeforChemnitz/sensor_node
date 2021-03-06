#include "sensor_node.h"

ArduRPC_SensorNode::ArduRPC_SensorNode(ArduRPC &rpc, char *name, SensorNode *node) : ArduRPCHandler()
{
  this->type = 0xff01;
  this->registerSelf(rpc, name, (void *)this);
  this->node = node;
}

uint8_t ArduRPC_SensorNode::call(uint8_t cmd_id)
{
  uint8_t sensor_id;
  uint16_t eeprom_pos;
  uint8_t i, data;
  char s[64] = {0};
  uint8_t len;
  int8_t payload_length;
  uint8_t config_payload[NODE_EEPROM_SENSOR_CONFIG_PAYLOAD_SIZE];
  uint16_t sensor_type;


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

    sensor_type = this->node->getSensorType(sensor_id);
    this->_rpc->writeResult_uint16(sensor_type);

    return RPC_RETURN_SUCCESS;
  } else if (cmd_id == 0x11) {
    /* getSensorConfig() */
    sensor_id = this->_rpc->getParam_uint8();

    payload_length = this->node->getSensorConfig(sensor_id, &config_payload[0], NODE_EEPROM_SENSOR_CONFIG_PAYLOAD_SIZE);
    // Return error code
    if (payload_length < 0) {
      return abs(payload_length);
    }

    this->_rpc->writeResult(RPC_ARRAY);
    this->_rpc->writeResult(RPC_UINT8);

    this->_rpc->writeResult(payload_length);

    for(i=0; i < payload_length; i++) {
      this->_rpc->writeResult(config_payload[i]);
    }

    return RPC_RETURN_SUCCESS;
  } else if (cmd_id == 0x12) {
    /* setSensor() */
    sensor_id = this->_rpc->getParam_uint8();
    NODE_DEBUG_PRINT("sensor_id ");
    NODE_DEBUG_PRINTLN(sensor_id);
    if (sensor_id >= NODE_MAX_SENSOR_COUNT) {
      return 1;
    }
    eeprom_pos = sensor_id;
    eeprom_pos *= (NODE_EEPROM_SENSOR_TYPE_SIZE + NODE_EEPROM_SENSOR_CONFIG_SIZE);
    eeprom_pos += NODE_EEPROM_BASIC_SENSOR_OFFSET;

    /* Write sensor type */
    sensor_type = 0;
    data = this->_rpc->getParam_uint8();
    sensor_type <<8;
    data = this->_rpc->getParam_uint8();
    sensor_type |= data;
    EEPROM.put(eeprom_pos, sensor_type);
    eeprom_pos += 2;

    /* Write sensor options length */
    i = this->_rpc->getParam_uint8();
    NODE_DEBUG_PRINT("payload_len ");
    NODE_DEBUG_PRINTLN(i);
    if (i > NODE_EEPROM_SENSOR_CONFIG_PAYLOAD_SIZE) {
      i = NODE_EEPROM_SENSOR_CONFIG_PAYLOAD_SIZE;
    }
    EEPROM.update(eeprom_pos, i);
    eeprom_pos++;

    /* Write sensor options */
    while(i--) {
      data = this->_rpc->getParam_uint8();
      NODE_DEBUG_PRINT("data ");
      NODE_DEBUG_PRINT(data);
      NODE_DEBUG_PRINT(" ");
      NODE_DEBUG_PRINTLN(eeprom_pos);
      EEPROM.update(eeprom_pos, data);
      eeprom_pos++;
    }
    return RPC_RETURN_SUCCESS;
  } else if (cmd_id == 0x21) {
    /* getUUID() */
    len = this->node->getUUID(&s[0], sizeof(s));
    this->_rpc->writeResult_string(s, len);

    return RPC_RETURN_SUCCESS;
  } else if (cmd_id == 0x22) {
    /* getKey() */
    len = this->node->getKey(&s[0], sizeof(s));
    this->_rpc->writeResult_string(s, len);

    return RPC_RETURN_SUCCESS;
  } else if (cmd_id == 0x23) {
    /* setCredentials() */

    /* UUID */
    len = this->_rpc->getParam_string(s, 64);

    eeprom_pos = NODE_EEPROM_UUID_OFFSET;
    EEPROM.update(NODE_EEPROM_UUID_OFFSET, len);
    eeprom_pos++;

    for(i = 0; i < len; i++) {
      EEPROM.update(eeprom_pos, s[i]);
      eeprom_pos++;
    }

    /* API Key */
    len = this->_rpc->getParam_string(s, 64);

    eeprom_pos = NODE_EEPROM_KEY_OFFSET;
    EEPROM.update(NODE_EEPROM_KEY_OFFSET, len);
    eeprom_pos++;

    for(i = 0; i < len; i++) {
      EEPROM.update(eeprom_pos, s[i]);
      eeprom_pos++;
    }
    return RPC_RETURN_SUCCESS;
  }
  return RPC_RETURN_COMMAND_NOT_FOUND;
}

