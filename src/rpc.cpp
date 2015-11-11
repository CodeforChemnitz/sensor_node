#include "sensor_node.h"

ArduRPC_SensorNode::ArduRPC_SensorNode(ArduRPC &rpc, char *name) : ArduRPCHandler()
{
  this->type = 0x9999;
  this->registerSelf(rpc, name, (void *)this);
}

uint8_t ArduRPC_SensorNode::call(uint8_t cmd_id)
{
  if (cmd_id == 0x10) {
    return RPC_RETURN_SUCCESS;
  }
  return RPC_RETURN_COMMAND_NOT_FOUND;
}

