#include <ArduRPC.h>
#include <SoftwareSerial.h>
#include "sensor_node.h"
#include "DHT.h"

#define RPC_SERIAL_PORT Serial
#define RPC_SERIAL_BAUD 9600

#define RPC_NUM_HANDLERS 20
#define RPC_NUM_FUNCTIONS 0

SoftwareSerial mySerial(10, 11);


//ArduRPC rpc = ArduRPC(RPC_NUM_HANDLERS, RPC_NUM_FUNCTIONS);
//ArduRPC_Serial rpc_serial = ArduRPC_Serial(RPC_SERIAL_PORT, rpc);

//ArduRPC_SensorNode sensor_node = ArduRPC_SensorNode();

ArduRPCRequest rpc_request = ArduRPCRequest();
ArduRPCRequest_Serial h = ArduRPCRequest_Serial(rpc_request, RPC_SERIAL_PORT);
SensorWifiModuleRemote sensor_remote = SensorWifiModuleRemote(rpc_request, 0x10);

SensorNode sensor_node = SensorNode();

void setup() {
  RPC_SERIAL_PORT.begin(RPC_SERIAL_BAUD);
  /*sensor_dht = new DHT(6, DHT22);
  sensor_dht->begin();*/
  //mySerial.begin(9600);
  //rpc.connectHandler(&sensor_node, 0x10);
  // put your setup code here, to run once:
  sensor_node.loadConfig();
  Serial.println("foo");
}

void loop() {
  //a->writeValue();
  //rpc_serial.readData();
  //sensor_dht->readTemperature();
  sensor_node.run();
  sensor_node.submitValues(&sensor_remote);
  delay(5000);
}
