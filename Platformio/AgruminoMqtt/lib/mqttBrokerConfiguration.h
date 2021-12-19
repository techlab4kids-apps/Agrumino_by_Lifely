#ifndef _mqttBrokerConfiguration_
#define _mqttBrokerConfiguration_

#include "../lib/agruminoUtilities.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Mqtt parameters
//#define MQTT_BROKER "192.168.12.1"
#define MQTT_BROKER "mqtt.flespi.io" // <--- per uso con scratch online (gorru.github.io/scratch-agrumino)
#define MQTT_BROKER_PORT 1883
#define MQTT_BROKER_USER "FlespiToken 6mcDMRfUctThiFyhFk6sIXDAySQAcRO83PzDwEnwV5D2yZ9174kDRL3gtKEtpipg"
#define MQTT_BROKER_PASSWORD ""

#define MQTT_DEVICE_TELEMETRY_TOPIC "devices/CLIENT_NAME/data";
#define MQTT_DEVICE_COMMAND_TOPIC "devices/CLIENT_NAME/command";

extern Agrumino agrumino;
extern WiFiClient espClient;

PubSubClient mqttClient(espClient);

void mqttBrokerLoop(){
    mqttClient.loop();
}

String getDeviceMqttClientName()
{
  // dmc: Device Mqtt Client ID
  return String(ESP.getChipId());
}

void payloadDeserialize( byte *payload, unsigned int length)
{
  char payloadChars[length + 1];
  strncpy(payloadChars, (char *)payload, length);
  payloadChars[length] = '\0';

  Serial.print("Message: ");
  Serial.println(payloadChars);
  String payloadString = String(payloadChars);

  // Decode JSON request
  StaticJsonDocument<200> doc;

  auto error = deserializeJson(doc, payloadChars);
  if (error)
  {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return;
  }
}

void sendAgruminoDataToMqttBroker(AgruminoData &agruminoData)
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

// The callback for when a PUBLISH message is received from the server.
void on_message(const char *topic, byte *payload, unsigned int length)
{

  Serial.println("On message");

  char payloadChars[length + 1];
  strncpy(payloadChars, (char *)payload, length);
  payloadChars[length] = '\0';

  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(payloadChars);
  String payloadString = String(payloadChars);

  Serial.println("methodName: " + payloadString);
  // We expect to recive a JSON in this format
  // name: command name
  // max 5 parameters: {parameter1: parameter1Value, parameter2: parameter2Value, ...}

  const int capacity = JSON_OBJECT_SIZE(1) + 5*JSON_OBJECT_SIZE(1);
  StaticJsonDocument<capacity> commandDoc;
  DeserializationError err = deserializeJson(commandDoc, payloadString);

  if (err) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(err.c_str());
  }

  const char* commandChars = commandDoc["name"];
  String command = String(commandChars);
  Serial.print("Received command: ");
  Serial.println(command.c_str());

  if (command.equals("getLedStatus"))
  {
    AgruminoData agruminoData;

    getAgruminoData(agruminoData);

    sendAgruminoDataToMqttBroker(agruminoData);
  }
  else if (command.equals("setLedStatus"))
  {
    bool status = commandDoc["parameters"]["value"];
    Serial.print("New led status: ");
    Serial.println(status);
    setLedStatus(status);
  }
  else if (command.equals("toggleLed"))
  {
    toggleLed();
    AgruminoData agruminoData;

    Serial.println("Updating led status data");
    getAgruminoData(agruminoData);

    sendAgruminoDataToMqttBroker(agruminoData);
  }
}

void checkMqttBrokerConnectionAndReconnectIfRequired()
{
  bool isConnected = (mqttClient.state() == 0) ?true :false;
  // Loop until we're reconnected
  while (!isConnected)
  {
    String clientName = getDeviceMqttClientName();
    Serial.print("Connecting to Mqtt Broker with client id: ");
    Serial.print(clientName);

    mqttClient.connect(clientName.c_str(), MQTT_BROKER_USER, "");
    isConnected = (mqttClient.state() == 0) ?true :false;
    // Attempt to connect (clientId, username, password)
    if (isConnected)
    {
      Serial.println(" - [DONE]");

      Serial.print("Connection status: ");
      Serial.print(mqttClient.state());
      Serial.println("");

      mqttClient.setCallback(on_message);

      String topic = MQTT_DEVICE_COMMAND_TOPIC;
      topic.replace("CLIENT_NAME", clientName);
      Serial.println("Subscribed to topic: " + topic);
      mqttClient.subscribe(topic.c_str());
      
      //isNotClientConnected = (mqttClient.state() != 0) ?true :false;
    }
    else
    {
      Serial.print(" - [FAILED] [ rc = ");
      Serial.print(mqttClient.state());
      Serial.println(" : retrying in 5 seconds]");
      // Wait 5 seconds before retrying
      delay(5000);
    }

    Serial.println("Connection status after configuration: ");
    Serial.print("IsConnected: ");
    Serial.println(mqttClient.connected());
    Serial.print("Status: ");
    Serial.print(mqttClient.state());
    Serial.println("");

    // isNotClientConnected = !mqttClient.connected();
  }
}

void connectToMqttBroker()
{
  mqttClient.setServer(MQTT_BROKER, MQTT_BROKER_PORT);
  checkMqttBrokerConnectionAndReconnectIfRequired();
}

#endif //_mqttBrokerConfiguration_