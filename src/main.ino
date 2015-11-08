#include <ArduRPC.h>
#include <SoftwareSerial.h>
#include "sensor_node.h"
#include "DHT.h"

#define RPC_SERIAL_PORT Serial
#define RPC_SERIAL_BAUD 9600

#define RPC_NUM_HANDLERS  2
#define RPC_NUM_FUNCTIONS 0

#define PIN_NODE_MODE     4

#define NODE_MODE_ACTIVE 0
#define NODE_MODE_CONFIG 1

#define PIN_ESP8266_SET   2
#define PIN_ESP8266_MODE  3
#define PIN_ESP8266_CH_PD 7

SoftwareSerial mySerial(10, 11);

ArduRPC *rpc;
ArduRPC_Serial *rpc_serial;

ArduRPC_SensorNode *rpc_sensor_node;

ArduRPCRequest *rpc_request;
ArduRPCRequest_Serial *h;
SensorWifiModuleRemote *sensor_remote;

SensorNode *sensor_node;

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
    rpc_request = new ArduRPCRequest();
    h = new ArduRPCRequest_Serial(*rpc_request, RPC_SERIAL_PORT);
    sensor_remote = new SensorWifiModuleRemote(*rpc_request, 0x00);
    sensor_node = new SensorNode();
    sensor_node->loadConfig();
  } else {
    rpc = new ArduRPC(RPC_NUM_HANDLERS, RPC_NUM_FUNCTIONS);
    rpc_serial = new ArduRPC_Serial(RPC_SERIAL_PORT, *rpc);

    rpc_sensor_node = new ArduRPC_SensorNode(*rpc, "");
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
    sensor_node->run();
    digitalWrite(PIN_ESP8266_SET, HIGH);
    digitalWrite(PIN_ESP8266_MODE, HIGH);

    digitalWrite(PIN_ESP8266_CH_PD, HIGH);
    // Wait for bootloader
    delay(250);
    digitalWrite(PIN_ESP8266_MODE, LOW);
    digitalWrite(PIN_ESP8266_SET, LOW);
    // Wait until boot
    delay(250);
    sensor_node->submitValues(sensor_remote);
    digitalWrite(PIN_ESP8266_CH_PD, LOW);
    delay(5000);
  } else {
    digitalWrite(PIN_ESP8266_SET, HIGH);
    digitalWrite(PIN_ESP8266_MODE, HIGH);

    digitalWrite(PIN_ESP8266_CH_PD, HIGH);
    // Wait for bootloader
    delay(250);
    digitalWrite(PIN_ESP8266_SET, LOW);
    rpc_serial->loop();
  }
}
