#include "../sensor_node.h"

SensorWifiModuleRemote::SensorWifiModuleRemote(ArduRPCRequest &rpc, uint8_t handler_id, SensorNode *node) : ArduRPCRequestHandler()
{
  this->_rpc = &rpc;
  this->_handler_id = handler_id;
  this->node = node;
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
  uint8_t length;
  char data[64];
  this->_rpc->reset();
  Serial.println("call");
  length = this->node->getUUID(&data[0], sizeof(data));
  // ToDo:
  data[length] = '\0';
  this->_rpc->writeRequest_string(data);
  length = this->node->getKey(&data[0], sizeof(data));
  // ToDo:
  data[length] = '\0';
  this->_rpc->writeRequest_string(data);

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
