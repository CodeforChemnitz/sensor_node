#include <ArduRPC.h>
#include <SoftwareSerial.h>
#include "sensor_node.h"
#include "DHT.h"

#define RPC_SERIAL_PORT Serial
#define RPC_SERIAL_BAUD 9600

#define RPC_NUM_HANDLERS 20
#define RPC_NUM_FUNCTIONS 0

#define PIN_NODE_MODE     4

#define NODE_MODE_ACTIVE 0
#define NODE_MODE_CONFIG 1

#define PIN_ESP8266_SET   2
#define PIN_ESP8266_MODE  3
#define PIN_ESP8266_CH_PD 7

SoftwareSerial mySerial(10, 11);

//ArduRPC rpc = ArduRPC(RPC_NUM_HANDLERS, RPC_NUM_FUNCTIONS);
//ArduRPC_Serial rpc_serial = ArduRPC_Serial(RPC_SERIAL_PORT, rpc);

//ArduRPC_SensorNode sensor_node = ArduRPC_SensorNode();

ArduRPCRequest rpc_request = ArduRPCRequest();
ArduRPCRequest_Serial h = ArduRPCRequest_Serial(rpc_request, RPC_SERIAL_PORT);
SensorWifiModuleRemote sensor_remote = SensorWifiModuleRemote(rpc_request, 0x00);

SensorNode sensor_node = SensorNode();

uint8_t sensor_node_mode = NODE_MODE_ACTIVE;

void setup() {
  pinMode(PIN_NODE_MODE, INPUT);

  pinMode(PIN_ESP8266_SET, OUTPUT);
  pinMode(PIN_ESP8266_MODE, OUTPUT);
  pinMode(PIN_ESP8266_CH_PD, OUTPUT);
  digitalWrite(PIN_ESP8266_CH_PD, LOW);

  RPC_SERIAL_PORT.begin(RPC_SERIAL_BAUD);

  if(digitalRead(PIN_NODE_MODE) == HIGH) {
    sensor_node_mode = NODE_MODE_CONFIG;
  }

  if(sensor_node_mode == NODE_MODE_ACTIVE) {
    sensor_node.loadConfig();
  } else {
  }
  /*sensor_dht = new DHT(6, DHT22);
  sensor_dht->begin();*/
  //mySerial.begin(9600);
  //rpc.connectHandler(&sensor_node, 0x10);
  // put your setup code here, to run once:
  Serial.println("foo");
}

void loop() {
  //a->writeValue();
  //rpc_serial.readData();
  //sensor_dht->readTemperature();
  if(sensor_node_mode == NODE_MODE_ACTIVE) {
    sensor_node.run();
    digitalWrite(PIN_ESP8266_SET, HIGH);
    digitalWrite(PIN_ESP8266_MODE, HIGH);

    digitalWrite(PIN_ESP8266_CH_PD, HIGH);
    // Wait for bootloader
    delay(250);
    digitalWrite(PIN_ESP8266_MODE, LOW);
    digitalWrite(PIN_ESP8266_SET, LOW);
    // Wait until boot
    delay(250);
    sensor_node.submitValues(&sensor_remote);
    digitalWrite(PIN_ESP8266_CH_PD, LOW);
    delay(5000);
  } else {
  }
}
