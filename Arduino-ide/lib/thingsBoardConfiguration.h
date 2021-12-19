#ifndef _thingsBoardConfiguration_
#define _thingsBoardConfiguration_

#include <ThingsBoard.h> // ThingsBoard SDK
#include <ESP8266WiFi.h>
#include "Agrumino.h"
#include "../lib/wifiConfiguration.h"

#define TOKEN "A1_TEST_TOKEN" // <-- MUST BE CHANGED

// Baud rate for debug serial
#define SERIAL_DEBUG_BAUD 115200

#ifndef COUNT_OF
// Helper macro to calculate array size
#define COUNT_OF(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))
#endif

// See https://thingsboard.io/docs/getting-started-guides/helloworld/
// to understand how to obtain an access token
// #define TOKEN "A1_TEST_TOKEN"

// ThingsBoard server instance.
#define THINGSBOARD_SERVER "192.168.12.1"

extern WiFiClient espClient;

// // Initialize ThingsBoard client
// WiFiClient espClient;
// Initialize ThingsBoard instance
ThingsBoard tb(espClient);

// // the Wifi radio's status
// int status = WL_IDLE_STATUS;

// Initial period of LED cycling.
int led_delay = 1000;
// Period of sending a temperature/humidity data.
int send_delay = 5000;

// Time passed after LED was turned ON, milliseconds.
int led_passed = 0;
// Time passed after temperature/humidity data was sent, milliseconds.
int send_passed = 0;

// Set to true if application is subscribed for the RPC messages.
bool subscribed = false;

extern Agrumino agrumino;
extern WiFiClient espClient;

String get_led_status()
{
  // Prepare gpios JSON payload string
  //  StaticJsonBuffer<200> jsonBuffer;
  // JsonObject& data = jsonBuffer.createObject();
  // data[String(GPIO0_PIN)] = gpioState[0] ? true : false;
  // data[String(GPIO2_PIN)] = gpioState[1] ? true : false;
  // char payload[256];
  // data.printTo(payload, sizeof(payload));
  // String strPayload = String(agrumino.isLedOn()); //String(payload);

  DynamicJsonDocument doc(1024);

  // create a variant
  JsonVariant variant = doc.to<JsonVariant>();
  variant.set(agrumino.isLedOn());

  String ledValue;
  // serialize the object and send the result to Serial
  serializeJson(doc, ledValue);
  String strPayload = "{\"ledValue\":" + ledValue + "}";
  Serial.print("Get led status: ");
  Serial.println(strPayload);
  return strPayload;
}

void set_gpio_status(int pin, boolean enabled)
{
  if (enabled)
  {
    agrumino.turnLedOn();
  }
  else
  {
    agrumino.turnLedOff();
  }
}

// Processes function for RPC call "setValue"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response setLedStatus(const RPC_Data &data)
{
  Serial.println("Received the set value RPC method");

  // Process data

  boolean newStatus = data.as<bool>();

  Serial.print("Set new status: ");
  Serial.println(newStatus);
  if (newStatus)
  {
    agrumino.turnLedOn();
  }
  else
  {
    agrumino.turnLedOff();
  }

  tb.sendAttributeBool("ledStatus", agrumino.isLedOn());
  return RPC_Response(NULL, newStatus);
}

// Processes function for RPC call "getValue"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response getLedStatus(const RPC_Data &data)
{
  Serial.println("Received the get value RPC method");
  // Serial.println("The value to return is: " + String(agrumino.isLedOn()));
  //String value = "{\"value\":" + String(agrumino.isLedOn()) + "}";
  tb.sendAttributeBool("ledStatus", agrumino.isLedOn());
  return RPC_Response(NULL, "0");
}

void sendAgruminoDataToTb(AgruminoData &agruminoData)
{
  // Prepare a JSON payload string
  String payload = "{";
  payload += "\"temperature\":";
  payload += agruminoData.temperature;
  payload += ",";
  payload += "\"soilMoisture\":";
  payload += agruminoData.soilMoisture;
  payload += ",";
  payload += "\"illuminance\":";
  payload += agruminoData.illuminance;
  payload += ",";
  payload += "\"ledStatus\":";
  payload += agruminoData.isLedOn;
  payload += ",";
  payload += "\"ledColor\":";
  payload += agruminoData.ledColor;
  // payload += ",";
  // payload += "\"batteryVoltage\":";
  // payload += agruminoData.batteryVoltage;
  // payload += ",";
  // payload += "\"batteryLevel\":";
  // payload += agruminoData.batteryLevel;
  // payload += ",";
  // payload += "\"isAttachedToUSB\":";
  // payload += agruminoData.isAttachedToUSB;
  // payload += ",";
  // payload += "\"isBatteryCharging\":";
  // payload += agruminoData.isBatteryCharging;
  // payload += ",";

  payload += "}";

  Serial.println("Payload: " + payload);
  // Serial.println("Payload length: " + payload.length());

  Serial.println("Sending data to MqttBbroker...");

  String clientName = getDeviceMqttClientName();
  Serial.println("... from client: " + clientName);

  String topic = MQTT_DEVICE_TELEMETRY_TOPIC; //"v1/devices/" + clientName + "/telemetry";
  topic.replace("CLIENT_NAME", clientName);
  Serial.println("... to topic: " + topic);

  char topicBuffer[200];
  topic.toCharArray(topicBuffer, topic.length() + 1);

  char payloadBuffer[200];
  payload.toCharArray(payloadBuffer, payload.length() + 1);

  bool status = mqttClient.publish(topicBuffer, payloadBuffer);
  Serial.println("status: " + String(status));
}

// RPC handlers
RPC_Callback callbacks[] = {
    {"setLedStatus", setLedStatus},
    {"getLedStatus", getLedStatus}
};


void connectThingsBoard()
{
  Serial.println("Connecting to Thingsboard node ...");
  // Attempt to connect (clientId, username, password)
  if (tb.connect(THINGSBOARD_SERVER, TOKEN))
  // if (mqttClient.connect("AgruminoGc device"))
  {
    Serial.println("ThingsBoard connection [DONE]");
    // Subscribing to receive RPC requests
    // mqttClient.subscribe("v1/devices/me/rpc/request/+");
    // Sending current GPIO status
    Serial.println("Sending current LED status ...");
    // mqttClient.publish("v1/devices/me/attributes", get_led_status().c_str());
    tb.sendAttributeBool("ledStatus", agrumino.isLedOn());

    // Perform a subscription. All consequent data processing will happen in
    // callbacks as denoted by callbacks[] array.
    if (!tb.RPC_Subscribe(callbacks, COUNT_OF(callbacks)))
    {
      Serial.println("Failed to subscribe for RPC");
      return;
    }
    else
    {
      Serial.println("Successed to subscribe for RPC");
      return;
    }
  }
  else
  {
    Serial.print("Mqtt connection [FAILED] [ rc = ");
    Serial.print(tb.connected());
    Serial.println(" : retrying in 5 seconds]");
    // Wait 5 seconds before retrying
    delay(5000);
  }
}

void checkTbConnectionAndReconnectIfRequired()
{
  // Reconnect to ThingsBoard, if needed
  bool tbIsNotConnected = !tb.connected();
  if (tbIsNotConnected)
  {
    subscribed = false;

    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN))
    {
      Serial.println("Failed to connect");
      return;
    }
  }
}

//

#endif //_thingsBoardConfiguration_