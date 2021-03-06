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

ArduRPC_Serial *rpc_serial;

SensorWifiModuleRemote *sensor_remote;

SensorNode *sensor_node;

uint8_t sensor_node_mode = NODE_MODE_ACTIVE;

void setup() {
  ArduRPC *rpc;
  ArduRPCRequest *rpc_request;

  pinMode(PIN_NODE_MODE, INPUT);

  pinMode(PIN_ESP8266_SET, OUTPUT);
  pinMode(PIN_ESP8266_MODE, OUTPUT);
  pinMode(PIN_ESP8266_CH_PD, OUTPUT);
  digitalWrite(PIN_ESP8266_CH_PD, LOW);

  RPC_SERIAL_PORT.begin(RPC_SERIAL_BAUD);

  if(digitalRead(PIN_NODE_MODE) == HIGH) {
    NODE_DEBUG_PRINTLN("Mode set to config");
    sensor_node_mode = NODE_MODE_CONFIG;
  }

  sensor_node = new SensorNode();
  if(sensor_node_mode == NODE_MODE_ACTIVE) {
    NODE_DEBUG_PRINTLN("load active mode");
    rpc_request = new ArduRPCRequest();
    new ArduRPCRequest_Serial(*rpc_request, RPC_SERIAL_PORT);
    sensor_remote = new SensorWifiModuleRemote(*rpc_request, 0x00, sensor_node);
    sensor_node->loadConfig();
  } else {
    rpc = new ArduRPC(RPC_NUM_HANDLERS, RPC_NUM_FUNCTIONS);
    rpc_serial = new ArduRPC_Serial(RPC_SERIAL_PORT, *rpc);

    new ArduRPC_SensorNode(*rpc, "node", sensor_node);
  }
}

void loop() {
  uint8_t i;
  uint8_t esp8266_ready = 0;

  NODE_DEBUG_PRINTLN("loop()");
  if(sensor_node_mode == NODE_MODE_ACTIVE) {
    NODE_DEBUG_PRINTLN("active begin");
    sensor_node->run();
    digitalWrite(PIN_ESP8266_CH_PD, HIGH);
    for(i = 0; i < 2; i++) {
      if(Serial.find("+READY")) {
        Serial.println("+MODE run");
        if(Serial.find("+OK")) {
          esp8266_ready = 1;
          break;
        }
      }
    }
    if(esp8266_ready) {
      sensor_node->submitValues(sensor_remote);
    }
    digitalWrite(PIN_ESP8266_CH_PD, LOW);
    // power down for 10s
    // ToDo: change value to 60s
    sensor_node->powerDown(10000);
    NODE_DEBUG_PRINTLN("active end");
  } else {
    while(1) {
      digitalWrite(PIN_ESP8266_CH_PD, HIGH);
      Serial.setTimeout(3000);
      for(i = 0; i < 2; i++) {
        if(Serial.find("+READY")) {
          Serial.println("+MODE cfg");
          if(Serial.find("+OK")) {
            esp8266_ready = 1;
            break;
          }
        }
      }
      if(esp8266_ready) {
        break;
      }
      // Restart ESP8266
      digitalWrite(PIN_ESP8266_CH_PD, LOW);
      delay(500);
    }
    rpc_serial->loop();
  }
}

